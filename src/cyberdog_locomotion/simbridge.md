## simbridge 模块详细介绍

`simbridge` 是 **CyberDog 仿真系统的核心桥接模块**，负责建立模拟器（Gazebo）与机器人控制器之间的通信桥梁。它是真实硬件桥接（HardwareBridge）的仿真版本。

### 一、模块定位与架构

```
┌─────────────────┐    LCM通信    ┌─────────────────────┐
│   遥控器/上位机   │◄─────────────►│   SimulationBridge  │
│  (LCM消息)       │               │   (控制命令处理)     │
└─────────────────┘               └────────────┬────────┘
                                               │
                                               │ 共享内存
                                               ▼
┌─────────────────┐    共享内存    ┌─────────────────────┐
│   Gazebo仿真器   │◄─────────────►│   legged_plugin     │
│  (物理模拟)      │               │   (传感器数据采集)   │
└─────────────────┘               └─────────────────────┘
```

### 二、文件结构

| 文件 | 类型 | 功能描述 |
|------|------|----------|
| [CMakeLists.txt](file:///home/lq/ros2_ws/src/cyberdog_sim/cyberdog_locomotion/simbridge/CMakeLists.txt) | 构建配置 | 编译为共享库 `libsimbridge.so`，依赖 Eigen3、lcm、OpenGL 等 |
| [shared_memory.hpp](file:///home/lq/ros2_ws/src/cyberdog_sim/cyberdog_locomotion/simbridge/include/shared_memory.hpp) | 工具类 | POSIX 共享内存和信号量封装，实现跨进程通信 |
| [simulator_message.hpp](file:///home/lq/ros2_ws/src/cyberdog_sim/cyberdog_locomotion/simbridge/include/simulator_message.hpp) | 数据结构 | 定义模拟器与控制器之间交换的所有消息格式 |
| [simulation_bridge.hpp](file:///home/lq/ros2_ws/src/cyberdog_sim/cyberdog_locomotion/simbridge/include/simulation_bridge.hpp) | 核心类头文件 | `SimulationBridge` 类定义 |
| [simulation_bridge_interface.hpp](file:///home/lq/ros2_ws/src/cyberdog_sim/cyberdog_locomotion/simbridge/include/simulation_bridge_interface.hpp) | 接口类 | 对外暴露的简单接口，封装 `SimulationBridge` |
| [simulation_bridge.cpp](file:///home/lq/ros2_ws/src/cyberdog_sim/cyberdog_locomotion/simbridge/src/simulation_bridge.cpp) | 核心实现 | `SimulationBridge` 的完整实现 |

### 三、核心组件详解

#### 1. SharedMemory（共享内存）

位于 [shared_memory.hpp](file:///home/lq/ros2_ws/src/cyberdog_sim/cyberdog_locomotion/simbridge/include/shared_memory.hpp)，提供：

- **SharedMemorySemaphore**：POSIX 信号量封装，用于进程同步
- **SharedMemoryObject<T>**：模板类，支持：
  - `CreateNew()`：创建新的共享内存对象
  - `Attach()`：连接到已存在的共享内存
  - `CloseNew()` / `Detach()`：关闭共享内存
  - 信号量同步机制：`WaitForSimulator()` / `SimulatorIsDone()` / `WaitForRobot()` / `RobotIsDone()`

共享内存名称固定为 `"development-simulator"`。

#### 2. SimulatorMessage（消息结构）

位于 [simulator_message.hpp](file:///home/lq/ros2_ws/src/cyberdog_sim/cyberdog_locomotion/simbridge/include/simulator_message.hpp)：

```cpp
struct SimulatorMessage {
    RobotToSimulatorMessage robotToSim;      // 控制器 → 模拟器
    SimulatorToRobotMessage simToRobot;      // 模拟器 → 控制器
};
```

**SimulatorToRobotMessage**（模拟器发送给控制器）：
- `gamepadCommand`：游戏手柄命令
- `robotType`：机器人类型
- `vectorNav`：IMU 数据（四元数、角速度、加速度）
- `cheater_state`：作弊状态（用于调试）
- `spiData`：腿部关节数据（位置、速度、力矩）
- `controlParameterRequest`：控制参数请求
- `mode`：仿真模式（`kRunContorlParameters` / `kRunContorller` / `kDoNothing` / `kExit`）

**RobotToSimulatorMessage**（控制器发送给模拟器）：
- `robotType`：机器人类型
- `spiCommand`：腿部关节控制命令
- `visualizationData`：可视化数据
- `mainCyberdog2Visualization`：CyberDog2 专属可视化数据
- `controlParameterResponse`：控制参数响应
- `errorMessage`：错误信息

#### 3. SimulationBridge（核心桥接类）

位于 [simulation_bridge.hpp](file:///home/lq/ros2_ws/src/cyberdog_sim/cyberdog_locomotion/simbridge/include/simulation_bridge.hpp) 和 [simulation_bridge.cpp](file:///home/lq/ros2_ws/src/cyberdog_sim/cyberdog_locomotion/simbridge/src/simulation_bridge.cpp)：

**主要功能**：
- 通过共享内存与 Gazebo 的 `legged_plugin` 通信
- 通过 LCM 接收外部控制命令（遥控器、上位机）
- 运行机器人控制器（`RobotRunnerInterface`）
- 处理控制参数的获取和设置

**LCM 通信通道**：
| LCM Channel | 用途 |
|-------------|------|
| `robot_control_cmd` | 机器人控制命令 |
| `motion-list` | 轨迹命令 |
| `user_gait_file` | 用户步态文件 |
| `motor_ctrl` | 电机控制命令 |

**主循环流程**（`Run()` 函数）：
1. 连接共享内存
2. 等待模拟器信号（`WaitForSimulator()`）
3. 根据仿真模式执行相应操作：
   - `kRunContorlParameters`：处理控制参数请求
   - `kRunContorller`：运行机器人控制器
   - `kDoNothing`：仅检查连接
   - `kExit`：退出
4. 发送完成信号（`RobotIsDone()`）

### 四、数据流

```
Gazebo legged_plugin
        │
        ▼ (写入共享内存)
┌──────────────────────────────────────────────────────────────┐
│              SimulatorMessage.simToRobot                      │
│  ┌───────────┐ ┌───────────┐ ┌───────────┐ ┌─────────────┐  │
│  │  IMU数据   │ │  关节状态  │ │ 游戏手柄   │ │ 控制参数请求 │  │
│  └─────┬─────┘ └─────┬─────┘ └─────┬─────┘ └──────┬──────┘  │
└────────│─────────────│─────────────│───────────────│──────────┘
         │             │             │               │
         ▼             ▼             ▼               ▼
┌──────────────────────────────────────────────────────────────┐
│                  SimulationBridge.Run()                       │
│  ┌─────────────────────────────────────────────────────────┐ │
│  │              RobotRunnerInterface.Run()                 │ │
│  │  ┌──────────┐  ┌──────────┐  ┌──────────────────────┐   │ │
│  │  │ 状态估计 │→ │ 运动规划 │→ │ 关节控制器(MPC等)   │   │ │
│  │  └──────────┘  └──────────┘  └──────────┬───────────┘   │ │
│  └─────────────────────────────────────────│────────────────┘ │
└────────────────────────────────────────────│───────────────────┘
         │                                  │
         ▼                                  ▼
┌──────────────────────────────────────────────────────────────┐
│              SimulatorMessage.robotToSim                      │
│  ┌───────────┐ ┌───────────────────────┐ ┌───────────────┐   │
│  │ 关节控制命令│ │ 可视化数据(调试用)     │ │ 参数响应      │   │
│  └─────┬─────┘ └───────────┬───────────┘ └───────────────┘   │
└────────│───────────────────│──────────────────────────────────┘
         │                   │
         ▼                   ▼
Gazebo legged_plugin      可视化工具
(执行关节命令)            (RViz等)
```

### 五、编译与使用

**编译**：
```bash
colcon build --packages-select cyberdog_locomotion
```

**生成产物**：
- `libsimbridge.so`：共享库，被 `cyberdog_control` 程序加载

**启动方式**：
通过 `launchcontrol.sh` 脚本启动：
```bash
cd cyberdog_locomotion/lib && ./cyberdog_control m s
```
其中 `m` 表示电机模式，`s` 表示仿真模式。

### 六、设计特点

1. **双通信机制**：
   - 共享内存：用于高频、低延迟的传感器数据和控制命令交换（1kHz 以上）
   - LCM：用于低频的控制指令和配置（如步态切换、参数调整）

2. **同步机制**：
   使用 POSIX 信号量确保模拟器和控制器之间的严格同步，避免数据竞争。

3. **参数管理**：
   支持运行时动态调整机器人参数（如 PID 参数、步态参数），无需重启仿真。

4. **错误处理**：
   支持将错误信息写入共享内存，便于调试。

### 七、与其他模块的关系

- **cyberdog_gazebo/legged_plugin**：通过共享内存与 simbridge 通信，提供传感器数据并执行控制命令
- **cyberdog_locomotion/control**：包含机器人控制器核心算法（MPC、状态估计等）
- **cyberdog_visual**：通过 LCM 接收可视化数据，在 RViz 中显示

这个模块是整个仿真系统的**神经中枢**，确保了物理仿真与高级控制算法之间的高效、可靠通信。