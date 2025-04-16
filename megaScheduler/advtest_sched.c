#include <stdio.h>

#define MAX_TASKS 3

// Coroutine-style yield macro
#define hc_task_yield(task)        \
  do {                             \
    (task)->state = __LINE__;      \
    return;                        \
    case __LINE__:;                \
  } while (0)

// Task structure
typedef struct {
    int state;
    int delay;
    int id;
} Task;

// Simulate delay by waiting N scheduler ticks
#define hc_task_delay(task, ticks) \
  do {                             \
    (task)->delay = (ticks);       \
    hc_task_yield(task);           \
  } while ((task)->delay > 0)

// === Task Definitions ===

// Task 1: Prints a message every 3 ticks
void task_blink(Task *task) {
    switch (task->state) {
        case 0:
        while (1) {
            printf("[Task %d] Blink ON\n", task->id);
            hc_task_delay(task, 3);
            printf("[Task %d] Blink OFF\n", task->id);
            hc_task_delay(task, 3);
        }
    }
}

// Task 2: Prints numbers every 2 ticks
void task_counter(Task *task) {
    static int count = 0;
    switch (task->state) {
        case 0:
        while (1) {
            printf("[Task %d] Count: %d\n", task->id, count++);
            hc_task_delay(task, 2);
        }
    }
}

// Task 3: Finite task, runs a few steps then ends
void task_once(Task *task) {
    switch (task->state) {
        case 0:
        printf("[Task %d] Step A\n", task->id);
        hc_task_delay(task, 1);
        printf("[Task %d] Step B\n", task->id);
        hc_task_delay(task, 1);
        printf("[Task %d] Done\n", task->id);
        break;
    }
    task->state = -1; // mark as finished
}

// === Tiny Scheduler ===

typedef void (*TaskFunc)(Task *);

typedef struct {
    Task task;
    TaskFunc func;
} TaskEntry;

TaskEntry tasks[MAX_TASKS];

// Called once per scheduler tick
void scheduler_tick() {
    for (int i = 0; i < MAX_TASKS; i++) {
        Task *t = &tasks[i].task;
        if (t->state != -1) { // skip finished tasks
            if (t->delay > 0) {
                t->delay--;
            }
            if (t->delay == 0) {
                tasks[i].func(t);
            }
        }
    }
}

// === Main ===

int main() {
    // Initialize tasks
    tasks[0].task = (Task){.state = 0, .id = 0};
    tasks[0].func = task_blink;

    tasks[1].task = (Task){.state = 0, .id = 1};
    tasks[1].func = task_counter;

    tasks[2].task = (Task){.state = 0, .id = 2};
    tasks[2].func = task_once;

    // Simulate ticks (like a main loop or timer interrupt)
    for (int tick = 0; tick < 20; tick++) {
        printf("\n== Tick %d ==\n", tick);
        scheduler_tick();
    }

    return 0;
}

