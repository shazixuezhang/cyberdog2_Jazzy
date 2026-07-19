"""gamepad_lcmt - 匹配 LCM C++ 编码"""
import struct

class gamepad_lcmt:
    # LCM 实际使用的 hash（经过 _computeHash 处理）
    # 原 hash: 0x37c71cc8957b05cf
    # computeHash: (hash<<1) + ((hash>>63)&1)
    _hash = 0x6f8e39912af60b9e
    
    def __init__(self):
        self.leftBumper = 0
        self.rightBumper = 0
        self.leftTriggerButton = 0
        self.rightTriggerButton = 0
        self.back = 0
        self.start = 0
        self.a = 0
        self.b = 0
        self.x = 0
        self.y = 0
        self.leftStickButton = 0
        self.rightStickButton = 0
        self.leftTriggerAnalog = 0.0
        self.rightTriggerAnalog = 0.0
        self.leftStickAnalog = [0.0, 0.0]
        self.rightStickAnalog = [0.0, 0.0]
    
    def encode(self):
        """编码为 LCM 兼容格式"""
        fmt = '>q12i6f'
        return struct.pack(fmt,
            self._hash,
            self.leftBumper, self.rightBumper,
            self.leftTriggerButton, self.rightTriggerButton,
            self.back, self.start,
            self.a, self.b, self.x, self.y,
            self.leftStickButton, self.rightStickButton,
            self.leftTriggerAnalog, self.rightTriggerAnalog,
            self.leftStickAnalog[0], self.leftStickAnalog[1],
            self.rightStickAnalog[0], self.rightStickAnalog[1]
        )
