```markdown
# CyberDog Web Controller

基于 Web 的 CyberDog 机器狗遥控器，支持手机浏览器远程控制，自适应竖屏/横屏布局。

## 功能特性

- 📱 **手机浏览器控制**：无需安装 APP，扫码或输入网址即可控制
- 🕹️ **双摇杆操作**：左摇杆控制前后左右移动，右摇杆控制原地转向
- 🎮 **一键模式切换**：恢复站立、行走、站立、阻尼四种模式
- 🛑 **紧急停止**：醒目红色按钮，一键急停并恢复站立
- 📶 **实时状态反馈**：ROS 连接、LCM 连接、当前模式一目了然
- 📐 **自适应布局**：竖屏/横屏自动切换布局，充分利用屏幕空间
- 🔌 **优雅退出**：Ctrl+C 自动发送趴下指令，保护机器人

## 系统架构

```
手机浏览器
    │  HTTP :8080 (加载页面)
    │  WebSocket :8765 (实时控制)
    ▼
Python Web 服务器
    │  LCM publish
    ▼
Gazebo 仿真 / CyberDog 真机
```

## 依赖

### 系统依赖

| 依赖 | 版本 |
|------|------|
| ROS 2 | Jazzy |
| LCM | 1.5+（本地编译） |
| Python | 3.12 |

### Python 包

| 包名 | 版本 | 用途 |
|------|------|------|
| `websockets` | 16.1.1 | WebSocket 通信 |
| `catkin_pkg` | 1.1.0 | ROS 2 编译支持 |
| `pyyaml` | 6.0.3 | YAML 解析 |
| `setuptools` | 83.0.0 | Python 打包工具 |
| `lark-parser` | 0.12.0 | 语法解析（ROS 2 依赖） |
| `numpy` | 2.5.1 | 数值计算（ROS 2 依赖） |
| `netifaces` | 0.11.0 | 网络接口检测 |
| `jinja2` | 3.1.6 | 模板引擎（ROS 2 依赖） |
| `markupsafe` | 3.0.3 | Jinja2 依赖 |
| `docutils` | 0.23 | 文档工具 |
| `pyparsing` | 3.3.2 | 解析工具 |
| `python-dateutil` | 2.9.0 | 日期处理 |
| `typeguard` | 4.5.2 | 类型检查（ROS 2 依赖） |
| `typing_extensions` | 4.16.0 | 类型扩展 |
| `six` | 1.17.0 | Python 2/3 兼容 |
| `packaging` | 26.2 | 包版本管理 |
| `lcm` | - | LCM 通信（本地编译） |

## 安装

### 1. 创建虚拟环境

```bash
python3 -m venv ~/cyberdog_venv
source ~/cyberdog_venv/bin/activate
```

### 2. 安装 Python 包

```bash
pip install \
    websockets \
    catkin_pkg \
    pyyaml \
    setuptools \
    lark-parser \
    numpy \
    netifaces \
    jinja2 \
    markupsafe \
    docutils \
    pyparsing \
    python-dateutil \
    typeguard \
    typing_extensions \
    six \
    packaging
```

### 3. 安装 LCM Python 绑定

```bash
# 假设已在 ~/lib/lcm 编译了 LCM
cp -r ~/lib/lcm/build/python/lcm ~/cyberdog_venv/lib/python3.12/site-packages/

# 验证
python3 -c "import lcm; print('LCM OK')"
```

### 4. 编译功能包

```bash
cd ~/ros2_ws
colcon build --merge-install --symlink-install --packages-select cyberdog_web_controller
```

## 运行

### 1. 启动仿真环境

```bash
ros2 launch cyberdog_gazebo cyberdog_gazebo.launch.py
```

### 2. 启动 Web 控制器

```bash
source ~/cyberdog_venv/bin/activate
ros2 launch cyberdog_web_controller web_controller.launch.py
```

启动成功后显示：
```
========================================
  📱 手机访问: http://192.168.x.x:8080
========================================
```

### 3. 手机连接

确保手机与机器狗连接同一 WiFi，浏览器打开显示的地址即可。

> **注意**：首次使用需先运行一次键盘控制程序初始化 LCM 通道。

## 操作说明

| 操作 | 说明 |
|------|------|
| 左摇杆 | 控制前后、左右移动 |
| 右摇杆 | 控制原地左转、右转 |
| 速度滑块 | 0-100% 速度调节 |
| 🔄 恢复 | 从趴下状态恢复站立（首次必点） |
| 🚶 行走 | 进入行走模式，解锁摇杆控制 |
| 🧍 站立 | QP 站立模式 |
| 🌊 阻尼 | 缓慢趴下 |
| 🛑 急停 | 紧急停止并恢复站立 |
| Ctrl+C | 退出并自动趴下 |

## 使用流程

```
启动仿真 → 启动 Web 控制器 → 手机打开网页
    → 点击 🔄恢复 → 点击 🚶行走 → 摇杆控制移动
    → 结束按 Ctrl+C（自动趴下）
```

## 文件结构

```
cyberdog_web_controller/
├── CMakeLists.txt                          # 编译配置
├── package.xml                             # 包描述
├── README.md                               # 本文件
├── launch/
│   └── web_controller.launch.py            # 启动文件
├── static/
│   └── cyberdog_control.html               # 网页控制界面
├── scripts/
│   └── web_controller_server.py            # 入口脚本
└── cyberdog_web_controller/
    ├── __init__.py
    ├── controller_bridge.py                # 核心控制逻辑
    └── gamepad_lcmt.py                    # LCM 消息编解码
```

## 常见问题

### 网页打不开
- 确认手机和机器狗在同一网络
- 检查防火墙：`sudo ufw allow 8080`

### LCM 未连接
- 先运行键盘控制程序初始化 LCM 通道
- 检查 LCM 是否安装：`python3 -c "import lcm; print('OK')"`

### 机器狗没反应
- 确认已点击"恢复"和"行走"按钮
- 查看日志是否有 `Control error`
