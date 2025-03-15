#ifndef ML_ENGINE_H
#define ML_ENGINE_H

#include "task_manager.h"
#include "scheduler.h"

// Feature vector for ML prediction
typedef struct
{
    float features[ML_FEATURE_COUNT];
} TaskFeatureVector;

// Function prototypes
void ml_engine_init(void);
float ml_predict_urgency(Task *task, SystemStateVector *sysState);
float fuzzy_adjust_priority(Task *task, float baseScore, SystemStateVector *sysState);
void ml_update_task_history(Task *task);
float compute_dynamic_priority(Task *task, SystemStateVector *sysState);
void ml_model_integrity_check(void);
int ml_load_model(const char *modelPath);

#endif // ML_ENGINE_H
