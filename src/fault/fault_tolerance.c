#include "../include/fault_tolerance.h"
#include "../include/system_config.h"
#include "../include/memory_matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <task_manager.h>

// External function declarations
extern Task *kernel_get_tasks(int *count);

// Fault monitoring variables
static FaultType g_injectedFaults[MAX_TASKS] = {NO_FAULT};
static uint32_t g_faultAddresses[MAX_TASKS] = {0};
static uint32_t g_watchdogTimers[MAX_TASKS] = {0};
static uint64_t g_watchdogDeadlines[MAX_TASKS] = {0};
static uint32_t g_faultRecoveryCounter[MAX_TASKS] = {0};
static uint64_t g_lastCheckTime = 0;

void fault_tolerance_init(void)
{
    printf("Initializing fault tolerance subsystem\n");

    // Reset all fault status
    for (int i = 0; i < MAX_TASKS; i++)
    {
        g_injectedFaults[i] = NO_FAULT;
        g_faultAddresses[i] = 0;
        g_watchdogTimers[i] = 0;
        g_watchdogDeadlines[i] = 0;
        g_faultRecoveryCounter[i] = 0;
    }

    g_lastCheckTime = GetTickCount64();
    printf("Fault tolerance initialized\n");
}

FaultDetectionResult fault_check_system(void)
{
    FaultDetectionResult result = {0};
    uint64_t currentTime = GetTickCount64();

    // Check for watchdog timeouts
    int count;
    Task *tasks = kernel_get_tasks(&count);

    for (int i = 0; i < count; i++)
    {
        if (tasks[i].active && g_watchdogTimers[i] > 0)
        {
            if (currentTime > g_watchdogDeadlines[i])
            {
                // Watchdog timeout detected
                result.faultDetected = 1;
                result.type = TIMING_FAULT;
                result.taskId = i;
                result.timestamp = currentTime;
                printf("FAULT DETECTED: Watchdog timeout for task '%s'\n", tasks[i].name);
                return result;
            }
        }

        // Check for injected faults (for testing)
        if (g_injectedFaults[i] != NO_FAULT)
        {
            result.faultDetected = 1;
            result.type = g_injectedFaults[i];
            result.taskId = i;
            result.address = g_faultAddresses[i];
            result.timestamp = currentTime;
            printf("FAULT DETECTED: Injected %d fault for task '%s'\n", result.type, tasks[i].name);
            return result;
        }
    }

    g_lastCheckTime = currentTime;
    return result;
}

int fault_inject(FaultType type, uint32_t taskId, uint32_t address)
{
    int count;
    Task *tasks = kernel_get_tasks(&count);

    if (taskId < count)
    {
        g_injectedFaults[taskId] = type;
        g_faultAddresses[taskId] = address;
        printf("Fault injected: Type %d for task '%s'\n", type, tasks[taskId].name);
        return 0; // Success
    }

    return -1; // Failed
}

void fault_recovery_action(FaultDetectionResult *result)
{
    if (!result || !result->faultDetected)
    {
        return;
    }

    int count;
    Task *tasks = kernel_get_tasks(&count);

    if (result->taskId >= count)
    {
        printf("Error: Invalid task ID in fault recovery\n");
        return;
    }

    Task *faultyTask = &tasks[result->taskId];

    printf("Executing recovery for task '%s' (fault type: %d)\n", faultyTask->name, result->type);

    switch (result->type)
    {
    case TIMING_FAULT:
        // Reset watchdog and mark task for re-execution
        set_watchdog_timer(result->taskId, faultyTask->deadlineMs * 2); // Extended deadline
        g_faultRecoveryCounter[result->taskId]++;
        break;

    case MEMORY_FAULT:
        // Restore task state from backup
        printf("Restoring task state from backup\n");
        g_faultRecoveryCounter[result->taskId]++;
        break;

    case COMPUTATION_FAULT:
        // Re-execute the task
        printf("Scheduling task re-execution\n");
        g_faultRecoveryCounter[result->taskId]++;
        break;

    case COMMUNICATION_FAULT:
        // Reset communication channels
        printf("Resetting communication channels\n");
        g_faultRecoveryCounter[result->taskId]++;
        break;

    case POWER_FAULT:
        // Switch to low-power mode
        printf("Switching to low-power mode\n");
        g_faultRecoveryCounter[result->taskId]++;
        break;

    default:
        printf("Unknown fault type, no recovery action taken\n");
        break;
    }

    // Clear the injected fault (for testing)
    g_injectedFaults[result->taskId] = NO_FAULT;
    g_faultAddresses[result->taskId] = 0;
}

uint8_t tmr_voting(uint32_t result1, uint32_t result2, uint32_t result3)
{
    // Simple majority voting for Triple Modular Redundancy
    if (result1 == result2 || result1 == result3)
    {
        return result1;
    }
    else if (result2 == result3)
    {
        return result2;
    }
    else
    {
        // No majority, this is an error case
        printf("TMR ERROR: No majority in voting\n");
        return 0xFF; // Error code
    }
}

float get_fault_recovery_factor(uint32_t taskId)
{
    // Higher value indicates task needs priority boost for recovery
    float factor = 1.0f;

    if (taskId < MAX_TASKS && g_faultRecoveryCounter[taskId] > 0)
    {
        // Apply exponential backoff for repeated faults
        factor = 1.0f + (float)(g_faultRecoveryCounter[taskId]) * 0.2f;
        if (factor > 2.0f)
            factor = 2.0f; // Cap at 2.0
    }

    return factor;
}

void set_watchdog_timer(uint32_t taskId, uint32_t timeoutMs)
{
    if (taskId < MAX_TASKS)
    {
        g_watchdogTimers[taskId] = timeoutMs;
        g_watchdogDeadlines[taskId] = GetTickCount64() + timeoutMs;
        printf("Watchdog set for task %u: %u ms\n", taskId, timeoutMs);
    }
}
