#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

// === CONFIG ===

#define MAX_TASKS     8
#define MAX_EVENTS    8
#define MAX_GROUPS    4

typedef uint8_t TaskGroup;
typedef uint8_t TaskPriority;
typedef uint8_t EventID;

// === TIME (millis) ===

uint32_t millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// === COROUTINE MACROS ===

#define hc_task_yield(task)        \
  do {                             \
    (task)->state = __LINE__;      \
    return;                        \
    case __LINE__:;                \
  } while (0)

#define hc_task_delay(task, ms)             \
  do {                                      \
    (task)->wake_time = millis() + (ms);    \
    hc_task_yield(task);                    \
  } while ((task)->wake_time > millis())

#define hc_task_wait_until(task, cond)      \
  do {                                      \
    (task)->state = __LINE__;               \
    return;                                 \
    case __LINE__:;                         \
  } while (!(cond))

#define hc_task_every(task, interval_ms)                \
  static uint32_t _last_run = 0;                        \
  if (millis() - _last_run < (interval_ms)) return;     \
  _last_run = millis()

// === TASK TYPES ===

typedef struct {
    int state;
    uint8_t id;
    uint32_t wake_time;
    void *user_data;
    TaskGroup group;
    TaskPriority priority;
    uint8_t active;
    uint8_t suspended;
} Task;

typedef void (*TaskFunc)(Task *);

typedef struct {
    Task task;
    TaskFunc func;
} TaskEntry;

TaskEntry task_list[MAX_TASKS];

// === EVENTS ===

uint8_t event_flags[MAX_EVENTS];

void event_set(EventID id) {
    if (id < MAX_EVENTS) event_flags[id] = 1;
}

void event_clear(EventID id) {
    if (id < MAX_EVENTS) event_flags[id] = 0;
}

uint8_t event_check(EventID id) {
    return id < MAX_EVENTS ? event_flags[id] : 0;
}

// === GROUP MANAGEMENT ===

uint8_t group_suspended[MAX_GROUPS];

void group_suspend(TaskGroup group) {
    if (group < MAX_GROUPS) group_suspended[group] = 1;
}

void group_resume(TaskGroup group) {
    if (group < MAX_GROUPS) group_suspended[group] = 0;
}

// === TASK MANAGEMENT ===

int register_task(TaskFunc func, uint8_t id, TaskPriority prio, TaskGroup group, void *data) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!task_list[i].task.active) {
            task_list[i].task = (Task){
                .id = id, .state = 0, .priority = prio,
                .group = group, .user_data = data,
                .wake_time = 0, .active = 1, .suspended = 0
            };
            task_list[i].func = func;
            return i;
        }
    }
    return -1;
}

void remove_task(int slot) {
    if (slot >= 0 && slot < MAX_TASKS)
        task_list[slot].task.active = 0;
}

// === SCHEDULER ===

void scheduler_tick() {
    // Sort by priority (simple bubble sort)
    for (int i = 0; i < MAX_TASKS - 1; i++) {
        for (int j = i + 1; j < MAX_TASKS; j++) {
            if (task_list[j].task.active && task_list[i].task.active &&
                task_list[j].task.priority > task_list[i].task.priority) {
                TaskEntry tmp = task_list[i];
                task_list[i] = task_list[j];
                task_list[j] = tmp;
            }
        }
    }

    for (int i = 0; i < MAX_TASKS; i++) {
        Task *t = &task_list[i].task;
        if (!t->active || t->suspended || group_suspended[t->group]) continue;
        if (millis() >= t->wake_time) {
            task_list[i].func(t);
            if (t->state == -1) t->active = 0;
        }
    }
}

// === TASKS ===

void task_blink(Task *task) {
    switch (task->state) {
        case 0:
        while (1) {
            printf("[Blink %d] LED ON\n", task->id);
            hc_task_delay(task, 500);
            printf("[Blink %d] LED OFF\n", task->id);
            hc_task_delay(task, 500);
        }
    }
}

void task_periodic_logger(Task *task) {
    hc_task_every(task, 1000);
    printf("[Logger %d] Tick at %u ms\n", task->id, millis());
}

void task_event_wait(Task *task) {
    EventID id = *(EventID*)task->user_data;
    switch (task->state) {
        case 0:
        printf("[Wait %d] Waiting for event %d...\n", task->id, id);
        hc_task_wait_until(task, event_check(id));
        printf("[Wait %d] Got event %d!\n", task->id, id);
        break;
    }
    task->state = -1;
}

void task_event_trigger(Task *task) {
    EventID id = *(EventID*)task->user_data;
    switch (task->state) {
        case 0:
        printf("[Trigger %d] Will trigger event %d in 2s\n", task->id, id);
        hc_task_delay(task, 2000);
        event_set(id);
        printf("[Trigger %d] Event %d triggered!\n", task->id, id);
        break;
    }
    task->state = -1;
}

void task_group_suspend(Task *task) {
    switch (task->state) {
        case 0:
        printf("[Suspender] Suspending group 1 in 3s...\n");
        hc_task_delay(task, 3000);
        group_suspend(1);
        printf("[Suspender] Group 1 suspended!\n");
        hc_task_delay(task, 2000);
        group_resume(1);
        printf("[Suspender] Group 1 resumed!\n");
        break;
    }
    task->state = -1;
}

// === MAIN ===

int main() {
    memset(event_flags, 0, sizeof(event_flags));
    memset(group_suspended, 0, sizeof(group_suspended));

    EventID eid = 2;

    register_task(task_blink,         0, 1, 0, NULL);
    register_task(task_periodic_logger, 1, 3, 0, NULL);
    register_task(task_event_wait,    2, 2, 1, &eid);
    register_task(task_event_trigger, 3, 2, 1, &eid);
    register_task(task_group_suspend, 4, 4, 0, NULL);

    uint32_t start = millis();
    while (1) {
        scheduler_tick();

        struct timespec wait = {0, 50 * 1000000}; // 50 ms
        nanosleep(&wait, NULL);

        if (millis() - start > 10000) {
            printf("\n[Main] Done.\n");
            break;
        }
    }

    return 0;
}

