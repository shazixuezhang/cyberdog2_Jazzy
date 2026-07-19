#!/usr/bin/env python3
import sys, signal, subprocess, time

timeout_before_kill = 0.8
timeout_after_kill = 1.0

def clean_gazebo_all():
    print("[Clean] 优雅终止gazebo与ros_gz桥接进程")
    # 优雅关闭
    subprocess.run("pkill -f 'gz sim'", shell=True, capture_output=True)
    time.sleep(timeout_after_kill)
    # 强制杀残留
    subprocess.run("pkill -9 -f 'gz sim'", shell=True, capture_output=True)
    print("[Clean] 所有gz仿真进程清理完成")

def sig_handler(sig, frame):
    print("\n收到Ctrl+C，开始清理仿真进程...")
    time.sleep(timeout_before_kill)
    clean_gazebo_all()
    sys.exit(0)

if __name__ == "__main__":
    # 监听多种终止信号
    signal.signal(signal.SIGINT, sig_handler)
    signal.signal(signal.SIGTERM, sig_handler)
    signal.signal(signal.SIGHUP, sig_handler)

    run_cmd = ' '.join(sys.argv[1:])
    print(f"启动指令：{run_cmd}")
    # 阻塞运行仿真launch
    ret_code = subprocess.call(run_cmd, shell=True)
    # 仿真正常退出也自动清理残留
    clean_gazebo_all()
    sys.exit(ret_code)
