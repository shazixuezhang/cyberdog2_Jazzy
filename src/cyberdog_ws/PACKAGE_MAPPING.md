# 旧包名 → 新包名 → 新目录 映射表

## Layer 0: 基础设施

### cyberdog_protocol/（原 bridges/protocol）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `bridges/protocol`（ROS msg/srv/action） | `cyberdog_protocol_ros` | `cyberdog_protocol/cyberdog_protocol_ros/` | ROS2 消息定义 |
| `bridges/protocol`（LCM 消息） | `cyberdog_protocol_lcm` | `cyberdog_protocol/cyberdog_protocol_lcm/` | LCM 消息定义 |

### cyberdog_params/（原 bridges/params）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `bridges/params` | `cyberdog_params` | `cyberdog_params/cyberdog_params/` | toml 配置参数 |

### cyberdog_utils/（原 utils/）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `cyberdog_common` | `cyberdog_utils_common` | `cyberdog_utils/cyberdog_utils_common/` | 通用工具（日志、toml解析） |
| `cyberdog_system` | `cyberdog_utils_system` | `cyberdog_utils/cyberdog_utils_system/` | 系统接口、错误码 |
| `cyberdog_debug` | `cyberdog_utils_debug` | `cyberdog_utils/cyberdog_utils_debug/` | 调试工具 |
| `cyberdog_machine` | `cyberdog_utils_machine` | `cyberdog_utils/cyberdog_utils_machine/` | 机器信息 |
| `cyberdog_parameter` | `cyberdog_utils_parameter` | `cyberdog_utils/cyberdog_utils_parameter/` | 参数管理 |

### cyberdog_third_party/（原 third_party/）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `rapidjson` | `cyberdog_third_party_rapidjson` | `cyberdog_third_party/cyberdog_third_party_rapidjson/` | JSON 解析库 |
| `toml` | `cyberdog_third_party_toml` | `cyberdog_third_party/cyberdog_third_party_toml/` | TOML 解析库 |
| `xpack` | `cyberdog_third_party_xpack` | `cyberdog_third_party/cyberdog_third_party_xpack/` | 序列化库 |
| `zxing` | `cyberdog_third_party_zxing` | `cyberdog_third_party/cyberdog_third_party_zxing/` | 二维码识别 |
| `cpp_httplib` | `cyberdog_third_party_cpphttplib` | `cyberdog_third_party/cyberdog_third_party_cpphttplib/` | HTTP 库 |
| `filesystem` | `cyberdog_third_party_filesystem` | `cyberdog_third_party/cyberdog_third_party_filesystem/` | 文件系统库 |
| `mqttc` | `cyberdog_third_party_mqttc` | `cyberdog_third_party/cyberdog_third_party_mqttc/` | MQTT 客户端 |
| `nvinference` | `cyberdog_third_party_nvinference` | `cyberdog_third_party/cyberdog_third_party_nvinference/` | TensorRT 推理 |

---

## Layer 1: 通讯桥接

### cyberdog_bridge/（原 bridges/cyberdog_grpc + bridges/embed_protocol）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `cyberdog_grpc` | `cyberdog_bridge_grpc` | `cyberdog_bridge/cyberdog_bridge_grpc/` | APP gRPC 通讯（节点包） |
| `embed_protocol`（cyberdog_embed_protocol） | `cyberdog_bridge_can` | `cyberdog_bridge/cyberdog_bridge_can/` | CAN 通讯（库包） |

---

## Layer 2: 硬件驱动

### cyberdog_device/（原 devices/）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `device_manager` | `cyberdog_device_manager` | `cyberdog_device/cyberdog_device_manager/` | 设备管理器 |
| `cyberdog_bms` | `cyberdog_device_bms` | `cyberdog_device/cyberdog_device_bms/` | BMS 电池管理 |
| `cyberdog_led` | `cyberdog_device_led` | `cyberdog_device/cyberdog_device_led/` | LED 灯光控制 |
| `cyberdog_touch` | `cyberdog_device_touch` | `cyberdog_device/cyberdog_device_touch/` | 触摸传感器 |
| `cyberdog_uwb` | `cyberdog_device_uwb` | `cyberdog_device/cyberdog_device_uwb/` | UWB 定位 |
| `cyberdog_wifi` | `cyberdog_device_wifi` | `cyberdog_device/cyberdog_device_wifi/` | WiFi 管理 |
| `cyberdog_bluetooth` | `cyberdog_device_bluetooth` | `cyberdog_device/cyberdog_device_bluetooth/` | 蓝牙通讯 |

### cyberdog_sensor/（原 sensors/）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `sensor_manager` | `cyberdog_sensor_manager` | `cyberdog_sensor/cyberdog_sensor_manager/` | 传感器管理器 |
| `cyberdog_gps` | `cyberdog_sensor_gps` | `cyberdog_sensor/cyberdog_sensor_gps/` | GPS 定位 |
| `cyberdog_lidar` | `cyberdog_sensor_lidar` | `cyberdog_sensor/cyberdog_sensor_lidar/` | 激光雷达 |
| `cyberdog_tof` | `cyberdog_sensor_tof` | `cyberdog_sensor/cyberdog_sensor_tof/` | ToF 深度传感器 |
| `cyberdog_ultrasonic` | `cyberdog_sensor_ultrasonic` | `cyberdog_sensor/cyberdog_sensor_ultrasonic/` | 超声波传感器 |

---

## Layer 3: 业务子系统

### cyberdog_motion/（原 motion/）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `motion_manager` | `cyberdog_motion_manager` | `cyberdog_motion/cyberdog_motion_manager/` | 运控管理主节点 |
| `motion_action` | `cyberdog_motion_action` | `cyberdog_motion/cyberdog_motion_action/` | 运动动作执行引擎 |
| `motion_bridge` | `cyberdog_motion_bridge` | `cyberdog_motion/cyberdog_motion_bridge/` | LCM→ROS2 桥接 |
| `motion_utils` | `cyberdog_motion_utils` | `cyberdog_motion/cyberdog_motion_utils/` | 运控工具库 |

### cyberdog_manager/（原 manager/）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `cyberdog_manager` | `cyberdog_manager_core` | `cyberdog_manager/cyberdog_manager_core/` | 系统管理主节点 |
| `manager_base` | `cyberdog_manager_base` | `cyberdog_manager/cyberdog_manager_base/` | 管理基类 |
| `cyberdog_permissions` | `cyberdog_manager_permission` | `cyberdog_manager/cyberdog_manager_permission/` | 权限管理 |
| `low_power_consumption` | `cyberdog_manager_lowpower` | `cyberdog_manager/cyberdog_manager_lowpower/` | 低功耗管理 |
| `user_info_manager` | `cyberdog_manager_user` | `cyberdog_manager/cyberdog_manager_user/` | 用户信息管理 |
| `black_box` | `cyberdog_manager_blackbox` | `cyberdog_manager/cyberdog_manager_blackbox/` | 黑匣子 |

### cyberdog_interaction/（原 interaction/）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `cyberdog_audio` | `cyberdog_interaction_audio` | `cyberdog_interaction/cyberdog_interaction_audio/` | 语音交互 |
| `cyberdog_face` | `cyberdog_interaction_face` | `cyberdog_interaction/cyberdog_interaction_face/` | 人脸识别 |
| `cyberdog_action` | `cyberdog_interaction_action` | `cyberdog_interaction/cyberdog_interaction_action/` | 手势动作识别 |
| `cyberdog_ai_sports` | `cyberdog_interaction_aisports` | `cyberdog_interaction/cyberdog_interaction_aisports/` | AI 运动计数 |
| `cyberdog_vp_engine` | `cyberdog_interaction_vp_engine` | `cyberdog_interaction/cyberdog_interaction_vp_engine/` | 可视化编程引擎 |
| `cyberdog_vp_abilityset` | `cyberdog_interaction_vp_abilityset` | `cyberdog_interaction/cyberdog_interaction_vp_abilityset/` | 可视化编程能力集 |
| `cyberdog_vp_common` | `cyberdog_interaction_vp_common` | `cyberdog_interaction/cyberdog_interaction_vp_common/` | 可视化编程公共库 |
| `cyberdog_vp_ui` | `cyberdog_interaction_vp_ui` | `cyberdog_interaction/cyberdog_interaction_vp_ui/` | 可视化编程 UI |
| `image_transmission` | `cyberdog_interaction_video` | `cyberdog_interaction/cyberdog_interaction_video/` | 图传 |
| `connector` | `cyberdog_interaction_connector` | `cyberdog_interaction/cyberdog_interaction_connector/` | 快连 |
| `cyberdog_bluetooth_network` | `cyberdog_interaction_bluetooth_net` | `cyberdog_interaction/cyberdog_interaction_bluetooth_net/` | 蓝牙网络 |
| `cyberdog_interactive` | `cyberdog_interaction_core` | `cyberdog_interaction/cyberdog_interaction_core/` | 交互主控 |
| `cyberdog_train` | `cyberdog_interaction_train` | `cyberdog_interaction/cyberdog_interaction_train/` | AI 训练 |

---

## Layer 4: 启动编排

### cyberdog_bringup/（原 cyberdog_bringup）

| 旧包名 | 新包名 | 新路径 | 说明 |
|---|---|---|---|
| `cyberdog_bringup` | `cyberdog_bringup` | `cyberdog_bringup/cyberdog_bringup/` | 启动编排 |

---

## 依赖关系变更对照表

### find_package 变更

| 旧 find_package | 新 find_package | 影响范围 |
|---|---|---|
| `find_package(protocol REQUIRED)` | `find_package(cyberdog_protocol_ros REQUIRED)` | motion、device、sensor、bridge |
| `find_package(params REQUIRED)` | `find_package(cyberdog_params REQUIRED)` | device、sensor |
| `find_package(cyberdog_embed_protocol REQUIRED)` | `find_package(cyberdog_bridge_can REQUIRED)` | device、sensor、interaction |
| `find_package(cyberdog_common REQUIRED)` | `find_package(cyberdog_utils_common REQUIRED)` | 所有子系统 |
| `find_package(cyberdog_system REQUIRED)` | `find_package(cyberdog_utils_system REQUIRED)` | motion、device |
| `find_package(cyberdog_debug REQUIRED)` | `find_package(cyberdog_utils_debug REQUIRED)` | motion |
| `find_package(cyberdog_machine REQUIRED)` | `find_package(cyberdog_utils_machine REQUIRED)` | motion |
| `find_package(motion_manager REQUIRED)` | `find_package(cyberdog_motion_manager REQUIRED)` | motion_utils |
| `find_package(motion_action REQUIRED)` | `find_package(cyberdog_motion_action REQUIRED)` | motion_manager、motion_bridge |
| `find_package(manager_base REQUIRED)` | `find_package(cyberdog_manager_base REQUIRED)` | motion_manager |

### node.yaml 包名变更

| 旧包名 | 新包名 |
|---|---|
| `connector` | `cyberdog_interaction_connector` |
| `device_manager` | `cyberdog_device_manager` |
| `sensor_manager` | `cyberdog_sensor_manager` |
| `cyberdog_manager` | `cyberdog_manager_core` |
| `cyberdog_wifi` | `cyberdog_device_wifi` |
| `motion_manager` | `cyberdog_motion_manager` |
| `motion_bridge` | `cyberdog_motion_bridge` |
| `cyberdog_grpc` | `cyberdog_bridge_grpc` |
| `motion_action` | `cyberdog_motion_action` |
| `cyberdog_vp_engine` | `cyberdog_interaction_vp_engine` |
| `cyberdog_audio` | `cyberdog_interaction_audio` |
| `cyberdog_permission` | `cyberdog_manager_permission` |
| `cyberdog_face` | `cyberdog_interaction_face` |
| `cyberdog_ota` | `cyberdog_manager_ota`（新增） |
| `bes_transmit` | `cyberdog_bridge_bes`（新增） |
| `cyberdog_bluetooth` | `cyberdog_device_bluetooth` |
| `cyberdog_action` | `cyberdog_interaction_action` |
| `cyberdog_interactive` | `cyberdog_interaction_core` |
| `cyberdog_ai_sports` | `cyberdog_interaction_aisports` |
| `cyberdog_train` | `cyberdog_interaction_train` |
| `realsense2_camera` | `realsense2_camera`（外部包，不变） |
| `cyberdog_bluetooth_network` | `cyberdog_interaction_bluetooth_net` |

---

## 改造顺序建议

| 阶段 | 改造内容 | 验证方式 |
|---|---|---|
| **Phase 1** | cyberdog_protocol（消息定义） | `colcon build --packages-select cyberdog_protocol_ros cyberdog_protocol_lcm` |
| **Phase 2** | cyberdog_params（配置参数） | `colcon build --packages-select cyberdog_params` |
| **Phase 3** | cyberdog_utils（通用库） | `colcon build --packages-select cyberdog_utils_common cyberdog_utils_system cyberdog_utils_debug cyberdog_utils_machine cyberdog_utils_parameter` |
| **Phase 4** | cyberdog_bridge（通讯桥接） | `colcon build --packages-select cyberdog_bridge_can cyberdog_bridge_grpc` |
| **Phase 5** | cyberdog_device + cyberdog_sensor（硬件驱动） | `colcon build --packages-select cyberdog_device_manager cyberdog_device_bms ...` |
| **Phase 6** | cyberdog_motion（运控） | `colcon build --packages-select cyberdog_motion_manager ...` |
| **Phase 7** | cyberdog_manager + cyberdog_interaction（管理+交互） | `colcon build --packages-select cyberdog_manager_core ...` |
| **Phase 8** | cyberdog_bringup（启动编排） | `colcon build --packages-select cyberdog_bringup && ros2 launch ...` |
| **Phase 9** | cyberdog_third_party（第三方依赖） | `colcon build --packages-select cyberdog_third_party_rapidjson ...` |

---

## 注意事项

1. **`bes_transmit` 和 `cyberdog_ota`**：当前源码中存在但未在旧结构中找到独立包，需确认是否需要新增
2. **`cyberdog_tracking_base`**：已被用户删除，不列入映射
3. **`elec_skin` 和 `skin_manager`**：已被用户删除，不列入映射
4. **头文件路径**：所有 `#include` 路径需要同步修改，例如 `#include "cyberdog_common/cyberdog_log.hpp"` → `#include "cyberdog_utils_common/cyberdog_log.hpp"`
5. **plugin.xml**：所有插件类名和包名需要同步修改
6. **toml 配置文件**：`params` 包名变更后，`ament_index_cpp::get_package_share_directory("params")` 需要改为 `get_package_share_directory("cyberdog_params")`