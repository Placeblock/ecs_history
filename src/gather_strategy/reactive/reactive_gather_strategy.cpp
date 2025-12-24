//
// Created by felix on 12/24/25.
//

#include "ecs_history/gather_strategy/reactive/reactive_gather_strategy.hpp"

#include "ecs_history/gather_strategy/reactive/monitor.hpp"

using namespace ecs_history;
using namespace entt::literals;

reactive_gather_strategy::reactive_gather_strategy(entt::registry &reg)
    : reg(reg),
      static_entities(this->reg.ctx().get<static_entities_t>()) {
}

std::vector<std::unique_ptr<base_change_set_t> > reactive_gather_strategy::get_change_sets() {
    std::vector<std::unique_ptr<base_change_set_t> > change_sets;
    const auto &monitors = this->reg.ctx().get<std::vector<std::shared_ptr<
        base_component_monitor_t> > >();
    for (auto &monitor : monitors) {
        change_sets.emplace_back(monitor->commit());
        monitor->clear();
    }
    return change_sets;
}

std::vector<static_entity_t> reactive_gather_strategy::get_created_entities() {
    std::vector<static_entity_t> created_entities;
    auto &recorded_entities = this->reg.ctx().get<reactive_entity_storage>(
        created_entities_storage_id);
    created_entities.reserve(recorded_entities.size());
    for (const auto &created_entity : recorded_entities) {
        static_entity_t static_entity = static_entities.get_static_entity(
            created_entity);
        created_entities.emplace_back(static_entity);
    }
    recorded_entities.clear();
    return created_entities;
}

std::vector<static_entity_t> reactive_gather_strategy::get_destroyed_entities() {
    std::vector<static_entity_t> destroyed_entities;
    auto &recorded_entities = this->reg.ctx().get<reactive_entity_storage>(
        destroyed_entities_storage_id);
    destroyed_entities.reserve(recorded_entities.size());
    for (const auto &created_entity : recorded_entities) {
        static_entity_t static_entity = static_entities.get_static_entity(
            created_entity);
        destroyed_entities.emplace_back(static_entity);
    }
    recorded_entities.clear();
    return destroyed_entities;
}

void reactive_gather_strategy::disable() {
    if (this->reg.ctx().contains<std::vector<std::shared_ptr<
        base_component_monitor_t> > >()) {
        const auto &monitors = reg.ctx().get<std::vector<std::shared_ptr<
            base_component_monitor_t> > >();
        for (auto &monitor : monitors) {
            monitor->disable();
        }
        }
    if (this->reg.ctx().contains<reactive_entity_storage>("created_entities_storage"_hs)) {
        auto &created_storage = reg.ctx().get<reactive_entity_storage>("created_entities_storage"_hs);
        created_storage.reset();
    }
    if (this->reg.ctx().contains<reactive_entity_storage>("destroyed_entities_storage"_hs)) {
        auto &destroyed_storage = reg.ctx().get<reactive_entity_storage>("destroyed_entities_storage"_hs);
        destroyed_storage.reset();
    }
}

void reactive_gather_strategy::enable() {
    if (reg.ctx().contains<std::vector<std::shared_ptr<base_component_monitor_t> > >()) {
        const auto &monitors = reg.ctx().get<std::vector<std::shared_ptr<
            base_component_monitor_t> > >();
        for (auto &monitor : monitors) {
            monitor->enable();
        }
    }
    if (reg.ctx().contains<reactive_entity_storage>("created_entities_storage"_hs)) {
        auto &created_storage = reg.ctx().get<reactive_entity_storage>("created_entities_storage"_hs);
        created_storage.on_construct<entt::entity>();
    }
    if (reg.ctx().contains<reactive_entity_storage>("destroyed_entities_storage"_hs)) {
        auto &destroyed_storage = reg.ctx().get<reactive_entity_storage>("destroyed_entities_storage"_hs);
        destroyed_storage.on_destroy<entt::entity>();
    }
}