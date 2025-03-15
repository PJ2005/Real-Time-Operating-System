#include "../../include/memory_matrix.h"
#include "../../include/system_config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// Shared memory regions
static void *g_sharedMemory[SHM_COUNT] = {NULL};
static size_t g_regionSizes[SHM_COUNT] = {0};
static CRITICAL_SECTION g_regionLocks[SHM_COUNT];
static int g_initialized = 0;

// Define default sizes for memory regions
static const size_t DEFAULT_SIZES[SHM_COUNT] = {
    1024, // SHM_SYSTEM_STATE
    4096, // SHM_TASK_DESCRIPTORS
    1024, // SHM_SCHEDULER_DECISIONS
    2048, // SHM_FAULT_REPORTS
    8192, // SHM_ML_DATA
    16384 // SHM_USER_DATA
};

void memory_matrix_init(void)
{
    if (g_initialized)
    {
        return;
    }

    printf("Initializing memory matrix...\n");

    // Initialize critical sections for each region
    for (int i = 0; i < SHM_COUNT; i++)
    {
        InitializeCriticalSection(&g_regionLocks[i]);

        // Allocate memory for each region
        g_regionSizes[i] = DEFAULT_SIZES[i];
        g_sharedMemory[i] = malloc(g_regionSizes[i]);

        if (!g_sharedMemory[i])
        {
            printf("Error: Failed to allocate memory region %d\n", i);
            exit(1);
        }

        // Clear memory region
        memset(g_sharedMemory[i], 0, g_regionSizes[i]);
        printf("Memory region %d allocated: %zu bytes\n", i, g_regionSizes[i]);
    }

    g_initialized = 1;
    printf("Memory matrix initialized\n");
}

void *memory_matrix_get_region(SharedMemoryRegion region)
{
    if (region >= SHM_COUNT || !g_initialized || !g_sharedMemory[region])
    {
        return NULL;
    }

    return g_sharedMemory[region];
}

int memory_matrix_write(SharedMemoryRegion region, void *data, size_t size)
{
    if (region >= SHM_COUNT || !g_initialized || !g_sharedMemory[region] || !data)
    {
        return -1;
    }

    if (size > g_regionSizes[region])
    {
        printf("Error: Write size %zu exceeds region size %zu\n", size, g_regionSizes[region]);
        return -2;
    }

    EnterCriticalSection(&g_regionLocks[region]);
    memcpy(g_sharedMemory[region], data, size);
    LeaveCriticalSection(&g_regionLocks[region]);

    return 0; // Success
}

int memory_matrix_read(SharedMemoryRegion region, void *buffer, size_t size)
{
    if (region >= SHM_COUNT || !g_initialized || !g_sharedMemory[region] || !buffer)
    {
        return -1;
    }

    if (size > g_regionSizes[region])
    {
        printf("Error: Read size %zu exceeds region size %zu\n", size, g_regionSizes[region]);
        return -2;
    }

    EnterCriticalSection(&g_regionLocks[region]);
    memcpy(buffer, g_sharedMemory[region], size);
    LeaveCriticalSection(&g_regionLocks[region]);

    return 0; // Success
}

int memory_matrix_lock(SharedMemoryRegion region)
{
    if (region >= SHM_COUNT || !g_initialized)
    {
        return -1;
    }

    EnterCriticalSection(&g_regionLocks[region]);
    return 0; // Success
}

int memory_matrix_unlock(SharedMemoryRegion region)
{
    if (region >= SHM_COUNT || !g_initialized)
    {
        return -1;
    }

    LeaveCriticalSection(&g_regionLocks[region]);
    return 0; // Success
}

void memory_matrix_barrier(void)
{
    // Implement a barrier synchronization
    // Lock all regions, then unlock them
    for (int i = 0; i < SHM_COUNT; i++)
    {
        EnterCriticalSection(&g_regionLocks[i]);
    }

    // Memory barrier (in a real system this would ensure coherence)
    MemoryBarrier();

    for (int i = 0; i < SHM_COUNT; i++)
    {
        LeaveCriticalSection(&g_regionLocks[i]);
    }

    printf("Memory matrix barrier completed\n");
}
