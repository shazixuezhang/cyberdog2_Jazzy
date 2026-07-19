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

#include "foot_contact_plugin.hpp"
#include <gz/plugin/Register.hh>

// Use gz::sim namespace to avoid namespace conflicts
using namespace gz::sim;

namespace gazebo
{

  // Register this plugin with the simulator
  GZ_ADD_PLUGIN(FootContactPlugin, System,
                ISystemConfigure,
                ISystemPostUpdate)

  /////////////////////////////////////////////////
  FootContactPlugin::FootContactPlugin()
  {
  }

  /////////////////////////////////////////////////
  FootContactPlugin::~FootContactPlugin()
  {
  }

  /////////////////////////////////////////////////
  void FootContactPlugin::Configure(const Entity &_entity,
                                    const std::shared_ptr<const sdf::Element> &/*_sdf*/,
                                    EntityComponentManager &_ecm,
                                    EventManager &/*_eventMgr*/)
  {
    // Store the sensor entity
    sensorEntity_ = _entity;
    sensor_ = Sensor(_entity);

    // Get the sensor name for debugging
    auto sensorName = _ecm.Component<components::Name>(_entity);
    if (sensorName) {
      std::cout << "FootContactPlugin configured for sensor: " 
                << sensorName->Data() << std::endl;
    }

    // Get the parent entity (model)
    auto parentEntity = _ecm.Component<components::ParentEntity>(_entity);
    if (parentEntity) {
      modelEntity_ = parentEntity->Data();
    }

    initialized_ = true;
  }

  /////////////////////////////////////////////////
  void FootContactPlugin::PostUpdate(const UpdateInfo &/*_info*/,
                                     const EntityComponentManager &/*_ecm*/)
  {
    if (!initialized_) return;

    // In gz-sim, contact data is obtained through contact components
    // This is a placeholder for contact processing logic
    // The actual contact force reading would require:
    // 1. Contact sensor component to be properly configured in the SDF
    // 2. Reading from ContactData component
    
    // Note: gz-sim uses a different approach for contact sensors
    // Contact data is typically obtained through:
    // - components::ContactData
    // - components::Collision
    
    // For now, this plugin is ready to be extended with contact processing
  }

}