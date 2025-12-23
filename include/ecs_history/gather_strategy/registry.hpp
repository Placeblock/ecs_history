//
// Created by felix on 17.12.25.
//

#ifndef ECS_HISTORY_REGISTRY_HPP
#define ECS_HISTORY_REGISTRY_HPP

#include "monitor.hpp"

using namespace entt::literals;

namespace ecs_history {
    typedef std::vector<std::shared_ptr<base_component_monitor_t> > component_monitor_list_t;
    using reactive_entity_storage = entt::storage_type_t<entt::reactive, entt::entity>;

    class registry_t {
    protected:
        static_entities_t &static_entities;
    public:
        entt::registry &handle;

        explicit registry_t(entt::registry& registry)
            : static_entities(registry.ctx().emplace<static_entities_t>()), handle(registry) {
        }

        entt::entity create() const {
            const auto entity = this->handle.create();
            this->static_entities.create(entity);
            return entity;
        }

        template<typename T>
        void record_changes(const entt::id_type id = entt::type_hash<T>::value()) {
            if (!this->handle.ctx().contains<static_entities_t>()) {
                throw std::runtime_error(
                    "Cannot record changes for component. Static entities not found in registry context.");
            }

            if constexpr (std::is_same_v<T, entt::entity>) {
                reactive_entity_storage &created_storage = this->handle.storage<entt::reactive>("entity_created_storage"_hs);
                reactive_entity_storage &destroyed_storage = this->handle.storage<entt::reactive>("entity_destroyed_storage"_hs);
                created_storage.on_construct<entt::entity>();
                destroyed_storage.on_destroy<entt::entity>();
            } else {
                auto &entities = this->handle.ctx().get<static_entities_t>();
                std::shared_ptr<component_monitor_t<T> > monitor = std::make_shared<component_monitor_t<T> >(entities, this->handle, id);
                if (this->handle.ctx().contains<component_monitor_list_t>()) {
                    auto &list = this->handle.ctx().get<component_monitor_list_t>();
                    list.emplace_back(monitor);
                } else {
                    auto &list = this->handle.ctx().emplace<component_monitor_list_t>();
                    list.emplace_back(monitor);
                }
            }
        }
    };

}

#endif //ECS_HISTORY_REGISTRY_HPP
