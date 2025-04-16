#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define MAX_TASKS 8

// --- Coroutine Macros ---

#define hc_task_yield(task)        \
  do {                             \
    (task)->state = __LINE__;      \
    return;                        \
    case __LINE__:;                \
  } while (0)

#define hc_task_delay(task, ticks) \
  do {                             \
    (task)->wake_time = millis() + (ticks); \
    hc_task_yield(task);          \
  } while ((task)->wake_time > millis())

#define hc_task_wait_until(task, condition) \
  do {                                      \
    (task)->state = __LINE__;               \
    return;                                 \
    case __LINE__:;                         \
  } while (!(condition))

// --- Millis Timer ---

unsigned long millis() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (unsigned long)(ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

// --- Task Types ---

typedef struct {
    int state;
    int id;
    unsigned long wake_time;
    void *user_data;
} Task;

typedef void (*TaskFunc)(Task *);

// --- Scheduler System ---

typedef struct {
    Task task;
    TaskFunc func;
    int active;
} TaskEntry;

TaskEntry tasks[MAX_TASKS];

int register_task(TaskFunc func, int id, void *user_data) {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (!tasks[i].active) {
            tasks[i].task = (Task){ .state = 0, .id = id, .wake_time = 0, .user_data = user_data };
            tasks[i].func = func;
            tasks[i].active = 1;
            return i;
        }
    }
    return -1; // no slots
}

void remove_task(int slot) {
    if (slot >= 0 && slot < MAX_TASKS) {
        tasks[slot].active = 0;
    }
}

void scheduler_tick() {
    for (int i = 0; i < MAX_TASKS; i++) {
        if (tasks[i].active) {
            Task *t = &tasks[i].task;
            if (millis() >= t->wake_time) {
                tasks[i].func(t);
                if (t->state == -1) {
                    remove_task(i);
                }
            }
        }
    }
}

// --- Demo Tasks ---

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

void task_counter(Task *task) {
    int *count = (int *)task->user_data;
    switch (task->state) {
        case 0:
        while (1) {
            printf("[Counter %d] Count: %d\n", task->id, (*count)++);
            hc_task_delay(task, 300);
        }
    }
}

void task_once(Task *task) {
    switch (task->state) {
        case 0:
        printf("[Once %d] Step A\n", task->id);
        hc_task_delay(task, 100);
        printf("[Once %d] Step B\n", task->id);
        hc_task_delay(task, 200);
        printf("[Once %d] Finished\n", task->id);
        break;
    }
    task->state = -1;
}

// --- Main Loop ---

int main() {
    int counter_data = 0;

    register_task(task_blink, 0, NULL);
    register_task(task_counter, 1, &counter_data);
    register_task(task_once, 2, NULL);

    unsigned long last_time = millis();
    while (1) {
        scheduler_tick();

        // Delay a bit to simulate system tick (~every 50ms)
        struct timespec req = {.tv_sec = 0, .tv_nsec = 50 * 1000000 };
        nanosleep(&req, NULL);

        // Optional: exit after 5 seconds
        if (millis() - last_time > 5000) {
            printf("\n[Main] Done.\n");
            break;
        }
    }

    return 0;
}

