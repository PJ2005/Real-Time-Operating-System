#ifndef MEMORY_MATRIX_H
#define MEMORY_MATRIX_H

#include <stdint.h>
#include <stddef.h>

// Shared memory region IDs
typedef enum
{
    SHM_SYSTEM_STATE,
    SHM_TASK_DESCRIPTORS,
    SHM_SCHEDULER_DECISIONS,
    SHM_FAULT_REPORTS,
    SHM_ML_DATA,
    SHM_USER_DATA,
    SHM_COUNT
} SharedMemoryRegion;

// Function prototypes
void memory_matrix_init(void);
void *memory_matrix_get_region(SharedMemoryRegion region);
int memory_matrix_write(SharedMemoryRegion region, void *data, size_t size);
int memory_matrix_read(SharedMemoryRegion region, void *buffer, size_t size);
int memory_matrix_lock(SharedMemoryRegion region);
int memory_matrix_unlock(SharedMemoryRegion region);
void memory_matrix_barrier(void);

#endif // MEMORY_MATRIX_H
