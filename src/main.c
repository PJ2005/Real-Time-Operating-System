#include "../include/system_config.h"
#include "../include/task_manager.h"
#include "../include/scheduler.h"
#include "../include/ml_engine.h"
#include "../include/fault_tolerance.h"
#include "../include/memory_matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>
#include <time.h>
#include <conio.h> // For _kbhit and _getch

// External kernel functions
extern void kernel_init(void);
extern void kernel_start(void);
extern void kernel_stop(void);
extern void kernel_update_load(float cpuLoad, float memoryUsage);
extern SystemStateVector *kernel_get_system_state(void);

// Example task functions
void safety_critical_task(void *args);
void control_task(void *args);
void monitoring_task(void *args);
void background_task(void *args);

// Test fault injection
void inject_random_faults(void);
DWORD WINAPI system_monitor_thread(LPVOID lpParam);

// Utility functions
void print_system_state(void);
void print_jitter_statistics(void);

int main(int argc, char *argv[])
{
    printf("Starting Hardware-Accelerated ML RTOS Simulation\n");
    printf("-----------------------------------------------\n\n");

    // Initialize kernel subsystems
    kernel_init();

    // Create tasks
    Task task1, task2, task3, task4;

    task_create(&task1, "SafetyCritical", safety_critical_task, NULL, 100, 100, DAL_A);
    task_create(&task2, "Control", control_task, NULL, 200, 180, DAL_B);
    task_create(&task3, "Monitoring", monitoring_task, NULL, 500, 450, DAL_C);
    task_create(&task4, "Background", background_task, NULL, 1000, 900, DAL_D);

    // Start system monitor thread
    HANDLE monitorThread = CreateThread(NULL, 0, system_monitor_thread, NULL, 0, NULL);
    if (monitorThread == NULL)
    {
        printf("Error: Failed to create system monitor thread\n");
        return 1;
    }

    // Start kernel
    kernel_start();

    // Main processing loop
    printf("\nSystem running... Press Enter to inject a fault, q to quit\n\n");
    char input;
    while (1)
    {
        if (_kbhit())
        {
            input = _getch();
            if (input == 'q' || input == 'Q')
            {
                break;
            }
            else if (input == '\r')
            {
                inject_random_faults();
            }
        }

        // Simulate varying system load
        float load = 0.3f + (float)(rand() % 40) / 100.0f;
        float mem = 0.4f + (float)(rand() % 30) / 100.0f;
        kernel_update_load(load, mem);

        // Update thermal conditions randomly
        SystemStateVector *state = kernel_get_system_state();
        state->temperature = 30.0f + (float)(rand() % 30);
        state->powerConsumption = 1.0f + (float)(rand() % 30) / 10.0f;

        // Print system state occasionally
        if (rand() % 50 == 0)
        {
            print_system_state();
        }

        Sleep(100); // Main loop interval
    }

    // Stop kernel
    kernel_stop();

    // Wait for monitor thread to terminate
    WaitForSingleObject(monitorThread, INFINITE);
    CloseHandle(monitorThread);

    // Print final statistics
    print_jitter_statistics();

    printf("\nRTOS simulation terminated.\n");
    return 0;
}

// Example task implementations
void safety_critical_task(void *args)
{
    printf("[SAFETY] Task executing critical operations\n");

    // Simulate computation
    Sleep(10);

    // Occasionally miss deadline (for testing fault detection)
    if (rand() % 100 < 2)
    {
        printf("[SAFETY] Simulating computation overrun\n");
        Sleep(150); // Sleep longer than deadline
    }
}

void control_task(void *args)
{
    printf("[CONTROL] Task executing control operations\n");

    // Simulate computation with varying execution time
    Sleep(15 + (rand() % 10));

    // Occasionally generate a computation fault
    if (rand() % 200 < 1)
    {
        printf("[CONTROL] Simulating computation fault\n");
        int result = 100 / (rand() % 2); // Potential div by zero
    }
}

void monitoring_task(void *args)
{
    printf("[MONITOR] Task collecting system data\n");

    // Simulate monitoring activities
    SystemStateVector *state = kernel_get_system_state();
    printf("[MONITOR] CPU Load: %.2f, Memory Usage: %.2f, Temp: %.1f°C\n",
           state->cpuLoad, state->memoryUsage, state->temperature);

    Sleep(20);
}

void background_task(void *args)
{
    printf("[BACKGROUND] Task performing maintenance\n");

    // Simulate background work
    Sleep(30 + (rand() % 20));
}

// System monitoring thread
DWORD WINAPI system_monitor_thread(LPVOID lpParam)
{
    while (kernel_get_running())
    {
        // Update ML model integrity periodically
        ml_model_integrity_check();

        // Periodically check overall system health
        if (rand() % 10 == 0)
        {
            SystemStateVector *state = kernel_get_system_state();
            if (state->temperature > 70.0f)
            {
                printf("WARNING: High temperature detected (%.1f°C)\n", state->temperature);
                state->state = DEGRADED_STATE;
            }
            else if (state->cpuLoad > 0.85f)
            {
                printf("WARNING: High CPU load detected (%.2f)\n", state->cpuLoad);
                state->state = DEGRADED_STATE;
            }
            else
            {
                state->state = NORMAL_STATE;
            }
        }

        Sleep(500); // Monitor interval
    }

    return 0;
}

// Utility function implementations
void print_system_state(void)
{
    SystemStateVector *state = kernel_get_system_state();
    printf("\nSystem State:\n");
    printf("  CPU Load: %.2f\n", state->cpuLoad);
    printf("  Memory Usage: %.2f\n", state->memoryUsage);
    printf("  Temperature: %.1f°C\n", state->temperature);
    printf("  Power Consumption: %.1fW\n", state->powerConsumption);
    printf("  Active Tasks: %u\n", state->activeTaskCount);
    printf("  System Mode: %s\n",
           state->state == NORMAL_STATE ? "NORMAL" : (state->state == RECOVERY_STATE ? "RECOVERY" : "DEGRADED"));
    printf("  Scheduling Jitter: %u ns\n", scheduler_get_jitter_ns());
}

void print_jitter_statistics(void)
{
    printf("\nPerformance Statistics:\n");
    printf("  Worst-case scheduling jitter: %u ns\n", scheduler_get_jitter_ns());
    // In a real system, we would have more statistics here
}

void inject_random_faults(void)
{
    // Get task count
    int count;
    Task *tasks = kernel_get_tasks(&count);

    if (count == 0)
        return;

    // Select random task and fault type
    uint32_t taskId = rand() % count;
    FaultType faultType = (FaultType)((rand() % 5) + 1); // 1-5 (skipping NO_FAULT)

    // Inject fault
    printf("\nInjecting %s fault into task '%s'...\n",
           faultType == TIMING_FAULT ? "TIMING" : faultType == MEMORY_FAULT      ? "MEMORY"
                                              : faultType == COMPUTATION_FAULT   ? "COMPUTATION"
                                              : faultType == COMMUNICATION_FAULT ? "COMMUNICATION"
                                                                                 : "POWER",
           tasks[taskId].name);

    fault_inject(faultType, taskId, 0x1000 + (rand() % 0x1000));
}
