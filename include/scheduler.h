#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "task_manager.h"

// Scheduler decisions
typedef struct
{
    uint32_t taskId;
    uint8_t targetCore;
    uint32_t timeSliceMs;
} ScheduleDecision;

// System state for scheduling
typedef struct
{
    float cpuLoad;
    float memoryUsage;
    float temperature;
    float powerConsumption;
    uint32_t activeTaskCount;
    SystemState state;
} SystemStateVector;

// Function prototypes
void scheduler_init(void);
void scheduler_start(void);
void scheduler_tick(void);
ScheduleDecision scheduler_next_task(void);
void scheduler_update_system_state(SystemStateVector *state);
uint32_t scheduler_get_jitter_ns(void);
void scheduler_set_policy(const char *policy);

// Simulated FPGA scheduler interface
ScheduleDecision fpga_scheduler_decide(Task *tasks, int taskCount, SystemStateVector *state);

#endif // SCHEDULER_H
