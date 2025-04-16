/*
 ============================================================================
  mega_hc.c - Embedded-Friendly Cooperative Multitasking Framework in C
 ============================================================================
  Features:
    ✅ Coroutine-style tasks using Duff's device macro (__LINE__ trick)
    ✅ Task priorities (higher runs first)
    ✅ Task groups/tags (for suspend/resume control)
    ✅ Delay/yield/wait/timer mechanisms
    ✅ Events/alarms (set/clear/check event flags)
    ✅ Task restart/reset API
    ✅ Watchdog timer (auto-reset unresponsive tasks)
    ✅ Logging/debugging hooks
    ✅ State snapshot API
    ✅ CLI-style debug commands (basic)
    ✅ Single-file and portable (no heap allocation or OS dependencies)

  Suitable for:
    - Embedded systems
    - Cooperative kernels
    - State machines and game loops
    - Teaching coroutine scheduling in C

  Author: seclorum
  License: MIT
============================================================================
*/

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>

#define MAX_TASKS     8
#define MAX_EVENTS    8
#define MAX_GROUPS    4
#define WATCHDOG_TIMEOUT_MS 3000

typedef uint8_t TaskGroup;
typedef uint8_t TaskPriority;
typedef uint8_t EventID;

typedef struct {
    int state;
    uint8_t id;
    uint8_t active;
    uint8_t suspended;
    TaskGroup group;
    TaskPriority priority;
    uint32_t wake_time;
    uint32_t last_run_time;
    void *user_data;
    uint8_t watchdog_enabled;
    uint32_t watchdog_reset_time;
} Task;

typedef void (*TaskFunc)(Task*);

typedef struct {
    Task task;
    TaskFunc func;
    TaskFunc original_func;
} TaskEntry;

TaskEntry task_list[MAX_TASKS];
uint8_t event_flags[MAX_EVENTS];
uint8_t group_suspended[MAX_GROUPS];

uint32_t millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint32_t)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

#define hc_task_yield(task)        \
    do {                           \
        (task)->state = __LINE__; \
        return;                    \
        case __LINE__:;            \
    } while (0)

#define hc_task_delay(task, ms)                \
    do {                                       \
        (task)->wake_time = millis() + (ms);   \
        hc_task_yield(task);                   \
    } while ((task)->wake_time > millis())

#define hc_task_wait_until(task, cond)         \
    do {                                       \
        (task)->state = __LINE__;              \
        return;                                \
        case __LINE__:;                        \
    } while (!(cond))

#define hc_task_every(task, interval_ms)       \
    static uint32_t _last = 0;                 \
    if (millis() - _last < (interval_ms)) return; \
    _last = millis()

int register_task(TaskFunc func, uint8_t id, TaskPriority prio, TaskGroup group, void *data) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!task_list[i].task.active) {
            task_list[i].task = (Task){
                .id = id, .state = 0, .active = 1,
                .priority = prio, .group = group,
                .wake_time = 0, .last_run_time = millis(),
                .user_data = data, .suspended = 0,
                .watchdog_enabled = 1,
                .watchdog_reset_time = millis() + WATCHDOG_TIMEOUT_MS
            };
            task_list[i].func = func;
            task_list[i].original_func = func;
            printf("[Log] Task %d registered (prio=%d, group=%d)\n", id, prio, group);
            return i;
        }
    }
    return -1;
}

void restart_task(int slot) {
    if (slot >= 0 && slot < MAX_TASKS && task_list[slot].task.active) {
        task_list[slot].task.state = 0;
        task_list[slot].task.wake_time = 0;
        task_list[slot].task.last_run_time = millis();
        task_list[slot].task.watchdog_reset_time = millis() + WATCHDOG_TIMEOUT_MS;
        task_list[slot].func = task_list[slot].original_func;
        printf("[Log] Task %d restarted\n", task_list[slot].task.id);
    }
}

void remove_task(int slot) {
    if (slot >= 0 && slot < MAX_TASKS) {
        printf("[Log] Task %d removed\n", task_list[slot].task.id);
        task_list[slot].task.active = 0;
    }
}

void group_suspend(TaskGroup group) {
    if (group < MAX_GROUPS) group_suspended[group] = 1;
}

void group_resume(TaskGroup group) {
    if (group < MAX_GROUPS) group_suspended[group] = 0;
}

void event_set(EventID id) {
    if (id < MAX_EVENTS) event_flags[id] = 1;
}

void event_clear(EventID id) {
    if (id < MAX_EVENTS) event_flags[id] = 0;
}

uint8_t event_check(EventID id) {
    return id < MAX_EVENTS ? event_flags[id] : 0;
}

void watchdog_check() {
    for (int i = 0; i < MAX_TASKS; i++) {
        Task *t = &task_list[i].task;
        if (!t->active || !t->watchdog_enabled) continue;
        if (millis() > t->watchdog_reset_time) {
            printf("[WDT] Task %d timeout. Restarting...\n", t->id);
            restart_task(i);
        }
    }
}

void scheduler_tick() {
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
                t->active = 0;
                printf("[Log] Task %d completed\n", t->id);
            }
        }
    }

    watchdog_check();
}

void dump_task_state() {
    printf("\n[Snapshot] Task States\n");
    for (int i = 0; i < MAX_TASKS; i++) {
        Task *t = &task_list[i].task;
        if (t->active) {
            printf(" - Task %d | Prio %d | Group %d | Susp %d | WT: %u\n",
                   t->id, t->priority, t->group, t->suspended, t->wake_time);
        }
    }
}

void debug_cli() {
    char cmd[32];
    printf("\n[CLI] > ");
    if (fgets(cmd, sizeof(cmd), stdin)) {
        if (strncmp(cmd, "dump", 4) == 0) {
            dump_task_state();
        } else if (strncmp(cmd, "suspend ", 8) == 0) {
            int g = atoi(cmd + 8);
            group_suspend((TaskGroup)g);
        } else if (strncmp(cmd, "resume ", 7) == 0) {
            int g = atoi(cmd + 7);
            group_resume((TaskGroup)g);
        } else {
            printf("Commands: dump | suspend <group> | resume <group>\n");
        }
    }
}

void task_blink(Task *task) {
    static int toggle = 0;
    switch (task->state) {
        case 0:
            while (1) {
                printf("[Task %d] LED %s\n", task->id, toggle ? "ON" : "OFF");
                toggle = !toggle;
                hc_task_delay(task, 500);
            }
    }
}

void task_counter(Task *task) {
    int *val = (int*)task->user_data;
    switch (task->state) {
        case 0:
            while (*val < 10) {
                printf("[Task %d] Counter: %d\n", task->id, (*val)++);
                hc_task_delay(task, 300);
            }
            task->state = -1;
            return;
    }
}

void task_logger(Task *task) {
    switch (task->state) {
        case 0:
            while (1) {
                hc_task_every(task, 1000);
                printf("[Task %d] Logger at %u ms\n", task->id, millis());
            }
    }
}

int main() {
    memset(task_list, 0, sizeof(task_list));
    memset(event_flags, 0, sizeof(event_flags));
    memset(group_suspended, 0, sizeof(group_suspended));

    int count = 0;
    register_task(task_blink, 0, 3, 0, NULL);
    register_task(task_counter, 1, 2, 1, &count);
    register_task(task_logger, 2, 1, 1, NULL);

    uint32_t start = millis();

    while (1) {
        scheduler_tick();

        static uint32_t last_cli = 0;
        if (millis() - last_cli > 5000) {
            last_cli = millis();
            debug_cli();
        }

        struct timespec wait = {0, 50 * 1000000};
        nanosleep(&wait, NULL);

        if (millis() - start > 20000) {
            printf("\n[Main] Shutting down after 20 sec\n");
            break;
        }
    }

    return 0;
}
