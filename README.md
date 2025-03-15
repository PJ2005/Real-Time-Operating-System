# Hardware-Accelerated RTOS with ML-Driven Scheduling

This project simulates a novel Real-Time Operating System (RTOS) with the following key features:

1. **FPGA-accelerated scheduling infrastructure** (simulated)
2. **ML-powered adaptive task management**
3. **Hardware-enforced fault tolerance mechanisms**

## Project Structure

```
rtos/
├── include/              # Header files
│   ├── system_config.h   # System-wide configurations
│   ├── task_manager.h    # Task management interfaces
│   ├── scheduler.h       # Scheduler interfaces
│   ├── ml_engine.h       # ML inferencing engine
│   ├── fault_tolerance.h # Fault detection/recovery
│   └── memory_matrix.h   # Shared memory infrastructure
├── src/
│   ├── core/             # Core RTOS components
│   │   ├── kernel.c      # Main kernel
│   │   ├── scheduler.c   # Scheduling algorithms
│   │   ├── task_manager.c# Task handling
│   │   └── memory_matrix.c# Shared memory
│   ├── ml/
│   │   └── ml_engine.c   # ML prediction implementation
│   ├── fault/
│   │   └── fault_tolerance.c # Fault handling
│   └── main.c            # Main application
└── CMakeLists.txt        # Build configuration
```

## Building the Project

This project uses CMake to generate build files:

```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Running the Simulation

Once built, run the binary from the `build/bin` directory:

```bash
cd bin
./ml_rtos
```

## Features

- **Triple Modular Redundancy** with hardware-based voting
- **XGBoost-Fuzzy Hybrid Scheduling** for adaptive task prioritization
- **Hardware-enforced task isolation**
- **Fault detection and recovery**

## Testing

The simulation includes a fault injection mechanism that allows you to test system response to various faults:

- Press Enter during runtime to inject a random fault
- Press Q to quit the simulation

## Performance Metrics

The system tracks and reports:

- Scheduling jitter (simulated)
- Fault recovery times
- Task execution patterns
- Energy consumption estimates
