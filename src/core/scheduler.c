#include "../../include/scheduler.h"
#include "../../include/ml_engine.h"
#include "../../include/system_config.h"
#include "../../include/memory_matrix.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <Windows.h>

// External functions
extern Task *kernel_get_tasks(int *count);
extern void kernel_increment_tick(void);
extern uint64_t kernel_get_tick_count(void);

// Scheduler variables
static char g_currentPolicy[32] = "ML_HYBRID"; // Default policy
static uint32_t g_lastJitterNs = 0;
static uint32_t g_worstCaseJitterNs = 0;
static uint32_t g_schedulerRuns = 0;
static SystemStateVector g_currentSystemState = {0};
static ScheduleDecision g_lastDecision = {0};

// Local functions
static ScheduleDecision rate_monotonic_schedule(Task *tasks, int count);
static ScheduleDecision earliest_deadline_schedule(Task *tasks, int count);
static ScheduleDecision ml_schedule(Task *tasks, int count, SystemStateVector *state);
static void update_task_metrics(Task *task);

void scheduler_init(void)
{
    printf("Initializing scheduler with %s policy\n", g_currentPolicy);
    g_lastJitterNs = 0;
    g_worstCaseJitterNs = 0;
    g_schedulerRuns = 0;
    memset(&g_lastDecision, 0, sizeof(ScheduleDecision));
}

void scheduler_start(void)
{
    printf("Starting scheduler\n");
}

void scheduler_tick(void)
{
    LARGE_INTEGER startTime, endTime, frequency;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&startTime);

    // Get current tasks
    int taskCount;
    Task *tasks = kernel_get_tasks(&taskCount);

    // Simulate FPGA-accelerated scheduling decision
    ScheduleDecision decision = fpga_scheduler_decide(tasks, taskCount, &g_currentSystemState);
    g_lastDecision = decision;

    // Simulate executing the selected task
    if (decision.taskId < taskCount && tasks[decision.taskId].active)
    {
        Task *selectedTask = &tasks[decision.taskId];
        update_task_metrics(selectedTask);

        // For simulation, just print what would be executed
        printf("Executing task %s (ID: %u) on core %u for %u ms\n",
               selectedTask->name,
               selectedTask->id,
               decision.targetCore,
               decision.timeSliceMs);

        // Simulate task execution - in a real system, the task would run here
        Sleep(1); // Just a tiny sleep to simulate some work
    }

    // Calculate jitter
    QueryPerformanceCounter(&endTime);
    uint64_t elapsed_ns = (endTime.QuadPart - startTime.QuadPart) * 1000000000 / frequency.QuadPart;
    g_lastJitterNs = (uint32_t)elapsed_ns;

    if (elapsed_ns > g_worstCaseJitterNs)
    {
        g_worstCaseJitterNs = (uint32_t)elapsed_ns;
    }

    g_schedulerRuns++;
    kernel_increment_tick();
}

ScheduleDecision scheduler_next_task(void)
{
    return g_lastDecision;
}

void scheduler_update_system_state(SystemStateVector *state)
{
    memcpy(&g_currentSystemState, state, sizeof(SystemStateVector));
}

uint32_t scheduler_get_jitter_ns(void)
{
    return g_worstCaseJitterNs;
}

void scheduler_set_policy(const char *policy)
{
    strncpy(g_currentPolicy, policy, sizeof(g_currentPolicy) - 1);
    g_currentPolicy[sizeof(g_currentPolicy) - 1] = '\0';
    printf("Scheduler policy changed to %s\n", g_currentPolicy);
}

// Simulated FPGA scheduler implementation
ScheduleDecision fpga_scheduler_decide(Task *tasks, int taskCount, SystemStateVector *state)
{
    ScheduleDecision decision = {0};

    // Choose scheduling algorithm based on policy
    if (strcmp(g_currentPolicy, "RMS") == 0)
    {
        // Rate Monotonic Scheduling
        decision = rate_monotonic_schedule(tasks, taskCount);
    }
    else if (strcmp(g_currentPolicy, "EDF") == 0)
    {
        // Earliest Deadline First
        decision = earliest_deadline_schedule(tasks, taskCount);
    }
    else
    {
        // ML-based hybrid scheduling (default)
        decision = ml_schedule(tasks, taskCount, state);
    }

    return decision;
}

// Implementation of traditional scheduling algorithms
static ScheduleDecision rate_monotonic_schedule(Task *tasks, int count)
{
    ScheduleDecision decision = {0};
    int highest_priority = -1;

    for (int i = 0; i < count; i++)
    {
        if (tasks[i].active && tasks[i].periodMs > 0)
        {
            int priority = 1000000 / tasks[i].periodMs; // Higher priority for shorter period
            if (priority > highest_priority)
            {
                highest_priority = priority;
                decision.taskId = tasks[i].id;
                decision.targetCore = tasks[i].coreAffinity;
                decision.timeSliceMs = tasks[i].executionTimeMs;
            }
        }
    }

    return decision;
}

static ScheduleDecision earliest_deadline_schedule(Task *tasks, int count)
{
    ScheduleDecision decision = {0};
    uint32_t earliest_deadline = UINT32_MAX;

    for (int i = 0; i < count; i++)
    {
        if (tasks[i].active && tasks[i].deadlineMs < earliest_deadline)
        {
            earliest_deadline = tasks[i].deadlineMs;
            decision.taskId = tasks[i].id;
            decision.targetCore = tasks[i].coreAffinity;
            decision.timeSliceMs = tasks[i].executionTimeMs;
        }
    }

    return decision;
}

static ScheduleDecision ml_schedule(Task *tasks, int count, SystemStateVector *state)
{
    ScheduleDecision decision = {0};
    float highest_score = -1.0f;

    for (int i = 0; i < count; i++)
    {
        if (tasks[i].active)
        {
            // Use ML engine to compute dynamic priority
            float score = compute_dynamic_priority(&tasks[i], state);

            if (score > highest_score)
            {
                highest_score = score;
                decision.taskId = tasks[i].id;
                decision.targetCore = tasks[i].coreAffinity;
                decision.timeSliceMs = tasks[i].executionTimeMs;
            }
        }
    }

    return decision;
}

static void update_task_metrics(Task *task)
{
    // Shift execution history and add new data point
    for (int i = 9; i > 0; i--)
    {
        task->executionHistory[i] = task->executionHistory[i - 1];
    }
    task->executionHistory[0] = (float)task->lastExecutionTime;
}
