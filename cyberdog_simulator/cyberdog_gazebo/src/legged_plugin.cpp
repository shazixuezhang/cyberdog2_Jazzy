// Copyright (c) 2023-2023 Beijing Xiaomi Mobile Software Co., Ltd. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include "legged_plugin.hpp"
#include <gz/plugin/Register.hh>

// Use gz::sim namespace to avoid namespace conflicts
using namespace gz::sim;

namespace gazebo
{

  // Register this plugin with the simulator
  GZ_ADD_PLUGIN(LeggedPlugin, System, 
                ISystemConfigure, 
                ISystemPreUpdate)

  void LeggedPlugin::Configure(const Entity &_entity,
                               const std::shared_ptr<const sdf::Element> &/*_sdf*/,
                               EntityComponentManager &_ecm,
                               EventManager &/*_eventMgr*/)
  {
    std::cout << "**************Enter plugin (gz-sim)**************" << std::endl;
    
    // Store the model entity
    modelEntity_ = _entity;
    model_ = Model(_entity);
    
    auto modelName = model_.Name(_ecm);
    std::cout << modelName << " is imported" << std::endl;

    // Initialize ROS2 if not already initialized (required when running inside gz-sim)
    if (!rclcpp::ok()) {
      rclcpp::init(0, nullptr);
    }

    // Initialize node executor to receive topic messages 
    node_executor_ = new NodeExc();

    force_node_ = std::make_shared<GazeboNode>("force_node");
    for_sub_ = force_node_->create_subscription<cyberdog_msg::msg::ApplyForce>(
        "apply_force", 10, 
        std::bind(&LeggedPlugin::ForceHandler, this, std::placeholders::_1));
    node_executor_->AddNode(force_node_);

    // Initialize the sender and receiver of simulator parameters
    simparam_ = new SimParam(model_.Name(_ecm), node_executor_);

    // Initialize joints
    InitializeJoints(_ecm);

    // Initialize sensors
    InitializeSensors(_ecm);

    // Enable currentloop response limit of the motors
    use_currentloop_response_ = true;
    // Enable TN curve limit of the motors
    use_TNcurve_motormodel_ = true;
    // Disable force contact sensors of the robot
    use_force_contact_sensor_ = true;

    simparam_->FirstRun();

    // Initialize LCMHandler
    lcmhandler_ = new LCMHandler();

    // Matching gazebo update frequency with control program frequency
    frequency_counter_ = 0; 

    // Count leg for transferring contact forces to body coordinate
    foot_counter_ = 0;

    initialized_ = true;
    std::cout << "**************Plugin configured**************" << std::endl;
  }

  void LeggedPlugin::InitializeJoints(EntityComponentManager &_ecm)
  {
    // Get all joints from the model
    auto jointEntities = model_.Joints(_ecm);
    
    for (const auto &jointEntity : jointEntities) {
      Joint joint(jointEntity);
      auto jointNameOpt = joint.Name(_ecm);
      
      if (jointNameOpt) {
        std::string jointName = *jointNameOpt;
        joint_names_.push_back(jointName);
        joint_map_[jointName] = joint;
        joints_.push_back(joint);
        std::cout << "Joint found: " << jointName << std::endl;
      }
    }

    // Resize state vectors
    q_.resize(joints_.size());
    dq_.resize(joints_.size());
    tau_.resize(joints_.size());
    q_ctrl_.resize(joints_.size());
    dq_ctrl_.resize(joints_.size());
    tau_ctrl_.resize(joints_.size());

    // Initialize joint states
    for (size_t i = 0; i < joints_.size(); i++) {
      auto &joint = joints_[i];
      // Enable position, velocity, and force components
      joint.EnablePositionCheck(_ecm, true);
      joint.EnableVelocityCheck(_ecm, true);
      joint.EnableTransmittedWrenchCheck(_ecm, true);
    }

    // Initialize control values
    for (uint i = 0; i < 4 && i * 3 + 2 < joints_.size(); i++) {
      q_ctrl_[3*i] = 0;
      q_ctrl_[3*i+1] = 0;
      q_ctrl_[3*i+2] = 0;
      dq_ctrl_[3*i] = 0;
      dq_ctrl_[3*i+1] = 0;
      dq_ctrl_[3*i+2] = 0;
      tau_ctrl_[3*i] = 0;
      tau_ctrl_[3*i+1] = 0;
      tau_ctrl_[3*i+2] = 0;
    }

    contact_ = std::vector<double>(4, 1.0);

    // Initialize links for force application
    auto linkEntities = model_.Links(_ecm);
    for (const auto &linkEntity : linkEntities) {
      Link link(linkEntity);
      auto linkNameOpt = link.Name(_ecm);
      if (linkNameOpt) {
        std::string linkName = *linkNameOpt;
        link_map_[linkName] = link;
        link.EnableVelocityChecks(_ecm, true);
        std::cout << "Link found: " << linkName << std::endl;
      }
    }
  }

  void LeggedPlugin::InitializeSensors(EntityComponentManager &_ecm)
  {
    // Find IMU sensors in the model
    auto imuEntities = _ecm.EntitiesByComponents(
        components::Imu());
    
    for (const auto &imuEntity : imuEntities) {
      auto parentEntity = _ecm.Component<components::ParentEntity>(imuEntity);
      if (parentEntity && parentEntity->Data() == modelEntity_) {
        auto sensorName = _ecm.Component<components::Name>(imuEntity);
        if (sensorName) {
          imuSensorEntity_ = imuEntity;
          std::cout << "IMU sensor found: " << sensorName->Data() << std::endl;
        }
      }
    }
    
    // Find contact sensors in the model
    auto contactEntities = _ecm.EntitiesByComponents(
        components::ContactSensor());
    
    for (const auto &contactEntity : contactEntities) {
      auto parentEntity = _ecm.Component<components::ParentEntity>(contactEntity);
      if (parentEntity && parentEntity->Data() == modelEntity_) {
        auto sensorName = _ecm.Component<components::Name>(contactEntity);
        if (sensorName) {
          std::string name = sensorName->Data();
          if (name == "FL_foot_contact") {
            contactSensorFL_ = contactEntity;
            std::cout << "Contact sensor FL found" << std::endl;
          }
          else if (name == "FR_foot_contact") {
            contactSensorFR_ = contactEntity;
            std::cout << "Contact sensor FR found" << std::endl;
          }
          else if (name == "RL_foot_contact") {
            contactSensorRL_ = contactEntity;
            std::cout << "Contact sensor RL found" << std::endl;
          }
          else if (name == "RR_foot_contact") {
            contactSensorRR_ = contactEntity;
            std::cout << "Contact sensor RR found" << std::endl;
          }
        }
      }
    }
  }

  void LeggedPlugin::PreUpdate(const UpdateInfo &/*_info*/,
                               EntityComponentManager &_ecm)
  {
    if (!initialized_) return;

    // Matching gazebo update frequency with control program frequency
    frequency_counter_++;

    GetJointStates(_ecm);

    if (frequency_counter_ < 2) {
      SetJointCom(_ecm);
      return;
    }
    
    // Send data of robot state by sharedmemory to control program 
    SendSMData(_ecm);

    // Received and set joint command of robot from control program 
    SetJointCom(_ecm);

    // Get contact force from foot contact sensor
    if (use_force_contact_sensor_) {
      GetContactForce4(_ecm);
    }
    
    // Send simulator states by lcm
    lcmhandler_->SendSimData(lcm_sim_handler_);

    // Receive ros topic
    node_executor_->ReceiveTopic();

    // Apply force to the links of robot if command is received 
    ApplyForce(_ecm);

    frequency_counter_ = 0;
  }

  void LeggedPlugin::GetJointStates(EntityComponentManager &_ecm)
  {
    for (size_t i = 0; i < joints_.size(); i++) {
      auto &joint = joints_[i];
      
      // Get joint position
      auto pos = joint.Position(_ecm);
      q_[i] = (pos && pos->size() > 0) ? (*pos)[0] : 0.0;
      
      // Get joint velocity
      auto vel = joint.Velocity(_ecm);
      dq_[i] = (vel && vel->size() > 0) ? (*vel)[0] : 0.0;
      
      // Get joint force (from transmitted wrench)
      auto wrench = joint.TransmittedWrench(_ecm);
      tau_[i] = (wrench && wrench->size() > 0) ? (*wrench)[0].force().z() : 0.0;
    }

    // Transform to control coordinate
    for (uint i = 0; i < 4 && i * 3 + 2 < joints_.size(); i++) {
      q_ctrl_[3*i] = q_[3*i];
      q_ctrl_[3*i+1] = -q_[3*i+1];
      q_ctrl_[3*i+2] = -q_[3*i+2];
      dq_ctrl_[3*i] = dq_[3*i];
      dq_ctrl_[3*i+1] = -dq_[3*i+1];
      dq_ctrl_[3*i+2] = -dq_[3*i+2];
      tau_ctrl_[3*i] = tau_[3*i];
      tau_ctrl_[3*i+1] = -tau_[3*i+1];
      tau_ctrl_[3*i+2] = -tau_[3*i+2];
    }
  }

  void LeggedPlugin::SendSMData(EntityComponentManager &_ecm)
  {
    // Read IMU data from base_link (simplified approach for gz-sim)
    // In gz-sim, we get orientation and angular velocity from the base link
    if (link_map_.find("base_link") != link_map_.end()) {
      auto &baseLink = link_map_["base_link"];
      
      // Get IMU orientation from base_link pose
      auto worldPose = baseLink.WorldPose(_ecm);
      if (worldPose) {
        Pose3d pose = *worldPose;
        Quaterniond quat = pose.Rot();
        
        simToRobot.vectorNav.quat[3] = quat.W();
        simToRobot.vectorNav.quat[0] = quat.X();
        simToRobot.vectorNav.quat[1] = quat.Y();
        simToRobot.vectorNav.quat[2] = quat.Z();
        simToRobot.vectorNav.quat.normalize();
        
        lcm_sim_handler_.quat[0] = quat.W();
        lcm_sim_handler_.quat[1] = quat.X();
        lcm_sim_handler_.quat[2] = quat.Y();
        lcm_sim_handler_.quat[3] = quat.Z();
        
        q_body_.w() = quat.W();
        q_body_.x() = quat.X();
        q_body_.y() = quat.Y();
        q_body_.z() = quat.Z();
        q_body_.normalize();
        
        Eigen::Vector3d eulerAngle = q_body_.matrix().eulerAngles(2, 1, 0);
        lcm_sim_handler_.rpy[0] = eulerAngle[2];
        lcm_sim_handler_.rpy[1] = eulerAngle[1];
        lcm_sim_handler_.rpy[2] = eulerAngle[0];
      }
      
      // Get angular velocity from base_link
      auto worldAngularVel = baseLink.WorldAngularVelocity(_ecm);
      if (worldAngularVel) {
        Vector3d omega = *worldAngularVel;
        simToRobot.vectorNav.gyro.x() = omega.X();
        simToRobot.vectorNav.gyro.y() = omega.Y();
        simToRobot.vectorNav.gyro.z() = omega.Z();
        lcm_sim_handler_.omega[0] = omega.X();
        lcm_sim_handler_.omega[1] = omega.Y();
        lcm_sim_handler_.omega[2] = omega.Z();
      }
      
      // Get linear acceleration from base_link component
      auto linearAccelComp = _ecm.Component<components::LinearAcceleration>(
          baseLink.Entity());
      if (linearAccelComp) {
        Vector3d accel = linearAccelComp->Data();
        simToRobot.vectorNav.accelerometer.x() = accel.X();
        simToRobot.vectorNav.accelerometer.y() = accel.Y();
        simToRobot.vectorNav.accelerometer.z() = accel.Z();
      }
    }

    // Joint data to controller
    for (uint i = 0; i < 4 && i * 3 + 2 < joints_.size(); i++) {
      simToRobot.spiData.q_abad[kleg_map[i]] = q_ctrl_[3 * i + 0];
      simToRobot.spiData.q_hip[kleg_map[i]] = q_ctrl_[3 * i + 1];
      simToRobot.spiData.q_knee[kleg_map[i]] = q_ctrl_[3 * i + 2];
      simToRobot.spiData.qd_abad[kleg_map[i]] = dq_ctrl_[3 * i + 0];
      simToRobot.spiData.qd_hip[kleg_map[i]] = dq_ctrl_[3 * i + 1];
      simToRobot.spiData.qd_knee[kleg_map[i]] = dq_ctrl_[3 * i + 2];
      simToRobot.spiData.tau_abad[kleg_map[i]] = tau_ctrl_[3 * i + 0];
      simToRobot.spiData.tau_hip[kleg_map[i]] = tau_ctrl_[3 * i + 1];
      simToRobot.spiData.tau_knee[kleg_map[i]] = tau_ctrl_[3 * i + 2];
      
      for (uint j = 0; j < 3; j++) {
        lcm_sim_handler_.q[kleg_map[i]*3+j] = q_ctrl_[i*3+j];
        lcm_sim_handler_.qd[kleg_map[i]*3+j] = dq_ctrl_[i*3+j];
        lcm_sim_handler_.tau[kleg_map[i]*3+j] = tau_ctrl_[i*3+j];
      }
    }

    // Read body states from base_link
    if (link_map_.find("base_link") != link_map_.end()) {
      auto &baseLink = link_map_["base_link"];
      auto worldPose = baseLink.WorldPose(_ecm);
      auto worldLinearVel = baseLink.WorldLinearVelocity(_ecm);
      auto worldAngularVel = baseLink.WorldAngularVelocity(_ecm);
      
      if (worldPose) {
        Pose3d pose = *worldPose;
        simToRobot.cheaterState.position.x() = pose.Pos().X();
        simToRobot.cheaterState.position.y() = pose.Pos().Y();
        simToRobot.cheaterState.position.z() = pose.Pos().Z();
        simToRobot.cheaterState.orientation[0] = pose.Rot().W();
        simToRobot.cheaterState.orientation[1] = pose.Rot().X();
        simToRobot.cheaterState.orientation[2] = pose.Rot().Y();
        simToRobot.cheaterState.orientation[3] = pose.Rot().Z();
        
        lcm_sim_handler_.p[0] = pose.Pos().X();
        lcm_sim_handler_.p[1] = pose.Pos().Y();
        lcm_sim_handler_.p[2] = pose.Pos().Z();
        
        Eigen::Quaterniond q(pose.Rot().W(), pose.Rot().X(), pose.Rot().Y(), pose.Rot().Z());
        q.normalize();
        
        if (worldLinearVel) {
          Vector3d vel = *worldLinearVel;
          simToRobot.cheaterState.vBody.x() = vel.X();
          simToRobot.cheaterState.vBody.y() = vel.Y();
          simToRobot.cheaterState.vBody.z() = vel.Z();
          lcm_sim_handler_.v[0] = vel.X();
          lcm_sim_handler_.v[1] = vel.Y();
          lcm_sim_handler_.v[2] = vel.Z();
          
          simToRobot.cheaterState.vBody = q.conjugate() * simToRobot.cheaterState.vBody;
          lcm_sim_handler_.vb[0] = simToRobot.cheaterState.vBody[0];
          lcm_sim_handler_.vb[1] = simToRobot.cheaterState.vBody[1];
          lcm_sim_handler_.vb[2] = simToRobot.cheaterState.vBody[2];
        }
        
        if (worldAngularVel) {
          Vector3d omega = *worldAngularVel;
          simToRobot.cheaterState.omegaBody.x() = omega.X();
          simToRobot.cheaterState.omegaBody.y() = omega.Y();
          simToRobot.cheaterState.omegaBody.z() = omega.Z();
          lcm_sim_handler_.omega[0] = omega.X();
          lcm_sim_handler_.omega[1] = omega.Y();
          lcm_sim_handler_.omega[2] = omega.Z();
          
          simToRobot.cheaterState.omegaBody = q.conjugate() * simToRobot.cheaterState.omegaBody;
          lcm_sim_handler_.omegab[0] = simToRobot.cheaterState.omegaBody[0];
          lcm_sim_handler_.omegab[1] = simToRobot.cheaterState.omegaBody[1];
          lcm_sim_handler_.omegab[2] = simToRobot.cheaterState.omegaBody[2];
        }
      }
    }
    
    // Read gamepad command if gamepad command is received by lcmhandler
    if (lcmhandler_->HasEvent()) {
      simToRobot.gamepadCommand = lcmhandler_->ReceiveGPC();
      simparam_->LcmHasEvent();
    }

    // Send data of robot state by sharedmemory to control program 
    simparam_->SendSMData(simToRobot);
  }

  void LeggedPlugin::SetJointCom(EntityComponentManager &_ecm)
  {
    // Receive joint command by sharedmemory from control program 
    SpiCommand cmd = simparam_->ReceiveSMData();

    // Calculate motor torque by joint command 
    for (int i = 0; i < 4 && static_cast<size_t>(kleg_map[i]*3+2) < joints_.size(); i++) {
      double abad_effort = cmd.kp_abad[i] * (cmd.q_des_abad[i] - q_ctrl_[kleg_map[i]*3]) 
                         + cmd.kd_abad[i] * (cmd.qd_des_abad[i] - dq_ctrl_[kleg_map[i]*3]) 
                         + cmd.tau_abad_ff[i];
      double hip_effort = -(cmd.kp_hip[i] * (cmd.q_des_hip[i] - q_ctrl_[kleg_map[i]*3+1]) 
                          + cmd.kd_hip[i] * (cmd.qd_des_hip[i] - dq_ctrl_[kleg_map[i]*3+1]) 
                          + cmd.tau_hip_ff[i]);
      double knee_effort = -(cmd.kp_knee[i] * (cmd.q_des_knee[i] - q_ctrl_[kleg_map[i]*3+2]) 
                           + cmd.kd_knee[i] * (cmd.qd_des_knee[i] - dq_ctrl_[kleg_map[i]*3+2]) 
                           + cmd.tau_knee_ff[i]);

      if (use_TNcurve_motormodel_) {
        abad_effort = motor_.GetTorque(abad_effort, dq_[kleg_map[i]*3]);
        hip_effort = motor_.GetTorque(hip_effort, dq_[kleg_map[i]*3+1]);
        knee_effort = motor_.GetTorque(knee_effort, dq_[kleg_map[i]*3+2]);
      }
        
      if (use_currentloop_response_) {
        abad_effort = motor_.CerrentLoopResponse(abad_effort, dq_[kleg_map[i]*3], kleg_map[i]*3);
        hip_effort = motor_.CerrentLoopResponse(hip_effort, dq_[kleg_map[i]*3+1], kleg_map[i]*3+1);
        knee_effort = motor_.CerrentLoopResponse(knee_effort, dq_[kleg_map[i]*3+2], kleg_map[i]*3+2);
      }

      // Set joint forces
      if (joint_names_.size() > static_cast<size_t>(kleg_map[i]*3)) {
        auto &abadJoint = joint_map_[joint_names_[kleg_map[i]*3]];
        abadJoint.SetForce(_ecm, {abad_effort});
      }
      if (joint_names_.size() > static_cast<size_t>(kleg_map[i]*3+1)) {
        auto &hipJoint = joint_map_[joint_names_[kleg_map[i]*3+1]];
        hipJoint.SetForce(_ecm, {hip_effort});
      }
      if (joint_names_.size() > static_cast<size_t>(kleg_map[i]*3+2)) {
        auto &kneeJoint = joint_map_[joint_names_[kleg_map[i]*3+2]];
        kneeJoint.SetForce(_ecm, {knee_effort});
      }
    }
  }

  void LeggedPlugin::GetContactForce4(EntityComponentManager &/*_ecm*/)
  {
    // In gz-sim, contact forces are obtained through contact components
    // This is a simplified implementation - actual contact handling requires
    // contact sensor components to be properly configured
    
    // For now, set default contact forces
    for (int i = 0; i < 12; i++) {
      lcm_sim_handler_.f_foot[i] = 0.0;
    }
    
    // TODO: Implement proper contact force reading from contact sensors
    // when contact sensor components are available
  }

  Eigen::Vector3d LeggedPlugin::forceToBody(_contact_force &_contact_force)
  {
    // Transfer contact forces to body coordinate
    Eigen::Vector3d force;
    
    if (foot_counter_ > 3) {
      foot_counter_ = 0;
    }
    
    if (_contact_force.parent_name.empty()) {
      force << 0, 0, 0;
    } else {
      // Find the parent link
      if (link_map_.find(_contact_force.parent_name) != link_map_.end()) {
        // Use body orientation to transform force
        Eigen::Matrix3d rotationMatrixW = q_body_.toRotationMatrix();
        force = rotationMatrixW.transpose() * _contact_force.force;
      } else {
        force << 0, 0, 0;
      }
    }

    foot_counter_++;
    return force;
  }

  void LeggedPlugin::ForceHandler(const cyberdog_msg::msg::ApplyForce::SharedPtr msg)
  {
    // Handle ApplyForce topic message 
    apply_force_.name = msg->link_name;
    apply_force_.time = msg->time;
    for (int i = 0; i < 3; i++) {
      apply_force_.force[i] = msg->force[i];
      apply_force_.rel_pos[i] = msg->rel_pos[i];
    }
  }

  void LeggedPlugin::ApplyForce(EntityComponentManager &_ecm)
  {
    // Apply force to the links
    if (apply_force_.time > 0) {
      apply_force_.time = apply_force_.time - 0.001;
      
      if (link_map_.find(apply_force_.name) != link_map_.end()) {
        auto &link = link_map_[apply_force_.name];
        
        // Add force at relative position
        Vector3d force(apply_force_.force.X(), apply_force_.force.Y(), apply_force_.force.Z());
        Vector3d relPos(apply_force_.rel_pos.X(), apply_force_.rel_pos.Y(), apply_force_.rel_pos.Z());
        
        link.AddWorldForce(_ecm, force);
        // Note: AddWorldForceAtRelativePosition is not directly available in gz-sim
        // You may need to compute the world position and use AddWorldForce
      }
    }
  }

}