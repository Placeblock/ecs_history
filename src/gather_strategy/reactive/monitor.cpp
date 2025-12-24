//
// Created by felix on 12/24/25.
//

#include "ecs_history/gather_strategy/reactive/monitor.hpp"

using namespace ecs_history;

base_component_monitor_t::base_component_monitor_t(const entt::id_type id)
    : id(id) {
}