# CyberDog2 铁蛋2 Ubuntu24 + ROS2 Jazzy + gz-sim 仿真仓库 README

## 项目简介

本仓库是**小米CyberDog 铁蛋2**四足机器狗仿真移植工程，基于官方原版仿真源码深度改造，完整适配 `Ubuntu 24.04`、`ROS 2 Jazzy`、新版gz-sim（原Ignition Gazebo/Jetty），解决旧版Gazebo Classic兼容性问题、模型加载缺失、TF关节断流、激光雷达无数据、足端悬空`Robot is lifted`等全套仿真适配bug，可直接编译运行完整机器狗仿真环境。

## 环境依赖

### 系统环境
- 操作系统：Ubuntu 24.04 LTS
- ROS版本：ROS 2 Jazzy Jalisco
- 仿真器：gz-sim Jetty 10.x

### 第三方库实测版本（编译通过）
| 库名称 | 版本号 |
|--------|--------|
| Eigen3 | 5.0.1 |
| yaml-cpp | 0.9.0（编译为动态库） |
| LCM | 1.5.2 |

### 一、APT基础软件包
```bash
sudo apt update && sudo apt install -y \
ros-jazzy-desktop-full \
ros-jazzy-ros-gz \
ros-jazzy-ros-gz-bridge \
ros-jazzy-xacro \
ros-jazzy-joint-state-publisher \
ros-jazzy-robot-state-publisher \
libgz-sim-dev git build-essential cmake libgl1-mesa-dev \
python3-colcon-common-extensions
```

### 二、手动编译安装第三方库（必装）
> 运动控制器依赖 LCM、yaml-cpp、Eigen，APT源版本较低，必须使用源码编译安装对应版本

#### 1. 安装 LCM 1.5.2
```bash
git clone https://github.com/lcm-proj/lcm.git
cd lcm
mkdir build && cd build
cmake ..
make -j$(nproc)
sudo make install
sudo ldconfig
cd ../..
```

#### 2. 安装 yaml-cpp 0.9.0（编译为动态库）
```bash
git clone https://github.com/jbeder/yaml-cpp.git
cd yaml-cpp
mkdir build && cd build
cmake .. -DYAML_BUILD_SHARED_LIBS=ON
make -j$(nproc)
sudo make install
sudo ldconfig
cd ../..
```

#### 3. 安装 Eigen3 5.0.1
```bash
git clone https://gitlab.com/libeigen/eigen.git
cd eigen
mkdir build && cd build
cmake ..
sudo make install
sudo ldconfig
cd ../..
```

## 仓库克隆
```bash
mkdir -p ~/jazzy_ws/src
cd ~/jazzy_ws/src
git clone https://github.com/shazixuezhang/cyberdog2_Jazzy.git
cd ..
```

## 源码编译指令
采用合并安装+软链接编译，仅编译运动控制与仿真核心包，编译命令：
```bash
source /opt/ros/jazzy/setup.bash
colcon build --merge-install --symlink-install --packages-up-to cyberdog_locomotion cyberdog_simulator
```

编译完成后刷新环境变量：
```bash
source install/setup.bash
```

## 仿真启动方式
### 启动脚本一键运行（推荐）
```bash
python3 src/cyberdog_sim/cyberdog_simulator/cyberdog_gazebo/script/launchsim.py
```
脚本内置：gz-sim世界加载、机器人模型生成、ros_gz_bridge话题桥、四足共享内存控制器后台自启动、雷达/IMU传感器自动使能。

### 备选launch启动（手动可调参数）
```bash
# 带激光雷达
ros2 launch cyberdog_gazebo gazebo.launch.py use_lidar:=true
# 不带激光雷达
ros2 launch cyberdog_gazebo gazebo.launch.py use_lidar:=false
```

## 仿真功能说明
1. **硬件仿真全覆盖**
   - 四足12自由度运动解算，自研`LeggedPlugin`共享内存和运动控制程序通信；
   - IMU惯性传感器、360°激光雷达完整仿真；
   - 足部接触力检测插件，解决薄地面足端悬空误判；
2. **ROS2话题桥接**
   - `/joint_states`：四肢关节实时角度（RViz完整TF树，无红色变换报错）
   - `/imu`：仿真惯性数据
   - `/scan`：激光雷达点云数据
   - `/yaml_parameter`：动态下发控制器参数至运动程序
   - `/clock`：仿真时钟同步，RViz无时间戳丢弃
3. **动力学优化**
   - world使用加厚实体方块地面，消除`Robot is lifted`悬空告警；
   - 连杆摩擦、自碰撞、视觉材质适配gz-sim规范；
   - 全部旧版`gazebo::`插件命名改为`gz::sim::`，兼容Jetty仿真内核；
4. **可视化支持**
   启动RViz可完整渲染铁蛋2整机机身、四条腿部、雷达、多相机坐标系，TF变换全部正常。

## 常见问题排查
1. 报错`Unable to find uri[model://ground_plane]`
   仓库内置适配world，已移除model引用，使用自定义实体地面，无需下载官方模型库；
2. RViz四肢提示`No transform`
   确认编译完整、环境变量已source、ros_gz_bridge正常启动，world加载全局JointState插件；
3. 无`/scan`激光雷达话题
   启动指令添加`use_lidar:=true`，world已开启Ogre2渲染引擎支持gpu_lidar；
4. 编译报错找不到 lcm / yaml-cpp / Eigen
   确认库版本严格匹配上表，执行编译安装后运行`sudo ldconfig`更新系统动态库缓存；
5. git push推送提示non-fast-forward
   执行分支上游绑定：`git branch --set-upstream-to=cyberdog2_Jazzy main`后再pull/push。

## 开源协议
Apache License 2.0，基于小米CyberDog官方仿真源码二次修改，仅供学习研发使用。