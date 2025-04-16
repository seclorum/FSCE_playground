#include <stdio.h>

// Define the yield macro
#define hc_task_yield(task)       \
  do {                            \
    (task)->state = __LINE__;     \
    return;                       \
    case __LINE__:;               \
  } while (0)

// Define a task structure
typedef struct {
    int state;      // Where to resume
    int counter;    // Some task-specific state
} Task;

// A simple task that runs in steps
void task_print(Task *task) {
    switch (task->state) {
        case 0:

        printf("Step 1: counter = %d\n", task->counter++);
        hc_task_yield(task);

        printf("Step 2: counter = %d\n", task->counter++);
        hc_task_yield(task);

        printf("Step 3: counter = %d\n", task->counter++);
        hc_task_yield(task);

        printf("Task complete!\n");
        break;
    }
    task->state = -1; // Mark as complete
}

// Main "scheduler"
int main() {
    Task myTask = {0, 0};

    // Simulate a simple cooperative loop
    while (myTask.state != -1) {
        task_print(&myTask);
    }

    return 0;
}

