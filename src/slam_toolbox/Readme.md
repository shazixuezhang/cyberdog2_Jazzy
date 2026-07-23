# 源码说明

## 1.config文件夹

| 文件                              | 模式         | 用途                                                         |
| --------------------------------- | ------------ | ------------------------------------------------------------ |
| `mapper_params_online_async.yaml` | 在线异步建图 | **你需要的**。边走边建图，激光数据异步处理，最常用           |
| `mapper_params_online_sync.yaml`  | 在线同步建图 | 边走边建图，同步处理，速度慢但更精确                         |
| `mapper_params_offline.yaml`      | 离线建图     | 用录好的 rosbag 回放建图，不需要实时                         |
| `mapper_params_localization.yaml` | 纯定位       | 加载已有地图，只做 AMCL 式定位，不建图                       |
| `mapper_params_lifelong.yaml`     | 终身建图     | 长期运行，地图会自动更新和优化（合并旧区域、删除消失的障碍物） |
| `slam_toolbox_default.rviz`       | RViz 配置    | slam_toolbox 自带的 RViz 视图布局                            |

## 运行模式对应的可执行文件

```bash
# 在线异步建图（你用的）
ros2 run slam_toolbox async_slam_toolbox_node --ros-args --params-file mapper_params_online_async.yaml

# 在线同步建图
ros2 run slam_toolbox sync_slam_toolbox_node --ros-args --params-file mapper_params_online_sync.yaml

# 离线建图
ros2 run slam_toolbox offline_slam_toolbox_node --ros-args --params-file mapper_params_offline.yaml

# 纯定位
ros2 run slam_toolbox lifelong_slam_toolbox_node --ros-args --params-file mapper_params_localization.yaml
```

## 后续使用流程

```
第一次跑: online_async 建图 → 保存地图
后续使用: localization 模式加载地图做纯定位导航
```



# slam\_toolbox 源码编译说明 (ROS 2 Jazzy)

## 1. 环境概述

- **操作系统**：Ubuntu 24.04 LTS
- **ROS版本**：ROS 2 Jazzy Jalisco
- **目标包**：slam\_toolbox (官方源码)
- **工作空间**：`~/ros2_ws`

## 2. 获取源码

```bash
# 进入工作空间源码目录
cd ~/ros2_ws/src

# 克隆 slam_toolbox 的 jazzy 分支（官方维护版本）
git clone https://github.com/SteveMacenski/slam_toolbox.git -b jazzy

# 返回工作空间根目录
cd ~/ros2_ws
```

## 3. 依赖安装（按出现顺序）

`slam_toolbox` 源码编译需要以下依赖，按编译报错顺序逐一安装：

### 3.1 bondcpp (ROS 2 通信库)

```bash
sudo apt install ros-jazzy-bondcpp
```

**报错信息**：`Could not find a package configuration file provided by "bondcpp"`

### 3.2 CHOLMOD (稀疏矩阵运算库)

```bash
sudo apt install libsuitesparse-dev
```

**报错信息**：`Could NOT find CHOLMOD (missing: CHOLMOD_INCLUDE_DIR CHOLMOD_LIBRARIES)`

### 3.3 Ceres Solver (非线性优化库)

```bash
sudo apt install libceres-dev
```

**报错信息**：`Could not find a package configuration file provided by "Ceres"`

### 3.4 Eigen3 (线性代数库) —— 特殊处理

Eigen3 在 Ubuntu 24.04 中头文件路径为 `/usr/include/eigen3`，CMake 可能无法自动定位。需要在编译时手动指定路径。

**报错信息**：`fatal error: Eigen/Core: 没有那个文件或目录`

## 4. 最终编译命令

```bash
colcon build --merge-install --symlink-install --packages-up-to slam_toolbox \
  --cmake-args -DEIGEN3_INCLUDE_DIR=/usr/include/eigen3
```

### 参数说明

| 参数                                      | 作用                                 |
| :-------------------------------------- | :--------------------------------- |
| `--merge-install`                       | 将所有安装文件合并到 `install/` 目录           |
| `--symlink-install`                     | 使用符号链接而非复制文件，方便调试                  |
| `--packages-up-to slam_toolbox`         | 只编译 slam\_toolbox 及其依赖             |
| `--cmake-args -DEIGEN3_INCLUDE_DIR=...` | 手动指定 Eigen3 头文件路径，绕过 CMake 查找失败的问题 |

## 5. 完整命令历史回顾

```bash
# 1. 克隆源码（第一次未指定分支，第二次指定 jazzy）
git clone https://github.com/SteveMacenski/slam_toolbox.git
git clone https://github.com/SteveMacenski/slam_toolbox.git -b jazzy

# 2. 首次编译失败 → 缺少 bondcpp
sudo apt install ros-jazzy-bondcpp

# 3. 第二次编译失败 → 缺少 CHOLMOD
sudo apt install libsuitesparse-dev

# 4. 第三次编译失败 → 缺少 Ceres
sudo apt install libceres-dev

# 5. 第四次编译失败 → Eigen/Core 找不到
# 6. 最终编译成功（添加 Eigen3 路径参数）
colcon build --merge-install --symlink-install --packages-up-to slam_toolbox \
  --cmake-args -DEIGEN3_INCLUDE_DIR=/usr/include/eigen3
```

## 6. 编译成功后验证

```bash
# 刷新环境变量
source install/setup.bash

# 检查 slam_toolbox 是否可用
ros2 pkg list | grep slam_toolbox

# 尝试启动异步建图（示例）
ros2 launch slam_toolbox online_async_launch.py use_sim_time:=true
```

## 7. 常见问题与解决方案

| 问题               | 解决方案                                                    |
| :--------------- | :------------------------------------------------------ |
| `bondcpp` 找不到    | `sudo apt install ros-jazzy-bondcpp`                    |
| `CHOLMOD` 找不到    | `sudo apt install libsuitesparse-dev`                   |
| `Ceres` 找不到      | `sudo apt install libceres-dev`                         |
| `Eigen/Core` 找不到 | 编译时添加 `-DEIGEN3_INCLUDE_DIR=/usr/include/eigen3`        |
| 其他依赖缺失           | `rosdep install -q -y -r --from-paths src --ignore-src` |

## 8. 补充建议

如果后续还需要编译其他包，可以将 Eigen3 路径设置为环境变量，避免每次都手动指定：

```bash
echo 'export Eigen3_DIR=/usr/lib/cmake/eigen3' >> ~/.bashrc
source ~/.bashrc
```

之后编译其他包时，CMake 会自动找到 Eigen3。
