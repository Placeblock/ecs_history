//
// Created by felix on 17.12.25.
//

#ifndef ECS_NET_REGISTRY_HPP
#define ECS_NET_REGISTRY_HPP

#include "monitor.hpp"

using namespace entt::literals;

namespace ecs_history {
    typedef std::vector<std::shared_ptr<base_component_monitor_t> > component_monitor_list_t;
    using reactive_entity_storage = entt::storage_type_t<entt::reactive, entt::entity>;

    template<typename T>
    void record_changes(entt::registry &reg,
                        const entt::id_type id = entt::type_hash<T>::value()) {
        if (!reg.ctx().contains<static_entities_t>()) {
            throw std::runtime_error(
                "Cannot record changes for component. Static entities not found in registry context.");
        }

        if constexpr (std::is_same_v<T, entt::entity>) {
            reactive_entity_storage &created_storage = reg.storage<entt::reactive>("entity_created_storage"_hs);
            reactive_entity_storage &destroyed_storage = reg.storage<entt::reactive>("entity_destroyed_storage"_hs);
            created_storage.on_construct<entt::entity>();
            destroyed_storage.on_destroy<entt::entity>();
        } else {
            auto &entities = reg.ctx().get<static_entities_t>();
            std::shared_ptr<component_monitor_t<T> > monitor = std::make_shared<component_monitor_t<T> >(entities, reg, id);
            if (reg.ctx().contains<component_monitor_list_t>()) {
                auto &list = reg.ctx().get<component_monitor_list_t>();
                list.emplace_back(monitor);
            } else {
                auto &list = reg.ctx().emplace<component_monitor_list_t>();
                list.emplace_back(monitor);
            }
        }
    }
}

#endif //ECS_NET_REGISTRY_HPP
