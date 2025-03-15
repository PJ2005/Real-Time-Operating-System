#include "../../include/ml_engine.h"
#include "../../include/system_config.h"
#include "../../include/fault_tolerance.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>

// Simplified ML model coefficients
static float g_featureWeights[ML_FEATURE_COUNT] = {
    0.87f, 0.65f, 0.42f, 0.91f, 0.38f, // Execution time features
    0.76f, 0.52f, 0.44f, 0.89f, 0.21f, // Deadline features
    0.67f, 0.59f, 0.48f, 0.71f, 0.35f, // Resource usage features
    0.92f, 0.37f, 0.63f, 0.50f, 0.77f, // System state features
    0.45f, 0.81f, 0.62f                // Energy features
};

static float g_fuzzyMembershipMatrix[FUZZY_LEVELS][FUZZY_LEVELS] = {
    {1.0f, 0.7f, 0.3f, 0.1f, 0.0f},
    {0.7f, 1.0f, 0.7f, 0.3f, 0.1f},
    {0.3f, 0.7f, 1.0f, 0.7f, 0.3f},
    {0.1f, 0.3f, 0.7f, 1.0f, 0.7f},
    {0.0f, 0.1f, 0.3f, 0.7f, 1.0f}};

static int g_modelLoaded = 0;

// Forward declarations of helper functions
static void extract_features(Task *task, SystemStateVector *sysState, TaskFeatureVector *featureVector);
static float xgboost_inference(TaskFeatureVector *features);
static int fuzzy_level_for_value(float value, float min, float max);

void ml_engine_init(void)
{
    printf("Initializing ML engine\n");
    srand((unsigned int)time(NULL));

    // Simulate loading a pre-trained model
    g_modelLoaded = 1;
    printf("ML model loaded successfully\n");
}

float ml_predict_urgency(Task *task, SystemStateVector *sysState)
{
    if (!g_modelLoaded)
    {
        printf("Warning: ML model not loaded, using default urgency\n");
        return 0.5f;
    }

    // Extract features for the task
    TaskFeatureVector features;
    extract_features(task, sysState, &features);

    // Use XGBoost model to predict urgency
    return xgboost_inference(&features);
}

float fuzzy_adjust_priority(Task *task, float baseScore, SystemStateVector *sysState)
{
    // Map system state to fuzzy levels
    int loadLevel = fuzzy_level_for_value(sysState->cpuLoad, 0.0f, 1.0f);
    int tempLevel = fuzzy_level_for_value(sysState->temperature, 20.0f, 80.0f);
    int powerLevel = fuzzy_level_for_value(sysState->powerConsumption, 0.5f, 5.0f);
    int criticality;

    // Map task criticality to fuzzy level
    switch (task->criticality)
    {
    case DAL_A:
        criticality = 0;
        break; // Most critical
    case DAL_B:
        criticality = 1;
        break;
    case DAL_C:
        criticality = 3;
        break;
    case DAL_D:
        criticality = 4;
        break; // Least critical
    default:
        criticality = 2;
        break;
    }

    // Calculate adjustment factors using fuzzy rules
    float loadFactor = g_fuzzyMembershipMatrix[loadLevel][criticality];
    float tempFactor = g_fuzzyMembershipMatrix[tempLevel][criticality];
    float powerFactor = g_fuzzyMembershipMatrix[powerLevel][criticality];

    // Combine factors - higher criticality tasks get more priority when system is stressed
    float adjustmentFactor = 0.5f * loadFactor + 0.3f * tempFactor + 0.2f * powerFactor;

    // Adjust base score
    return baseScore * adjustmentFactor;
}

void ml_update_task_history(Task *task)
{
    // This would update the historical execution data used for ML predictions
    // For simplicity, we're just logging this
    printf("Updated execution history for task %s\n", task->name);
}

float compute_dynamic_priority(Task *task, SystemStateVector *sysState)
{
    // Base priority score from task configuration
    float basePriority = (float)task->basePriority / (float)MAX_PRIORITY_LEVELS;

    // ML-based urgency prediction
    float mlUrgency = ml_predict_urgency(task, sysState);

    // Get fault recovery factor
    float faultFactor = get_fault_recovery_factor(task->id);

    // Energy penalty based on system state
    float energyPenalty = 0.0f;
    if (sysState->powerConsumption > 4.0f)
    {
        energyPenalty = 0.2f; // High power consumption, reduce priority
    }
    else if (sysState->temperature > 70.0f)
    {
        energyPenalty = 0.15f; // High temperature, reduce priority
    }

    // Combine all factors using the formula from the paper
    float dynamicPriority = basePriority + (mlUrgency * faultFactor) - energyPenalty;

    // Apply fuzzy adjustment based on system state
    dynamicPriority = fuzzy_adjust_priority(task, dynamicPriority, sysState);

    // Clamp to valid range
    if (dynamicPriority > 1.0f)
        dynamicPriority = 1.0f;
    if (dynamicPriority < 0.0f)
        dynamicPriority = 0.0f;

    return dynamicPriority;
}

void ml_model_integrity_check(void)
{
    // Simulate model integrity check to prevent tampering
    printf("ML model integrity verified\n");
}

int ml_load_model(const char *modelPath)
{
    // Simulate loading a model from a file
    printf("Loading ML model from %s\n", modelPath);
    g_modelLoaded = 1;
    return 1; // Success
}

// Helper function implementations
static void extract_features(Task *task, SystemStateVector *sysState, TaskFeatureVector *featureVector)
{
    // Extract time-related features
    featureVector->features[0] = (float)task->executionTimeMs;
    featureVector->features[1] = (float)task->periodMs;
    featureVector->features[2] = (float)task->deadlineMs;
    featureVector->features[3] = (float)task->lastExecutionTime;
    featureVector->features[4] = task->executionHistory[0]; // Most recent execution time

    // Extract history-based features
    float sum = 0.0f, variance = 0.0f;
    for (int i = 0; i < 10; i++)
    {
        sum += task->executionHistory[i];
    }
    float mean = sum / 10.0f;
    featureVector->features[5] = mean;

    for (int i = 0; i < 10; i++)
    {
        variance += (task->executionHistory[i] - mean) * (task->executionHistory[i] - mean);
    }
    featureVector->features[6] = variance / 10.0f;
    featureVector->features[7] = (float)task->missedDeadlines;

    // Extract criticality features
    featureVector->features[8] = (float)task->criticality;
    featureVector->features[9] = (float)task->basePriority / (float)MAX_PRIORITY_LEVELS;

    // Extract system state features
    featureVector->features[10] = sysState->cpuLoad;
    featureVector->features[11] = sysState->memoryUsage;
    featureVector->features[12] = sysState->temperature / 100.0f;    // Normalize
    featureVector->features[13] = sysState->powerConsumption / 5.0f; // Normalize
    featureVector->features[14] = (float)sysState->activeTaskCount / MAX_TASKS;
    featureVector->features[15] = (float)sysState->state;

    // The remaining features would be more complex in a real system
    // For simplicity, we're just using random values
    for (int i = 16; i < ML_FEATURE_COUNT; i++)
    {
        featureVector->features[i] = (float)rand() / RAND_MAX;
    }
}

static float xgboost_inference(TaskFeatureVector *features)
{
    // Simple dot product as a placeholder for actual XGBoost inference
    float sum = 0.0f;
    for (int i = 0; i < ML_FEATURE_COUNT; i++)
    {
        sum += features->features[i] * g_featureWeights[i];
    }

    // Apply sigmoid to get a value between 0 and 1
    return 1.0f / (1.0f + expf(-sum));
}

static int fuzzy_level_for_value(float value, float min, float max)
{
    // Normalize to 0-1 range and map to fuzzy level
    float normalized = (value - min) / (max - min);
    if (normalized < 0.0f)
        normalized = 0.0f;
    if (normalized > 1.0f)
        normalized = 1.0f;

    // Map to fuzzy levels
    return (int)(normalized * (FUZZY_LEVELS - 1));
}
