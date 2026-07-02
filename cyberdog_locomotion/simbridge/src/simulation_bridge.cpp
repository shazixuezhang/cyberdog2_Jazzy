/*! @file simulation_bridge.cpp
 *  @brief  SimulationBridge 运行 RobotController 并将其连接到
 *  使用共享内存的 Simulator。它是 HardwareBridge 的仿真版本。
 */

#include <iostream>

#include "simulation_bridge.hpp"
#include "utilities/segfault_handler.hpp"

/**
 * @brief 连接到仿真。
 *
 */
void SimulationBridge::Run() {
    // 初始化共享内存
    shared_memory_.Attach( DEVELOPMENT_SIMULATOR_SHARED_MEMORY_NAME );
    shared_memory_.Init( false );

    InstallSegfaultHandler( shared_memory_().robotToSim.errorMessage );

    // 初始化四足机器人控制器
    try {
        printf( "[Simulation Driver] 开始主循环（...\n" );
        bool first_run = true;
        for ( ;; ) {
            // 等待轮到我们访问共享内存
            // 在第一次循环中，这给模拟器一个机会在我们开始之前将内容放入共享内存
            shared_memory_.WaitForSimulator();

            if ( first_run ) {
                first_run = false;
                // 检查机器人类型是否正确
                if ( robot_ != shared_memory_().simToRobot.robotType ) {
                    printf( "模拟器和仿真驱动 不同意我们正在模拟的机器人类型（机器人 %d，仿真 %d）\n",
                            ( int )robot_, ( int )shared_memory_().simToRobot.robotType );
                    throw std::runtime_error( "机器人类型不匹配" );
                }
            }

            // 模拟器告诉我们要运行的仿真模式
            sim_mode_ = shared_memory_().simToRobot.mode;
            switch ( sim_mode_ ) {
            case SimulatorMode::kRunContorlParameters:  // 有新的控制参数请求
                // 处理控制参数请求
                HandleControlParameters();
                break;
            case SimulatorMode::kRunContorller:  // 模拟器准备运行下一个机器人控制器迭代
                iterations_++;
                RunRobotControl();
                break;
            case SimulatorMode::kDoNothing:  // 模拟器仅检查连接状态，不运行机器人控制器
                break;
            case SimulatorMode::kExit:  // 模拟器完成与我们的交互
                printf( "[Simulation Driver] 退出仿真\n" );
                return;
                break;
            default:
                throw std::runtime_error( "未知仿真模式" );
            }

            // 告诉模拟器我们完成了一个迭代
            shared_memory_.RobotIsDone();
        }
    }
    catch ( std::exception& e ) {
        strncpy( shared_memory_().robotToSim.errorMessage, e.what(), sizeof( shared_memory_().robotToSim.errorMessage ) - 1 );
        shared_memory_().robotToSim.errorMessage[ sizeof( shared_memory_().robotToSim.errorMessage ) - 1 ] = '\0';
        std::cout << "错误信息: " << e.what() << std::endl;
        throw e;
    }
}

/**
 * @brief 处理控制参数请求
 *
 */
void SimulationBridge::HandleControlParameters() {
    ControlParameterRequest&  request  = shared_memory_().simToRobot.controlParameterRequest;
    ControlParameterResponse& response = shared_memory_().robotToSim.controlParameterResponse;
    if ( request.requestNumber <= response.requestNumber ) {
        // 没有新的请求！
        printf( "[SimulationBridge] 警告: 模拟器已经运行了一个 ControlParameter 迭代，但是没有新的请求！\n" );
        return;
    }

    // 检查请求是否有效
    u64 num_requests = request.requestNumber - response.requestNumber;
    assert( num_requests == 1 );

    response.nParameters = robot_params_->collection_.map_.size();  // todo don't do this every single time?

    switch ( request.requestKind ) {
    case ControlParameterRequestKind::kSET_ROBOT_PARAM_BY_NAME: {
        std::string       name( request.name );
        ControlParameter& param = robot_params_->collection_.LookUp( name );

        // 检查参数类型是否匹配请求类型
        if ( param.kind_ != request.parameterKind ) {
            throw std::runtime_error( "参数类型不匹配: " + name + ", 机器人认为它是 " + ControlParameterValueKindToString( param.kind_ ) + "，但是收到了设置为 "
                                      + ControlParameterValueKindToString( request.parameterKind ) );
        }

        // 设置参数值
        param.Set( request.value, request.parameterKind );

        // 响应请求
        response.requestNumber = request.requestNumber;  // 确认设置操作已完成
        response.parameterKind = request.parameterKind;  // 返回请求的参数类型
        response.value         = request.value;          // 返回请求的参数值
        strcpy( response.name, name.c_str() );  // 返回请求的参数名称
        response.requestKind = request.requestKind;

        printf( "%s\n", response.ToString().c_str() );

    } break;

    case ControlParameterRequestKind::kSET_USER_PARAM_BY_NAME: {
        std::string name( request.name );
        if ( !user_params_ ) {
            printf( "[SimulationBridge] 警告: 尝试设置用户参数，但是机器人没有用户参数！\n" );
        }
        else {
            ControlParameter& param = user_params_->collection_.LookUp( name );

            // 检查参数类型是否匹配请求类型
            if ( param.kind_ != request.parameterKind ) {
                throw std::runtime_error( "参数类型不匹配: " + name + ", 机器人认为它是 " + ControlParameterValueKindToString( param.kind_ ) + "，但是收到了设置为 "
                                          + ControlParameterValueKindToString( request.parameterKind ) );
            }

            // 设置参数值
            param.Set( request.value, request.parameterKind );
        }

        // 响应请求
        response.requestNumber = request.requestNumber;  // 确认设置操作已完成
        response.parameterKind = request.parameterKind;  // 返回请求的参数类型
        response.value         = request.value;          // 返回请求的参数值
        strcpy( response.name, name.c_str() );  // 返回请求的参数名称
        response.requestKind = request.requestKind;

        printf( "%s\n", response.ToString().c_str() );

    } break;

    case ControlParameterRequestKind::kGET_ROBOT_PARAM_BY_NAME: {
        std::string       name( request.name );
        ControlParameter& param = robot_params_->collection_.LookUp( name );

        // 检查参数类型是否匹配请求类型
        if ( param.kind_ != request.parameterKind ) {
            throw std::runtime_error( "参数类型不匹配: " + name + ", 机器人认为它是 " + ControlParameterValueKindToString( param.kind_ ) + "，但是收到了设置为 "
                                      + ControlParameterValueKindToString( request.parameterKind ) );
        }

        // 响应请求
        response.value         = param.Get( request.parameterKind );
        response.requestNumber = request.requestNumber;  // 确认获取操作已完成
        response.parameterKind = request.parameterKind;  // 返回请求的参数类型
        strcpy( response.name, name.c_str() );                      // 返回请求的参数名称
        response.requestKind = request.requestKind;  // 返回请求的类型

        printf( "%s\n", response.ToString().c_str() );
    } break;
    default:
        throw std::runtime_error( "unhandled get/set" );
    }
}

/**
 * @brief Run the robot controller.
 *
 */
void SimulationBridge::RunRobotControl() {
    if ( first_controller_run_ ) {
        printf( "[Simulator Driver] First run of robot controller...\n" );
        if ( robot_params_->IsFullyInitialized() ) {
            printf( "\tAll %ld control parameters are initialized\n", robot_params_->collection_.map_.size() );
        }
        else {
            printf( "\tbut not all control parameters were initialized. Missing:\n%s\n", robot_params_->GenerateUnitializedList().c_str() );
            throw std::runtime_error( "not all parameters initialized when going into kRunContorller" );
        }

        // auto* userControlParameters = robot_runner_->robot_ctrl_->GetUserControlParameters();
        auto* userControlParameters = robot_runner_->GetUserControlParameters();
        if ( userControlParameters ) {
            if ( userControlParameters->IsFullyInitialized() ) {
                printf( "\tAll %ld user parameters are initialized\n", userControlParameters->collection_.map_.size() );
                sim_mode_ = SimulatorMode::kRunContorller;
            }
            else {
                printf( "\tbut not all control parameters were initialized. Missing:\n%s\n", userControlParameters->GenerateUnitializedList().c_str() );
                throw std::runtime_error( "not all parameters initialized when going into kRunContorller" );
            }
        }
        else {
            sim_mode_ = SimulatorMode::kRunContorller;
        }

        interface_lcm_thread_cmd_          = std::thread( &SimulationBridge::HandleInterfaceLCM_cmd, this );
        interface_lcm_thread_cyberdog_cmd_ = std::thread( &SimulationBridge::HandleInterfaceLCM_cyberdog_cmd, this );
        interface_lcm_thread_motion_list_  = std::thread( &SimulationBridge::HandleInterfaceLCM_motion_list, this );
        interface_lcm_thread_usergait_     = std::thread( &SimulationBridge::HandleInterfaceLCM_usergait_file, this );
        interface_motor_control_thread_    = std::thread( &SimulationBridge::HandleMotorCtrlLcmThread, this );

        robot_runner_->SetCommandInterface( &cmd_interface_ );
        robot_runner_->SetSpiData( &shared_memory_().simToRobot.spiData );
        robot_runner_->SetSpiCommand( &shared_memory_().robotToSim.spiCommand );
        robot_runner_->SetRobotType( robot_ );
        robot_runner_->SetRobotAppearanceType( RobotAppearanceType::CURVED );
        robot_runner_->SetVectorNavData( &shared_memory_().simToRobot.vectorNav );
        robot_runner_->SetCheaterState( &shared_memory_().simToRobot.cheater_state );
        robot_runner_->SetRobotControlParameters( robot_params_ );
        robot_runner_->SetVisualizationData( &shared_memory_().robotToSim.visualizationData );
        robot_runner_->SetCyberdog2Visualization( &shared_memory_().robotToSim.mainCyberdog2Visualization );

        robot_runner_->Init();
        first_controller_run_ = false;
    }
    cmd_interface_.ProcessGamepadCommand( shared_memory_().simToRobot.gamepadCommand );

    robot_runner_->Run();
    robot_runner_->LCMPublishByThread();
}

void SimulationBridge::CyberdogLcmCmdCallback( const lcm::ReceiveBuffer* buf, const std::string& channel, const motion_control_request_lcmt* msg ) {
    ( void )buf;
    ( void )channel;
    ( void )msg;
    cmd_interface_.ProcessCyberdogLcmCommand( msg );
}

void SimulationBridge::UserGaitFileCallback( const lcm::ReceiveBuffer* buf, const std::string& channel, const file_send_lcmt* msg ) {
    ( void )buf;
    ( void )channel;
    file_recv_lcmt receive_msg_;
    receive_msg_.result = cmd_interface_.ProcessUserGaitFile( msg );
    user_gait_file_responce_lcm_.publish( "user_gait_result", &receive_msg_ );
}

void SimulationBridge::LcmCmdCallback( const lcm::ReceiveBuffer* buf, const std::string& channel, const robot_control_cmd_lcmt* msg ) {
    ( void )buf;
    ( void )channel;
    ( void )msg;
    cmd_interface_.ProcessLcmCommand( msg );
}

void SimulationBridge::LcmMotionCallback( const lcm::ReceiveBuffer* buf, const std::string& channel, const trajectory_command_lcmt* msg ) {
    ( void )buf;
    ( void )channel;
    cmd_interface_.ProcessLcmMotionCommand( msg );
}

void SimulationBridge::LcmMotorCtrlCallback( const lcm::ReceiveBuffer* rbuf, const std::string& chan, const motor_ctrl_lcmt* msg ) {
    ( void )rbuf;
    ( void )chan;
    cmd_interface_.ProcessLcmMotorCtrlCommand( msg );
}

void SimulationBridge::HandleInterfaceLCM_cmd() {
    while ( !interface_lcm_quit_ ) {
        lcm_.handle();
    }
}

void SimulationBridge::HandleInterfaceLCM_cyberdog_cmd() {
    while ( !interface_lcm_quit_ ) {
        cyberdog_lcm_.handle();
    }
}

void SimulationBridge::HandleInterfaceLCM_motion_list() {
    while ( !interface_lcm_quit_ ) {
        motion_list_lcm_.handle();
    }
}

void SimulationBridge::HandleInterfaceLCM_usergait_file() {
    while ( !interface_lcm_quit_ ) {
        user_gait_file_lcm_.handle();
    }
}
void SimulationBridge::HandleMotorCtrlLcmThread() {
    while ( !interface_lcm_quit_ ) {
        motor_control_lcm_.handle();
    }
}