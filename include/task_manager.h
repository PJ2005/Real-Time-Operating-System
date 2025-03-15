#ifndef TASK_MANAGER_H
#define TASK_MANAGER_H

#include "system_config.h"
#include <stdint.h>

// Task structure
typedef struct
{
    uint32_t id;
    char name[32];
    void (*entryPoint)(void *);
    void *args;
    uint32_t periodMs;
    uint32_t deadlineMs;
    uint32_t executionTimeMs;
    uint32_t basePriority;
    uint32_t dynamicPriority;
    CriticalityLevel criticality;
    float executionHistory[10];
    uint32_t lastExecutionTime;
    uint32_t missedDeadlines;
    uint8_t coreAffinity;
    uint8_t active;
} Task;

// Function prototypes
int task_create(Task *task, char *name, void (*entryPoint)(void *),
                void *args, uint32_t periodMs, uint32_t deadlineMs,
                CriticalityLevel criticality);
void task_delete(uint32_t taskId);
Task *task_get_current(void);
void task_yield(void);
void task_delay(uint32_t milliseconds);
void task_set_priority(uint32_t taskId, uint32_t priority);
int task_ready(uint32_t taskId);
int task_suspend(uint32_t taskId);
int task_resume(uint32_t taskId);

#endif // TASK_MANAGER_H
