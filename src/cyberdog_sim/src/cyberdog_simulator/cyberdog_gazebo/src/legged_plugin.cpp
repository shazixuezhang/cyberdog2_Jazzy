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
#include <gz/msgs/contacts.pb.h>

using namespace gz::sim;

namespace gazebo
{

  _contact_force GetContactForce(const ::gz::msgs::Contacts &contacts);

  GZ_ADD_PLUGIN(LeggedPlugin, System, 
                ISystemConfigure, 
                ISystemPreUpdate)

  _contact_force GetContactForce(const ::gz::msgs::Contacts &contacts)
  {
    Eigen::Vector3d force;
    std::string parent_name;
    unsigned int count_ = contacts.contact_size();
    for (unsigned int i = 0; i < count_; ++i) {

      if (contacts.contact(i).position_size() != 1) {
        std::cerr << "Contact count isn't correct!!!!" << std::endl;
      }

      for (int j = 0; j < contacts.contact(i).position_size(); ++j) {
        force[0] += contacts.contact(i).wrench(0).body_1_wrench().force().x();
        force[1] += contacts.contact(i).wrench(0).body_1_wrench().force().y();
        force[2] += contacts.contact(i).wrench(0).body_1_wrench().force().z();
      }
    }

    if (count_ != 0) {
      force[0] = force[0] / double(count_);
      force[1] = force[1] / double(count_);
      force[2] = force[2] / double(count_);

      parent_name = contacts.contact(0).wrench(0).body_1_name();
      int index = parent_name.find("::");
      parent_name = parent_name.substr(index+2, parent_name.length());
      index = parent_name.find("::");
      parent_name = parent_name.substr(0, index);
    } else {
      force[0] = 0;
      force[1] = 0;
      force[2] = 0;
      parent_name = "";
    }

    return {force, parent_name};
  }

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

    tf_broadcaster_ = std::make_shared<tf2_ros::TransformBroadcaster>(force_node_);

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

    gzNode_ = std::make_unique<::gz::transport::Node>();
    gzNode_->Subscribe("/sensor/imu", &LeggedPlugin::OnIMUMsg, this);
    std::cout << "Subscribed to /sensor/imu" << std::endl;

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

    if (link_map_.find("base_link") != link_map_.end()) {
      auto &baseLink = link_map_["base_link"];
      auto worldPose = baseLink.WorldPose(_ecm);
      if (worldPose) {
        geometry_msgs::msg::TransformStamped transform;
        transform.header.stamp = force_node_->get_clock()->now();
        transform.header.frame_id = "odom";
        transform.child_frame_id = "base_link";
        transform.transform.translation.x = worldPose->Pos().X();
        transform.transform.translation.y = worldPose->Pos().Y();
        transform.transform.translation.z = worldPose->Pos().Z();
        tf2::Quaternion q(
            worldPose->Rot().X(),
            worldPose->Rot().Y(),
            worldPose->Rot().Z(),
            worldPose->Rot().W());
        transform.transform.rotation = tf2::toMsg(q);
        tf_broadcaster_->sendTransform(transform);
      }
    }

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

  void LeggedPlugin::OnIMUMsg(const ::gz::msgs::IMU &_msg)
  {
    imuMsg_ = _msg;
  }

  void LeggedPlugin::SendSMData(EntityComponentManager &_ecm)
  {
    // Get IMU data from IMU sensor topic /sensor/imu
    simToRobot.vectorNav.quat[3] = imuMsg_.orientation().w();
    simToRobot.vectorNav.quat[0] = imuMsg_.orientation().x();
    simToRobot.vectorNav.quat[1] = imuMsg_.orientation().y();
    simToRobot.vectorNav.quat[2] = imuMsg_.orientation().z();
    simToRobot.vectorNav.quat.normalize();
    
    simToRobot.vectorNav.gyro.x() = imuMsg_.angular_velocity().x();
    simToRobot.vectorNav.gyro.y() = imuMsg_.angular_velocity().y();
    simToRobot.vectorNav.gyro.z() = imuMsg_.angular_velocity().z();
    
    simToRobot.vectorNav.accelerometer.x() = imuMsg_.linear_acceleration().x();
    simToRobot.vectorNav.accelerometer.y() = imuMsg_.linear_acceleration().y();
    simToRobot.vectorNav.accelerometer.z() = imuMsg_.linear_acceleration().z();

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

        q_body_.w() = pose.Rot().W();
        q_body_.x() = pose.Rot().X();
        q_body_.y() = pose.Rot().Y();
        q_body_.z() = pose.Rot().Z();
        q_body_.normalize();

        lcm_sim_handler_.quat[0] = pose.Rot().W();
        lcm_sim_handler_.quat[1] = pose.Rot().X();
        lcm_sim_handler_.quat[2] = pose.Rot().Y();
        lcm_sim_handler_.quat[3] = pose.Rot().Z();

        Eigen::Vector3d eulerAngle = q_body_.matrix().eulerAngles(2, 1, 0);
        lcm_sim_handler_.rpy[0] = eulerAngle[2];
        lcm_sim_handler_.rpy[1] = eulerAngle[1];
        lcm_sim_handler_.rpy[2] = eulerAngle[0];
        
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

  void LeggedPlugin::GetContactForce4(EntityComponentManager &_ecm)
  {
    _contact_force contact_force_fl_;
    _contact_force contact_force_fr_;
    _contact_force contact_force_hl_;
    _contact_force contact_force_hr_;

    auto contactDataFL = _ecm.Component<components::ContactSensorData>(contactSensorFL_);
    auto contactDataFR = _ecm.Component<components::ContactSensorData>(contactSensorFR_);
    auto contactDataRL = _ecm.Component<components::ContactSensorData>(contactSensorRL_);
    auto contactDataRR = _ecm.Component<components::ContactSensorData>(contactSensorRR_);

    if (contactDataFL) {
      contact_force_fl_ = GetContactForce(contactDataFL->Data());
    } else {
      contact_force_fl_ = {Eigen::Vector3d::Zero(), ""};
    }
    if (contactDataFR) {
      contact_force_fr_ = GetContactForce(contactDataFR->Data());
    } else {
      contact_force_fr_ = {Eigen::Vector3d::Zero(), ""};
    }
    if (contactDataRL) {
      contact_force_hl_ = GetContactForce(contactDataRL->Data());
    } else {
      contact_force_hl_ = {Eigen::Vector3d::Zero(), ""};
    }
    if (contactDataRR) {
      contact_force_hr_ = GetContactForce(contactDataRR->Data());
    } else {
      contact_force_hr_ = {Eigen::Vector3d::Zero(), ""};
    }

    for (unsigned int i = 0; i < 3; i++) {
      lcm_sim_handler_.f_foot[i] = LeggedPlugin::forceToBody(contact_force_fl_, _ecm)[i];
      lcm_sim_handler_.f_foot[i+3] = LeggedPlugin::forceToBody(contact_force_fr_, _ecm)[i];
      lcm_sim_handler_.f_foot[i+6] = LeggedPlugin::forceToBody(contact_force_hl_, _ecm)[i];
      lcm_sim_handler_.f_foot[i+9] = LeggedPlugin::forceToBody(contact_force_hr_, _ecm)[i];
    }
  }

  Eigen::Vector3d LeggedPlugin::forceToBody(_contact_force &_contact_force, EntityComponentManager &_ecm)
  {
    Eigen::Vector3d force;
    if (foot_counter_ > 3) {
      foot_counter_ = 0;
    }
    if (_contact_force.parent_name.empty()) {
      force << 0, 0, 0;
    } else {
      if (link_map_.find(_contact_force.parent_name) != link_map_.end()) {
        auto &parent_link = link_map_[_contact_force.parent_name];
        auto parent_pose = parent_link.WorldPose(_ecm);
        if (parent_pose) {
          Eigen::Quaterniond q(
              parent_pose->Rot().W(),
              parent_pose->Rot().X(),
              parent_pose->Rot().Y(),
              parent_pose->Rot().Z());
          q.normalize();

          Eigen::Matrix3d rotationMatrix = q.toRotationMatrix();
          Eigen::Matrix3d rotationMatrixW = q_body_.toRotationMatrix();

          force = rotationMatrixW.transpose() * rotationMatrix * _contact_force.force;
        } else {
          force << 0, 0, 0;
        }
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
    if (apply_force_.time > 0) {
      apply_force_.time = apply_force_.time - 0.001;
      
      if (link_map_.find(apply_force_.name) != link_map_.end()) {
        auto &link = link_map_[apply_force_.name];
        
        Vector3d force(apply_force_.force.X(), apply_force_.force.Y(), apply_force_.force.Z());
        Vector3d relPos(apply_force_.rel_pos.X(), apply_force_.rel_pos.Y(), apply_force_.rel_pos.Z());
        
        link.AddWorldForce(_ecm, force, relPos);
      }
    }
  }

}
