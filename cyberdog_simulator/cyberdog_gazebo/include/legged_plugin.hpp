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

#ifndef LEGGED_PLUGIN_HPP
#define LEGGED_PLUGIN_HPP

#include <functional>
#include <memory>
#include <string>
#include <vector>
#include <map>

// Gazebo Sim (gz-sim) headers - migrated from Gazebo Classic
#include <gz/sim/System.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/Joint.hh>
#include <gz/sim/Link.hh>
#include <gz/sim/Sensor.hh>
#include <gz/sim/World.hh>
#include <gz/sim/components.hh>
#include <gz/math.hh>
#include <sdf/sdf.hh>
#include <lcm/lcm-cpp.hpp>

#include "legged_simparam.hpp"
#include "actuator.hpp"
#include "lcmhandler.hpp"

#include <cyberdog_msg/msg/apply_force.hpp>

// Use gz::sim namespace to avoid namespace conflicts in gazebo namespace
using namespace gz::sim;
using namespace gz::math;

namespace gazebo
{

  struct _contact_force //Holds contact force data from foot contact sensors 
  {
    Eigen::Vector3d force; 
    std::string parent_name;
  };

  struct _apply_force //Holds apply force command from apply_force message
  {
    std::string name; 
    double time; 
    gz::math::Vector3d force; 
    gz::math::Vector3d rel_pos;
  };

  class LeggedPlugin 
    : public System
    , public ISystemConfigure
    , public ISystemPreUpdate
  {
  public:
    /// \brief Configure the plugin
    /// \param[in] _entity The entity the plugin is attached to
    /// \param[in] _sdf The SDF element for the plugin
    /// \param[in] _ecm The EntityComponentManager
    /// \param[in] _eventMgr The EventManager
    void Configure(const Entity &_entity,
                   const std::shared_ptr<const sdf::Element> &_sdf,
                   EntityComponentManager &_ecm,
                   EventManager &_eventMgr) override;

    /// \brief PreUpdate callback
    /// \param[in] _info Update info
    /// \param[in] _ecm The EntityComponentManager
    void PreUpdate(const UpdateInfo &_info,
                   EntityComponentManager &_ecm) override;

    Eigen::Vector3d forceToBody(_contact_force &_contact_force);

  private:
    /// \brief Update joint states from gazebo
    void GetJointStates(EntityComponentManager &_ecm);

    /// \brief Send sharedmemory data to control program
    void SendSMData(EntityComponentManager &_ecm);
    
    /// \brief Set the joint command of robot in gazebo
    void SetJointCom(EntityComponentManager &_ecm);

    /// \brief Handle gamepad command lcm messages 
    void HandleCommand(const lcm::ReceiveBuffer *rbuf, 
                       const std::string &chan, 
                       const gamepad_lcmt *msg);

    /// \brief Get the Contact Force from foot contact sensor
    void GetContactForce4(EntityComponentManager &_ecm);

    /// \brief Handle ApplyForce topic message 
    void ForceHandler(const cyberdog_msg::msg::ApplyForce::SharedPtr msg);

    /// \brief Apply force to the links of robot
    void ApplyForce(gz::sim::EntityComponentManager &_ecm);

    /// \brief Initialize joints from model
    void InitializeJoints(gz::sim::EntityComponentManager &_ecm);

    /// \brief Initialize sensors from model
    void InitializeSensors(gz::sim::EntityComponentManager &_ecm);

    // Model entity
    gz::sim::Entity modelEntity_;
    
    // Model object
    gz::sim::Model model_;

    // Joint entities
    std::vector<gz::sim::Joint> joints_;
    
    // Joint names vector
    std::vector<std::string> joint_names_;

    // Joint entity map
    std::map<std::string, gz::sim::Joint> joint_map_;

    // Link entities for force application
    std::map<std::string, gz::sim::Link> link_map_;

    // IMU sensor entity
    gz::sim::Entity imuSensorEntity_;

    // Contact sensor entities
    gz::sim::Entity contactSensorFL_;
    gz::sim::Entity contactSensorFR_;
    gz::sim::Entity contactSensorRL_;
    gz::sim::Entity contactSensorRR_;

    // ApplyForce topic subscription
    std::shared_ptr<GazeboNode> force_node_;
    rclcpp::Subscription<cyberdog_msg::msg::ApplyForce>::SharedPtr for_sub_;    
    _apply_force apply_force_;

    // Shared memory message
    SimulatorToRobotMessage simToRobot;

    // Lcm message of simulator state 
    simulator_lcmt lcm_sim_handler_;

    SimParam*     simparam_     = nullptr;
    LCMHandler*   lcmhandler_   = nullptr;
    NodeExc*      node_executor_ = nullptr;
    
    std::vector<double> q_;
    std::vector<double> dq_;
    std::vector<double> tau_;
    std::vector<double> contact_;
    std::vector<double> q_ctrl_;
    std::vector<double> dq_ctrl_;
    std::vector<double> tau_ctrl_;
    Actuator motor_;

    int foot_counter_;
    int frequency_counter_;

    Eigen::Quaterniond q_body_;
    uint kleg_map[4] = {1, 0, 3, 2};
    
    bool use_currentloop_response_;
    bool use_TNcurve_motormodel_;
    bool use_force_contact_sensor_;
    bool initialized_ = false;
  };
}

#endif // LEGGED_PLUGIN_HPP