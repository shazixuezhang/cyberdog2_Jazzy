#!/usr/bin/env python3
"""
CyberDog 控制桥接模块
只通过 LCM 控制，不发送错误的 ROS 2 参数
"""

import asyncio
import json
import threading
import time
from pathlib import Path
from http.server import HTTPServer, SimpleHTTPRequestHandler
import socket
import sys
import os

import rclpy
from rclpy.node import Node
from cyberdog_msg.msg import YamlParam

try:
    import lcm
    from lcm import LCM
    LCM_AVAILABLE = True
except ImportError:
    LCM_AVAILABLE = False

# 尝试导入 gamepad_lcmt（从自己的包）
try:
    from cyberdog_web_controller.gamepad_lcmt import gamepad_lcmt
    GAMEPAD_AVAILABLE = True
except ImportError:
    GAMEPAD_AVAILABLE = False
    class gamepad_lcmt:
        def __init__(self):
            self.leftStickAnalog = [0.0, 0.0]
            self.rightStickAnalog = [0.0, 0.0]
            self.x = 0
            self.y = 0
            self.a = 0
            self.b = 0
        def encode(self):
            return b''

try:
    import websockets
except ImportError:
    print("请安装 websockets: pip3 install websockets")
    sys.exit(1)


class CyberDogController(Node):
    """CyberDog 控制器节点 - 只使用 LCM 控制"""
    
    def __init__(self):
        super().__init__('cyberdog_web_controller')
        
        # ROS 2 发布者（只用于切换到 RC 模式）
        self.param_pub = self.create_publisher(YamlParam, 'yaml_parameter', 10)
        
        # LCM 初始化
        self.lcm = None
        if LCM_AVAILABLE:
            try:
                self.lcm = lcm.LCM()
                self.get_logger().info('LCM initialized')
            except Exception as e:
                self.get_logger().warn(f'LCM init failed: {e}')
        
        # 控制状态
        self.move_command = {'x': 0.0, 'y': 0.0, 'yaw': 0.0}
        self.current_mode = 'stand'
        self.running = True
        self.started = False

        self._switch_to_gamepad_mode()
        
        # 显示访问地址
        try:
            s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
            s.connect(('8.8.8.8', 80))
            ip = s.getsockname()[0]
            s.close()
        except:
            ip = 'localhost'
        
        self.get_logger().info('')
        self.get_logger().info('========================================')
        self.get_logger().info(f'  📱 手机访问: http://{ip}:8080')
        self.get_logger().info('========================================')
        self.get_logger().info('')
    
    def _switch_to_gamepad_mode(self):
        """切换到游戏手柄控制模式"""
        msg = YamlParam()
        msg.name = "use_rc"
        msg.kind = 2
        msg.s64_value = 0
        msg.is_user = 0
        time.sleep(1)
        self.param_pub.publish(msg)
        self.get_logger().info('Switched to gamepad mode')
    
    def _control_loop(self):
        """50Hz 控制循环 - 只通过 LCM 发送"""
        interval = 1.0 / 50
        
        while self.running:
            try:
                if self.lcm:
                    msg = gamepad_lcmt()
                    # 左摇杆：前后(x)和左右(y)
                    msg.leftStickAnalog[0] = float(self.move_command['y'])
                    msg.leftStickAnalog[1] = float(self.move_command['x'])
                    # 右摇杆：转向
                    msg.rightStickAnalog[0] = float(self.move_command['yaw'])
                    msg.rightStickAnalog[1] = 0.0
                    self.lcm.publish("gamepad_lcmt", msg.encode())
                time.sleep(interval)
            except Exception as e:
                self.get_logger().error(f'Control error: {e}')
    
    def set_movement(self, x, y, yaw):
        """设置移动速度"""
        self.move_command = {'x': x, 'y': y, 'yaw': yaw}
    
    def set_mode(self, mode):
        self.current_mode = mode
        
        if self.lcm:
            msg = gamepad_lcmt()
            if mode == 'stand':
                msg.x = 1
            elif mode == 'locomotion':
                msg.y = 1
            elif mode == 'damper':
                msg.a = 1
            elif mode == 'recovery':
                msg.b = 1
            self.lcm.publish("gamepad_lcmt", msg.encode())
            self.get_logger().info(f'Mode: {mode}')
        
        # 第一次收到命令时启动控制循环
        if not self.started:
            self.started = True
            self.control_thread = threading.Thread(target=self._control_loop, daemon=True)
            self.control_thread.start()
            self.get_logger().info('Control loop started')
    
    def emergency_stop(self):
        """紧急停止"""
        self.move_command = {'x': 0.0, 'y': 0.0, 'yaw': 0.0}
        self.get_logger().warn('EMERGENCY STOP!')
    
    def get_status(self):
        """获取完整状态"""
        return {
            'ros_connected': True,
            'lcm_connected': self.lcm is not None,
            'mode': self.current_mode
        }

    def shutdown(self):
        """切换为缓慢趴下模式"""
        if self.lcm:
            msg = gamepad_lcmt()
            msg.a = 1  # Pure damper 模式（缓慢趴下）
            self.lcm.publish("gamepad_lcmt", msg.encode())
            self.get_logger().info('Sent: Pure damper (slow sit down)')
        self.running = False
        time.sleep(0.5)


class WebServer:
    """Web 服务器（HTTP + WebSocket）"""
    
    def __init__(self, static_dir, http_port=8080, ws_port=8765):
        self.static_dir = Path(static_dir)
        self.http_port = http_port
        self.ws_port = ws_port
        self.controller = None
        self.clients = set()
    
    def start_http(self):
        """启动 HTTP 文件服务器"""
        os.chdir(str(self.static_dir))
        
        class Handler(SimpleHTTPRequestHandler):
            def log_message(self, *args):
                pass
        
        httpd = HTTPServer(('0.0.0.0', self.http_port), Handler)
        threading.Thread(target=httpd.serve_forever, daemon=True).start()
        print(f"HTTP: http://0.0.0.0:{self.http_port}")
    
    async def ws_handler(self, websocket):
        """WebSocket 连接处理"""
        self.clients.add(websocket)
        addr = websocket.remote_address
        print(f"📱 Client: {addr}")
        
        try:
            if self.controller:
                await websocket.send(json.dumps({
                    'type': 'status',
                    **self.controller.get_status()
                }))
            
            async for message in websocket:
                try:
                    data = json.loads(message)
                    await self._process(data, websocket)
                except Exception as e:
                    print(f"Error: {e}")
        
        except websockets.exceptions.ConnectionClosed:
            print(f"📱 Client disconnected: {addr}")
        finally:
            self.clients.discard(websocket)
    
    async def _process(self, data, ws):
        """处理控制消息"""
        msg_type = data.get('type', '')
        
        if msg_type == 'movement' and self.controller:
            self.controller.set_movement(
                data.get('x_speed', 0.0),
                data.get('y_speed', 0.0),
                data.get('yaw_rate', 0.0)
            )
        
        elif msg_type == 'mode_change' and self.controller:
            self.controller.set_mode(data.get('mode', 'stand'))
        
        elif msg_type == 'emergency_stop' and self.controller:
            self.controller.emergency_stop()
            await ws.send(json.dumps({'type': 'alert', 'message': 'Emergency Stop!'}))
        
        elif msg_type == 'heartbeat' and self.controller:
            await ws.send(json.dumps({
                'type': 'status',
                **self.controller.get_status()
            }))
    
    async def start_ws(self):
        """启动 WebSocket 服务器"""
        print(f"WebSocket: ws://0.0.0.0:{self.ws_port}")
        async with websockets.serve(self.ws_handler, '0.0.0.0', self.ws_port):
            await asyncio.Future()
    
    def run(self):
        """启动所有服务"""
        self.start_http()
        print(f"\n{'='*50}")
        print(f"  🤖 CyberDog Web Controller")
        print(f"{'='*50}\n")
        asyncio.run(self.start_ws())

def main():
    """主函数"""
    rclpy.init()
    
    controller = CyberDogController()
    
    def ros_spin():
        while rclpy.ok():
            rclpy.spin_once(controller, timeout_sec=0.01)
    
    threading.Thread(target=ros_spin, daemon=True).start()
    
    # 获取静态文件目录
    try:
        from ament_index_python.packages import get_package_share_directory
        static_dir = Path(get_package_share_directory('cyberdog_web_controller')) / 'static'
    except:
        static_dir = Path(__file__).parent.parent / 'static'
        static_dir.mkdir(exist_ok=True)
    
    server = WebServer(static_dir)
    server.controller = controller
    
    try:
        server.run()
    except KeyboardInterrupt:
        print("\nShutting down...")
    finally:
        controller.shutdown()  # 先趴下
        controller.running = False
        controller.destroy_node()
        rclpy.shutdown()