#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

// === CONFIG ===

#define MAX_TASKS     8
#define MAX_EVENTS    8

typedef uint8_t TaskGroup;
typedef uint8_t TaskPriority;
typedef uint8_t EventID;

// === TIME ===

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

#define hc_task_delay(task, ms)            \
  do {                                     \
    (task)->wake_time = millis() + (ms);   \
    hc_task_yield(task);                   \
  } while ((task)->wake_time > millis())

#define hc_task_wait_until(task, cond)     \
  do {                                     \
    (task)->state = __LINE__;              \
    return;                                \
    case __LINE__:;                        \
  } while (!(cond))

// === TASK TYPES ===

typedef struct {
    int state;
    uint8_t id;
    uint32_t wake_time;
    void *user_data;
    TaskGroup group;
    TaskPriority priority;
    uint8_t active;
} Task;

typedef void (*TaskFunc)(Task*);

typedef struct {
    Task task;
    TaskFunc func;
} TaskEntry;

TaskEntry task_list[MAX_TASKS];

// === EVENTS / ALARMS ===

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

// === TASK SCHEDULER ===

int register_task(TaskFunc func, uint8_t id, TaskPriority priority, TaskGroup group, void *data) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!task_list[i].task.active) {
            task_list[i].task = (Task){
                .id = id, .state = 0, .priority = priority,
                .group = group, .user_data = data, .active = 1
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

void scheduler_tick() {
    // Sort by priority (simple bubble sort — small list)
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
        if (!t->active) continue;
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

void task_alarm_wait(Task *task) {
    EventID event_id = *(EventID*)task->user_data;
    switch (task->state) {
        case 0:
        printf("[Alarm %d] Waiting for event %d...\n", task->id, event_id);
        hc_task_wait_until(task, event_check(event_id));
        printf("[Alarm %d] Got event %d!\n", task->id, event_id);
        break;
    }
    task->state = -1;
}

void task_trigger_alarm(Task *task) {
    EventID event_id = *(EventID*)task->user_data;
    switch (task->state) {
        case 0:
        printf("[Trigger %d] Will trigger event %d in 2s\n", task->id, event_id);
        hc_task_delay(task, 2000);
        event_set(event_id);
        printf("[Trigger %d] Event %d triggered!\n", task->id, event_id);
        break;
    }
    task->state = -1;
}

void task_count(Task *task) {
    int *count = (int*)task->user_data;
    switch (task->state) {
        case 0:
        while (*count < 5) {
            printf("[Count %d] %d\n", task->id, (*count)++);
            hc_task_delay(task, 400);
        }
        printf("[Count %d] Done counting.\n", task->id);
        break;
    }
    task->state = -1;
}

// === MAIN ===

int main() {
    memset(event_flags, 0, sizeof(event_flags));
    int counter = 0;
    EventID alarm_id = 1;

    register_task(task_blink, 0, 1, 0, NULL);
    register_task(task_count, 1, 2, 0, &counter);
    register_task(task_alarm_wait, 2, 1, 1, &alarm_id);
    register_task(task_trigger_alarm, 3, 2, 1, &alarm_id);

    uint32_t start_time = millis();
    while (1) {
        scheduler_tick();

        struct timespec wait = {0, 50 * 1000000};
        nanosleep(&wait, NULL);

        if (millis() - start_time > 7000) {
            printf("\n[Main] Done.\n");
            break;
        }
    }

    return 0;
}

