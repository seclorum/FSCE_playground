#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <string.h>

// === CONFIG ===

#define MAX_TASKS     8
#define MAX_EVENTS    8
#define MAX_GROUPS    4
#define WATCHDOG_TIMEOUT_MS 3000

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

// === TASK STRUCT ===

typedef struct {
    int state;
    uint8_t id;
    uint32_t wake_time;
    uint32_t last_run_time;
    void *user_data;
    TaskGroup group;
    TaskPriority priority;
    uint8_t active;
    uint8_t suspended;
    uint8_t watchdog_enabled;
    uint32_t watchdog_reset_time;
} Task;

typedef void (*TaskFunc)(Task*);

typedef struct {
    Task task;
    TaskFunc func;
    TaskFunc initial_func;
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

// === GROUPS ===

uint8_t group_suspended[MAX_GROUPS];

void group_suspend(TaskGroup group) {
    if (group < MAX_GROUPS) group_suspended[group] = 1;
}

void group_resume(TaskGroup group) {
    if (group < MAX_GROUPS) group_suspended[group] = 0;
}

// === LOGGING HOOKS ===

void log_task_start(Task *task) {
    printf("[Log] Task %d started (Group %d, Prio %d)\n",
           task->id, task->group, task->priority);
}

void log_task_end(Task *task) {
    printf("[Log] Task %d ended\n", task->id);
}

void log_task_watchdog_reset(Task *task) {
    printf("[WDT] Task %d watchdog reset triggered!\n", task->id);
}

// === TASK MANAGEMENT ===

int register_task(TaskFunc func, uint8_t id, TaskPriority prio, TaskGroup group, void *data) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!task_list[i].task.active) {
            task_list[i].task = (Task){
                .id = id, .state = 0, .priority = prio,
                .group = group, .user_data = data,
                .wake_time = 0, .active = 1, .suspended = 0,
                .last_run_time = millis(),
                .watchdog_enabled = 1,
                .watchdog_reset_time = millis() + WATCHDOG_TIMEOUT_MS
            };
            task_list[i].func = func;
            task_list[i].initial_func = func;
            log_task_start(&task_list[i].task);
            return i;
        }
    }
    return -1;
}

void remove_task(int slot) {
    if (slot >= 0 && slot < MAX_TASKS) {
        log_task_end(&task_list[slot].task);
        task_list[slot].task.active = 0;
    }
}

void restart_task(int slot) {
    if (slot >= 0 && slot < MAX_TASKS && task_list[slot].task.active) {
        Task *t = &task_list[slot].task;
        t->state = 0;
        t->wake_time = 0;
        t->last_run_time = millis();
        t->watchdog_reset_time = millis() + WATCHDOG_TIMEOUT_MS;
        task_list[slot].func = task_list[slot].initial_func;
        log_task_start(t);
    }
}

// === WATCHDOG ===

void watchdog_check() {
    for (int i = 0; i < MAX_TASKS; i++) {
        Task *t = &task_list[i].task;
        if (!t->active || !t->watchdog_enabled) continue;
        if (millis() > t->watchdog_reset_time) {
            log_task_watchdog_reset(t);
            restart_task(i);
        }
    }
}

// === SCHEDULER ===

void scheduler_tick() {
    // Sort by priority
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
            t->last_run_time = millis();
            t->watchdog_reset_time = millis() + WATCHDOG_TIMEOUT_MS;
            task_list[i].func(t);
            if (t->state == -1) {
                log_task_end(t);
                t->active = 0;
            }
        }
    }

    watchdog_check();
}

// === EXAMPLE TASKS ===

void task_counter(Task *task) {
    int *count = (int*)task->user_data;
    switch (task->state) {
        case 0:
        while (*count < 5) {
            printf("[Counter %d] %d\n", task->id, (*count)++);
            hc_task_delay(task, 400);
        }
        break;
    }
    task->state = -1;
}

void task_flaky(Task *task) {
    static uint8_t fail = 1;
    switch (task->state) {
        case 0:
        printf("[Flaky %d] Running...\n", task->id);
        if (fail) {
            fail = 0;
            hc_task_delay(task, 5000); // exceeds watchdog
        } else {
            printf("[Flaky %d] Success on retry.\n", task->id);
        }
        break;
    }
    task->state = -1;
}

void task_logger(Task *task) {
    hc_task_every(task, 1000);
    printf("[Logger %d] Running at %u ms\n", task->id, millis());
}

// === MAIN ===

int main() {
    memset(event_flags, 0, sizeof(event_flags));
    memset(group_suspended, 0, sizeof(group_suspended));

    int count = 0;
    register_task(task_counter, 0, 2, 0, &count);
    register_task(task_flaky, 1, 3, 0, NULL);
    register_task(task_logger, 2, 1, 1, NULL);

    uint32_t start = millis();
    while (1) {
        scheduler_tick();

        struct timespec wait = {0, 50 * 1000000}; // 50 ms
        nanosleep(&wait, NULL);

        if (millis() - start > 12000) {
            printf("\n[Main] Finished.\n");
            break;
        }
    }

    return 0;
}

