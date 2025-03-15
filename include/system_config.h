#ifndef SYSTEM_CONFIG_H
#define SYSTEM_CONFIG_H

// System constants
#define MAX_TASKS 32
#define MAX_PRIORITY_LEVELS 16
#define SYSTEM_TICK_MS 1
#define SCHEDULER_PERIOD_MS 10

// ML constants
#define ML_FEATURE_COUNT 23
#define FUZZY_LEVELS 5

// Fault tolerance
#define FAULT_DETECTION_ENABLED 1
#define TMR_ENABLED 1 // Triple Modular Redundancy
#define VOTING_PERIOD_MS 10

// Task criticality levels (DAL - Design Assurance Level)
typedef enum
{
    DAL_A, // Safety-critical
    DAL_B, // Mission-critical
    DAL_C, // Important
    DAL_D  // Non-critical
} CriticalityLevel;

// System states
typedef enum
{
    NORMAL_STATE,
    RECOVERY_STATE,
    DEGRADED_STATE
} SystemState;

#endif // SYSTEM_CONFIG_H
