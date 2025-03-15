#include "../../include/task_manager.h"
#include "../../include/system_config.h"
#include "../../include/fault_tolerance.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// External function declarations
extern int kernel_add_task(Task *task);
extern Task *kernel_get_tasks(int *count);
extern int kernel_get_running(void);
extern uint64_t kernel_get_tick_count(void);

// Current task being executed
static Task *g_currentTask = NULL;
static uint32_t g_nextTaskId = 0;

int task_create(Task *task, char *name, void (*entryPoint)(void *),
                void *args, uint32_t periodMs, uint32_t deadlineMs,
                CriticalityLevel criticality)
{
    if (!task || !name || !entryPoint)
    {
        return -1; // Invalid parameters
    }

    // Initialize task structure
    memset(task, 0, sizeof(Task));
    strncpy(task->name, name, sizeof(task->name) - 1);
    task->entryPoint = entryPoint;
    task->args = args;
    task->periodMs = periodMs;
    task->deadlineMs = deadlineMs;
    task->executionTimeMs = periodMs / 10; // Estimate execution time as 10% of period
    task->basePriority = 8;                // Mid priority by default
    task->dynamicPriority = 8;
    task->criticality = criticality;
    task->coreAffinity = 0; // Default to first core
    task->active = 1;

    // Initialize execution history with estimated execution time
    for (int i = 0; i < 10; i++)
    {
        task->executionHistory[i] = (float)task->executionTimeMs;
    }

    // Add task to kernel
    int taskId = kernel_add_task(task);
    if (taskId >= 0)
    {
        task->id = taskId;
        printf("Task '%s' created with ID %d\n", name, taskId);

        // Set watchdog for critical tasks
        if (criticality == DAL_A || criticality == DAL_B)
        {
            set_watchdog_timer(taskId, deadlineMs);
        }

        return taskId;
    }

    return -1; // Failed to add task
}

void task_delete(uint32_t taskId)
{
    int count;
    Task *tasks = kernel_get_tasks(&count);

    if (taskId < count)
    {
        tasks[taskId].active = 0;
        printf("Task '%s' (ID: %u) deleted\n", tasks[taskId].name, taskId);
    }
}

Task *task_get_current(void)
{
    return g_currentTask;
}

void task_yield(void)
{
    // In a real RTOS this would yield execution to the next task
    // For our simulation, we just print a message
    printf("Task '%s' yielded execution\n", g_currentTask ? g_currentTask->name : "Unknown");
    Sleep(1); // Small sleep to simulate yielding
}

void task_delay(uint32_t milliseconds)
{
    // Simulate task delay
    printf("Task '%s' delayed for %u ms\n", g_currentTask ? g_currentTask->name : "Unknown", milliseconds);
    Sleep(milliseconds); // Use Windows Sleep for simulation
}

void task_set_priority(uint32_t taskId, uint32_t priority)
{
    int count;
    Task *tasks = kernel_get_tasks(&count);

    if (taskId < count && priority < MAX_PRIORITY_LEVELS)
    {
        tasks[taskId].basePriority = priority;
        printf("Task '%s' priority set to %u\n", tasks[taskId].name, priority);
    }
}

int task_ready(uint32_t taskId)
{
    int count;
    Task *tasks = kernel_get_tasks(&count);

    if (taskId < count)
    {
        return tasks[taskId].active;
    }

    return 0; // Task not found or not ready
}

int task_suspend(uint32_t taskId)
{
    int count;
    Task *tasks = kernel_get_tasks(&count);

    if (taskId < count)
    {
        if (tasks[taskId].active)
        {
            tasks[taskId].active = 0;
            printf("Task '%s' suspended\n", tasks[taskId].name);
            return 0; // Success
        }
    }

    return -1; // Failed
}

int task_resume(uint32_t taskId)
{
    int count;
    Task *tasks = kernel_get_tasks(&count);

    if (taskId < count)
    {
        if (!tasks[taskId].active)
        {
            tasks[taskId].active = 1;
            printf("Task '%s' resumed\n", tasks[taskId].name);
            return 0; // Success
        }
    }

    return -1; // Failed
}
