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

#ifndef FOOT_CONTACT_PLUGIN_HPP
#define FOOT_CONTACT_PLUGIN_HPP

#include <string>
#include <functional>
#include <memory>

// Gazebo Sim (gz-sim) headers - migrated from Gazebo Classic
#include <gz/sim/System.hh>
#include <gz/sim/Sensor.hh>
#include <gz/sim/Model.hh>
#include <gz/sim/components.hh>
#include <gz/math.hh>
#include <sdf/sdf.hh>
#include <lcm/lcm-cpp.hpp>

#include "gazebo_foot_contact.hpp"

// Use gz::sim namespace to avoid namespace conflicts in gazebo namespace
using namespace gz::sim;

namespace gazebo
{
  /// \brief A plugin that processes foot contact sensor data
  class FootContactPlugin 
    : public System
    , public ISystemConfigure
    , public ISystemPostUpdate
  {
  public:
    /// \brief Constructor
    FootContactPlugin();

    /// \brief Destructor
    virtual ~FootContactPlugin();

    /// \brief Configure the plugin
    /// \param[in] _entity The entity the plugin is attached to
    /// \param[in] _sdf The SDF element for the plugin
    /// \param[in] _ecm The EntityComponentManager
    /// \param[in] _eventMgr The EventManager
    void Configure(const Entity &_entity,
                   const std::shared_ptr<const sdf::Element> &_sdf,
                   EntityComponentManager &_ecm,
                   EventManager &_eventMgr) override;

    /// \brief PostUpdate callback - called after physics update
    /// \param[in] _info Update info
    /// \param[in] _ecm The EntityComponentManager
    void PostUpdate(const UpdateInfo &_info,
                    const EntityComponentManager &_ecm) override;

  private:
    /// \brief Sensor entity
    Entity sensorEntity_;

    /// \brief Sensor object
    Sensor sensor_;

    /// \brief Model entity (parent)
    Entity modelEntity_;

    /// \brief Flag indicating if plugin is initialized
    bool initialized_ = false;
  };
}

#endif // FOOT_CONTACT_PLUGIN_HPP