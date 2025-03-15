#include "../../include/system_config.h"
#include "../../include/task_manager.h"
#include "../../include/scheduler.h"
#include "../../include/ml_engine.h"
#include "../../include/fault_tolerance.h"
#include "../../include/memory_matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

// Global system state
static SystemStateVector g_systemState = {0};
static Task g_tasks[MAX_TASKS] = {0};
static int g_taskCount = 0;
static SystemState g_currentState = NORMAL_STATE;
static HANDLE g_schedulerThread = NULL;
static HANDLE g_faultMonitorThread = NULL;
static HANDLE g_systemTimer = NULL;
static int g_running = 0;
static uint64_t g_tickCount = 0;

// Helper functions for Windows simulation
LARGE_INTEGER g_frequency;
LARGE_INTEGER g_startTime;

static uint64_t get_time_ns(void)
{
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    return (current.QuadPart - g_startTime.QuadPart) * 1000000000 / g_frequency.QuadPart;
}

static DWORD WINAPI scheduler_thread_func(LPVOID lpParam)
{
    while (g_running)
    {
        scheduler_tick();
        Sleep(SCHEDULER_PERIOD_MS);
    }
    return 0;
}

static DWORD WINAPI fault_monitor_thread_func(LPVOID lpParam)
{
    while (g_running)
    {
        FaultDetectionResult result = fault_check_system();
        if (result.faultDetected)
        {
            fault_recovery_action(&result);
        }
        Sleep(VOTING_PERIOD_MS);
    }
    return 0;
}

void kernel_init(void)
{
    // Initialize timing
    QueryPerformanceFrequency(&g_frequency);
    QueryPerformanceCounter(&g_startTime);

    // Initialize subsystems
    printf("Initializing memory matrix...\n");
    memory_matrix_init();

    printf("Initializing scheduler...\n");
    scheduler_init();

    printf("Initializing ML engine...\n");
    ml_engine_init();

    printf("Initializing fault tolerance...\n");
    fault_tolerance_init();

    // Initialize system state
    g_systemState.cpuLoad = 0.0f;
    g_systemState.memoryUsage = 0.0f;
    g_systemState.temperature = 25.0f; // Room temperature
    g_systemState.powerConsumption = 1.0f;
    g_systemState.activeTaskCount = 0;
    g_systemState.state = NORMAL_STATE;

    g_running = 0;
    g_taskCount = 0;
    g_tickCount = 0;

    printf("RTOS kernel initialized successfully\n");
}

void kernel_start(void)
{
    if (g_running)
        return;

    g_running = 1;

    // Create scheduler thread
    g_schedulerThread = CreateThread(NULL, 0, scheduler_thread_func, NULL, 0, NULL);
    if (g_schedulerThread == NULL)
    {
        printf("Error: Failed to create scheduler thread\n");
        exit(1);
    }

    // Create fault monitor thread
    g_faultMonitorThread = CreateThread(NULL, 0, fault_monitor_thread_func, NULL, 0, NULL);
    if (g_faultMonitorThread == NULL)
    {
        printf("Error: Failed to create fault monitor thread\n");
        exit(1);
    }

    printf("RTOS kernel started\n");
}

void kernel_stop(void)
{
    if (!g_running)
        return;

    g_running = 0;

    // Wait for threads to terminate
    WaitForSingleObject(g_schedulerThread, INFINITE);
    WaitForSingleObject(g_faultMonitorThread, INFINITE);

    CloseHandle(g_schedulerThread);
    CloseHandle(g_faultMonitorThread);

    printf("RTOS kernel stopped\n");
}

SystemStateVector *kernel_get_system_state(void)
{
    return &g_systemState;
}

void kernel_update_load(float cpuLoad, float memoryUsage)
{
    g_systemState.cpuLoad = cpuLoad;
    g_systemState.memoryUsage = memoryUsage;

    // Update scheduler with new system state
    scheduler_update_system_state(&g_systemState);
}

int kernel_add_task(Task *task)
{
    if (g_taskCount >= MAX_TASKS)
    {
        return -1; // Too many tasks
    }

    g_tasks[g_taskCount] = *task;
    g_tasks[g_taskCount].id = g_taskCount;
    g_tasks[g_taskCount].active = 1;
    g_taskCount++;
    g_systemState.activeTaskCount = g_taskCount;

    return g_taskCount - 1; // Return task ID
}

Task *kernel_get_tasks(int *count)
{
    *count = g_taskCount;
    return g_tasks;
}

int kernel_get_running(void)
{
    return g_running;
}

uint64_t kernel_get_tick_count(void)
{
    return g_tickCount;
}

void kernel_increment_tick(void)
{
    g_tickCount++;
}
