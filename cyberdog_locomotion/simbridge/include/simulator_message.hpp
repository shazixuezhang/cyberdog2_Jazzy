#ifndef SIMULATOR_MESSAGE_HPP_
#define SIMULATOR_MESSAGE_HPP_

#include "command_interface/gamepad_command.hpp"
#include "control_parameters/control_parameter_interface.hpp"
#include "shared_memory.hpp"
#include "sim_utilities/imu_types.hpp"
#include "sim_utilities/spine_board.hpp"
#include "sim_utilities/visualization_data.hpp"

/**
* @brief 这些消息包含了机器人程序和模拟器之间通过共享内存交换的所有数据。
* 这基本上包括了除调试日志之外的所有内容，调试日志由 LCM 处理。
*
*/

/**
 * @brief 仿真模式
 *
 */
enum class SimulatorMode {
    kRunContorlParameters,  // 运行控制参数（不运行机器人控制器，仅处理控制参数）
    kRunContorller,         // 运行机器人控制器
    kDoNothing,             // 仅检查连接状态
    kExit                   // 退出
                            // 退出仿真
};

/**
 * @brief 模拟器到机器人消息
 *
 */
struct SimulatorToRobotMessage {
    GamepadCommand gamepadCommand;  // 游戏手柄命令
    RobotType      robotType;       // 机器人类型

    // imu data
    VectorNavData          vectorNav;   // IMU 数据（四元数、角速度、加速度）
    CheaterState< double > cheater_state; // 模拟器状态（是否作弊、作弊时间）

    // leg data
    SpiData spiData;    // 腿部关节数据（位置、速度、力矩）
    // TODO: remove tiboard related later
    // TiBoardData tiBoardData[4];
    ControlParameterRequest controlParameterRequest;     // 控制参数请求

    SimulatorMode mode; // 仿真模式（ kRunContorlParameters / kRunContorller / kDoNothing / kExit ）
};

/**
 * @brief 机器人到模拟器的消息
 *
 */
struct RobotToSimulatorMessage {
    RobotType  robotType;    // 机器人类型
    SpiCommand spiCommand;  // 腿部关节控制命令
    // TiBoardCommand tiBoardCommand[4];

    VisualizationData        visualizationData; // 可视化数据
    Cyberdog2Visualization   mainCyberdog2Visualization; // 主 Cyberdog2 可视化数据
    ControlParameterResponse controlParameterResponse; // 控制参数响应

    char errorMessage[ 2056 ]; // 错误信息
};

/**
 * @brief 机器人和模拟器之间的共享数据
 *
 */
struct SimulatorMessage {
    RobotToSimulatorMessage robotToSim; // 控制器 → 模拟器
    SimulatorToRobotMessage simToRobot; // 模拟器 → 控制器
};

#endif  // SIMULATOR_MESSAGE_HPP_
