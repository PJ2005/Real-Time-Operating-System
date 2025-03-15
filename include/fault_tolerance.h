#ifndef FAULT_TOLERANCE_H
#define FAULT_TOLERANCE_H

#include "system_config.h"
#include <stdint.h>

// Fault types
typedef enum
{
    NO_FAULT,
    TIMING_FAULT,
    MEMORY_FAULT,
    COMPUTATION_FAULT,
    COMMUNICATION_FAULT,
    POWER_FAULT
} FaultType;

// Fault detection result
typedef struct
{
    uint8_t faultDetected;
    FaultType type;
    uint32_t taskId;
    uint32_t address;
    uint64_t timestamp;
} FaultDetectionResult;

// Function prototypes
void fault_tolerance_init(void);
FaultDetectionResult fault_check_system(void);
int fault_inject(FaultType type, uint32_t taskId, uint32_t address); // For testing
void fault_recovery_action(FaultDetectionResult *result);
uint8_t tmr_voting(uint32_t result1, uint32_t result2, uint32_t result3);
float get_fault_recovery_factor(uint32_t taskId);
void set_watchdog_timer(uint32_t taskId, uint32_t timeoutMs);

#endif // FAULT_TOLERANCE_H
