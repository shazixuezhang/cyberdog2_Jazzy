# motion_manager 功能包编译运行指南

## 概述

`motion_manager` 是 CyberDog 四足机器人的运动管理功能包，负责处理运动指令、状态机管理和伺服控制等核心功能。

## 环境要求

- Ubuntu 24.04 LTS
- ROS2 Jazzy
- Gazebo Jetty

## 依赖包

以下依赖包需要先编译：

1. `protocol` - 协议消息定义
2. `cyberdog_common` - 通用工具库
3. `cyberdog_system` - 系统基础模块
4. `cyberdog_debug` - 调试工具
5. `cyberdog_machine` - 状态机框架
6. `manager_base` - 管理器基类
7. `motion_action` - 运动动作模块
8. `params` - 参数配置包
9. `cyberdog_embed_protocol` - 嵌入式协议（编译时需禁用测试）

## 编译步骤

### 1. 设置 ROS2 环境

```bash
source /opt/ros/jazzy/setup.bash
```

### 2. 编译 params 包

```bash
cd /home/lq/ros2_ws
colcon build --packages-select params --symlink-install --merge-install
```

### 3. 编译 cyberdog_embed_protocol 包（禁用测试）

```bash
colcon build --packages-select cyberdog_embed_protocol --symlink-install --merge-install --cmake-args -DBUILD_TESTING=OFF
```

### 4. 编译 motion_manager 及其依赖

```bash
colcon build --packages-select protocol cyberdog_common cyberdog_system cyberdog_debug cyberdog_machine manager_base motion_action motion_manager --symlink-install --merge-install
```

## 运行步骤

### 1. 设置环境变量

```bash
source /opt/ros/jazzy/setup.bash
source /home/lq/ros2_ws/install/setup.bash
```

### 2. 运行 motion_manager 节点

```bash
ros2 run motion_manager motion_manager
```

## 关键话题和服务

### 话题

- `/motion_servo_cmd` - 运动伺服指令输入 (protocol/msg/MotionServoCmd)
- `/motion_status` - 运动状态反馈 (protocol/msg/MotionStatus)
- `/motion_servo_response` - 伺服指令响应 (protocol/msg/MotionServoResponse)

### 服务

- `/motion_result_cmd` - 运动结果指令服务 (protocol/srv/MotionResultCmd)
- `/motion_custom_cmd` - 自定义运动指令服务 (protocol/srv/MotionCustomCmd)
- `/motion_queue_custom_cmd` - 运动队列指令服务 (protocol/srv/MotionQueueCustomCmd)
- `/motion_sequence_show` - 运动序列展示服务 (protocol/srv/MotionSequenceShow)

## 重要配置文件

- `config/priority.toml` - 运动优先级配置
- `config/preset/motion_id_map.toml` - 运动ID映射配置（位于 motion_action 包）

## 已知问题和解决方案

### 问题 1: 找不到 `params` 包

**错误信息**:
```
package 'params' not found
```

**解决方案**:
```bash
colcon build --packages-select params --symlink-install --merge-install
```

### 问题 2: 找不到 `cyberdog_embed_protocol` 的 `local_setup.bash`

**错误信息**:
```
not found: "/home/lq/ros2_ws/install/share/cyberdog_embed_protocol/local_setup.bash"
```

**解决方案**:
```bash
colcon build --packages-select cyberdog_embed_protocol --symlink-install --merge-install --cmake-args -DBUILD_TESTING=OFF
```

### 问题 3: 运行时缺少状态机配置文件

**错误信息**:
```
fopen failed: No such file or directory
```

**解决方案**:
确保 `params` 包已正确编译，该包包含状态机配置文件 `toml_config/manager/state_machine_config.toml`。

## 使用示例

### 发布运动伺服指令

```bash
ros2 topic pub /motion_servo_cmd protocol/msg/MotionServoCmd "{motion_id: 303, vel_des: [0.3, 0, 0]}"
```

### 调用运动结果服务

```bash
ros2 service call /motion_result_cmd protocol/srv/MotionResultCmd "{motion_id: 101, contact: 15}"
```

## 注意事项

1. 在运行 `motion_manager` 之前，需要确保 Gazebo 仿真环境已经启动，并且 `/clock` 话题正在发布（使用 `use_sim_time` 参数）。
2. 该功能包依赖 LCM（Lightweight Communications and Marshalling）进行与底层控制器的通信。
3. 电子皮肤功能（`elec_skin`）和日志上传功能（`file_uploading`）已被移除，适用于仿真环境。
