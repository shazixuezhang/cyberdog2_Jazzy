#include <iostream>
#include <unistd.h>
#include <thread>
#include <atomic>
#include <termios.h>
#include <cmath>
#include <rclcpp/rclcpp.hpp>
#include <cyberdog_msg/msg/yaml_param.hpp>
#include <lcm/lcm-cpp.hpp>
#include "cyberdog_example/gamepad_lcmt.hpp"

// 非阻塞键盘输入
class KeyboardReader {
public:
    KeyboardReader() {
        tcgetattr(0, &old_);
        new_ = old_;
        new_.c_lflag &= ~ICANON;
        new_.c_lflag &= ~ECHO;
        tcsetattr(0, TCSANOW, &new_);
    }
    
    ~KeyboardReader() {
        tcsetattr(0, TCSANOW, &old_);
    }
    
    bool kbhit() {
        struct timeval tv = {0, 0};
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        return select(1, &fds, NULL, NULL, &tv) == 1;
    }
    
    char getch() {
        char ch;
        read(0, &ch, 1);
        return ch;
    }
    
private:
    struct termios old_, new_;
};

enum class ControlParameterValueKind : uint64_t {
    kDOUBLE = 1,
    kS64 = 2,
    kVEC_X_DOUBLE = 3,
    kMAT_X_DOUBLE = 4
};

class CyberDogController : public rclcpp::Node {
public:
    CyberDogController() : Node("cyberdog_controller") {
        // 创建发布者
        para_pub_ = this->create_publisher<cyberdog_msg::msg::YamlParam>("yaml_parameter", 10);
        
        // 初始化 LCM
        lcm_ = std::make_shared<lcm::LCM>();
        if (!lcm_->good()) {
            RCLCPP_ERROR(this->get_logger(), "LCM initialization failed!");
            rclcpp::shutdown();
            return;
        }
        
        // 切换到游戏手柄控制模式
        switchToGamepadMode();
        
        // 初始化状态
        current_speed_x_ = 0.0;
        current_speed_y_ = 0.0;
        current_yaw_rate_ = 0.0;
        
        RCLCPP_INFO(this->get_logger(), "CyberDog Controller initialized");
        printHelp();
    }
    
    ~CyberDogController() {
        // 停止机器人
        stopRobot();
    }
    
    void processKey(char key) {
        switch (key) {
            // 前后移动
            case 'w':
                current_speed_x_ = clamp(current_speed_x_ + 0.1, -1.0, 1.0);
                RCLCPP_INFO(this->get_logger(), "Forward speed: %.1f", current_speed_x_);
                break;
            case 's':
                current_speed_x_ = clamp(current_speed_x_ - 0.1, -1.0, 1.0);
                RCLCPP_INFO(this->get_logger(), "Forward speed: %.1f", current_speed_x_);
                break;
                
            // 左右移动
            case 'd':
                current_speed_y_ = clamp(current_speed_y_ + 0.1, -1.0, 1.0);
                RCLCPP_INFO(this->get_logger(), "Lateral speed: %.1f", current_speed_y_);
                break;
            case 'a':
                current_speed_y_ = clamp(current_speed_y_ - 0.1, -1.0, 1.0);
                RCLCPP_INFO(this->get_logger(), "Lateral speed: %.1f", current_speed_y_);
                break;
                
            // 转向控制
            case 'j':
                current_yaw_rate_ = clamp(current_yaw_rate_ + 0.1, -1.0, 1.0);
                RCLCPP_INFO(this->get_logger(), "Yaw rate: %.1f", current_yaw_rate_);
                break;
            case 'l':
                current_yaw_rate_ = clamp(current_yaw_rate_ - 0.1, -1.0, 1.0);
                RCLCPP_INFO(this->get_logger(), "Yaw rate: %.1f", current_yaw_rate_);
                break;
                
            // 速度调整
            case '1':  // 低速模式
                speed_multiplier_ = 0.3;
                RCLCPP_INFO(this->get_logger(), "Speed mode: LOW (%.1fx)", speed_multiplier_);
                break;
            case '2':  // 中速模式
                speed_multiplier_ = 0.6;
                RCLCPP_INFO(this->get_logger(), "Speed mode: MEDIUM (%.1fx)", speed_multiplier_);
                break;
            case '3':  // 高速模式
                speed_multiplier_ = 1.0;
                RCLCPP_INFO(this->get_logger(), "Speed mode: HIGH (%.1fx)", speed_multiplier_);
                break;
                
            // 模式切换
            case 'e':  // QP站立
                sendCommand(0, 0, 0, true, false, false, false);
                RCLCPP_INFO(this->get_logger(), "QP Stand");
                break;
            case 'r':  // 运动模式
                sendCommand(0, 0, 0, false, true, false, false);
                RCLCPP_INFO(this->get_logger(), "Locomotion mode");
                break;
            case 't':  // 纯阻尼模式
                sendCommand(0, 0, 0, false, false, true, false);
                RCLCPP_INFO(this->get_logger(), "Pure damper mode");
                break;
            case 'y':  // 恢复站立
                sendCommand(0, 0, 0, false, false, false, true);
                RCLCPP_INFO(this->get_logger(), "Recovery stand");
                break;
                
            // 停止
            case 'c':
                current_speed_x_ = 0.0;
                current_speed_y_ = 0.0;
                current_yaw_rate_ = 0.0;
                RCLCPP_INFO(this->get_logger(), "All speeds cleared");
                break;
            case ' ':  // 空格键紧急停止
                emergencyStop();
                break;
                
            // 预设动作
            case 'f':  // 前进0.5
                executePresetMove(0.5, 0.0, 0.0, 2.0);
                break;
            case 'b':  // 后退0.5
                executePresetMove(-0.5, 0.0, 0.0, 2.0);
                break;
            case 'z':  // 原地左转90度
                executePresetMove(0.0, 0.0, 1.57, 3.0);
                break;
            case 'x':  // 原地右转90度
                executePresetMove(0.0, 0.0, -1.57, 3.0);
                break;
                
            case 'h':
                printHelp();
                break;
        }
        
        // 更新控制命令
        updateControl();
    }
    
    void updateControl() {
        double x_speed = current_speed_x_ * speed_multiplier_;
        double y_speed = current_speed_y_ * speed_multiplier_;
        double yaw_rate = current_yaw_rate_ * speed_multiplier_;
        
        sendCommand(x_speed, y_speed, yaw_rate, false, false, false, false);
    }
    
    void sendCommand(double x, double y, double yaw, 
                     bool x_btn, bool y_btn, bool a_btn, bool b_btn) {
        gamepad_lcmt gamepad{};
        
        // 左摇杆：前后(x)和左右(y)移动
        gamepad.leftStickAnalog[0] = static_cast<float>(y);   // 左右移动
        gamepad.leftStickAnalog[1] = static_cast<float>(x);   // 前后移动
        
        // 右摇杆：转向
        gamepad.rightStickAnalog[0] = static_cast<float>(yaw); // 偏航角速度
        gamepad.rightStickAnalog[1] = 0.0;                     // 俯仰
        
        // 按钮状态
        gamepad.x = x_btn ? 1 : 0;
        gamepad.y = y_btn ? 1 : 0;
        gamepad.a = a_btn ? 1 : 0;
        gamepad.b = b_btn ? 1 : 0;
        
        // DEBUG
        // uint8_t buf[256];
        // gamepad.encode(buf, 0, gamepad.getEncodedSize());
        // printf("C++ encoded hex: ");
        // for(int i=0; i<20; i++) printf("%02x", ((uint8_t*)buf)[i]);
        // printf("\n");

        // 发布LCM消息
        lcm_->publish("gamepad_lcmt", &gamepad);
    }
    
    void executePresetMove(double x_speed, double y_speed, double yaw_rate, double duration) {
        RCLCPP_INFO(this->get_logger(), "Executing preset move: x=%.2f, y=%.2f, yaw=%.2f for %.1fs", 
                   x_speed, y_speed, yaw_rate, duration);
        
        auto start_time = this->now();
        rclcpp::Rate rate(50);  // 50Hz控制频率
        
        while (rclcpp::ok()) {
            auto elapsed = (this->now() - start_time).seconds();
            if (elapsed >= duration) break;
            
            sendCommand(x_speed, y_speed, yaw_rate, false, false, false, false);
            rate.sleep();
        }
        
        // 停止
        sendCommand(0, 0, 0, false, false, false, false);
        RCLCPP_INFO(this->get_logger(), "Preset move completed");
    }
    
    void emergencyStop() {
        current_speed_x_ = 0.0;
        current_speed_y_ = 0.0;
        current_yaw_rate_ = 0.0;
        sendCommand(0, 0, 0, false, false, false, true);  // 恢复站立
        RCLCPP_WARN(this->get_logger(), "EMERGENCY STOP!");
    }
    
    void stopRobot() {
        sendCommand(0, 0, 0, false, false, false, false);
        RCLCPP_INFO(this->get_logger(), "Robot stopped");
    }
    
    void printHelp() {
        std::cout << "\n===== CyberDog Control =====\n"
                  << "Movement:\n"
                  << "  w/s: forward/backward\n"
                  << "  a/d: left/right\n"
                  << "  j/l: turn left/right\n"
                  << "\nSpeed Modes:\n"
                  << "  1: Low (30%)\n"
                  << "  2: Medium (60%)\n"
                  << "  3: High (100%)\n"
                  << "\nModes:\n"
                  << "  e: QP Stand\n"
                  << "  r: Locomotion\n"
                  << "  t: Pure Damper\n"
                  << "  y: Recovery Stand\n"
                  << "\nActions:\n"
                  << "  c: Clear all speeds\n"
                  << "  Space: Emergency Stop\n"
                  << "  f: Forward 0.5m (preset)\n"
                  << "  b: Backward 0.5m (preset)\n"
                  << "  z: Turn left 90° (preset)\n"
                  << "  x: Turn right 90° (preset)\n"
                  << "\nOther:\n"
                  << "  h: Show this help\n"
                  << "  q: Quit\n"
                  << "===========================\n" << std::endl;
    }
    
private:
    void switchToGamepadMode() {
        auto param_msg = cyberdog_msg::msg::YamlParam();
        param_msg.name = "use_rc";
        param_msg.kind = static_cast<uint64_t>(ControlParameterValueKind::kS64);
        param_msg.s64_value = 0;
        param_msg.is_user = 0;
        para_pub_->publish(param_msg);
        RCLCPP_INFO(this->get_logger(), "Switched to gamepad control mode");
        sleep(1);  // 等待模式切换完成
    }
    
    template<typename T>
    T clamp(T value, T min, T max) {
        return std::max(min, std::min(max, value));
    }
    
    rclcpp::Publisher<cyberdog_msg::msg::YamlParam>::SharedPtr para_pub_;
    std::shared_ptr<lcm::LCM> lcm_;
    
    double current_speed_x_;
    double current_speed_y_;
    double current_yaw_rate_;
    double speed_multiplier_ = 0.6;  // 默认中速
};

int main(int argc, char** argv) {
    rclcpp::init(argc, argv);
    
    auto controller = std::make_shared<CyberDogController>();
    
    KeyboardReader kb;
    
    // 主循环
    const int control_rate = 50;  // 50Hz
    rclcpp::Rate rate(control_rate);
    
    while (rclcpp::ok()) {
        rclcpp::spin_some(controller);
        
        if (kb.kbhit()) {
            char key = kb.getch();
            if (key == 'q') {
                RCLCPP_INFO(controller->get_logger(), "Quitting...");
                break;
            }
            controller->processKey(key);
        }
        
        // 定期更新控制命令（保持连续控制）
        controller->updateControl();
        
        rate.sleep();
    }
    
    rclcpp::shutdown();
    return 0;
}