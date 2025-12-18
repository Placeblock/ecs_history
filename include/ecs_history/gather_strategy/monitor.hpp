//
// Created by felix on 17.12.25.
//

#ifndef ECS_NET_MONITOR_HPP
#define ECS_NET_MONITOR_HPP

#include <entt/entt.hpp>
#include "change_reactive_mixin.hpp"
#include "../static_entity.hpp"
#include "../commit.hpp"

namespace ecs_history {
    class base_component_monitor_t {
    protected:
        const entt::id_type id;

    public:
        explicit base_component_monitor_t(const entt::id_type id)
            : id(id) {
        }

        virtual std::unique_ptr<base_component_commit_t> commit() = 0;

        virtual ~base_component_monitor_t() = default;
    };

    template<typename T>
    class component_monitor_t final : public base_component_monitor_t {
        using construct_reactive_storage_t = entt::change_reactive_mixin_t<entt::storage<entt::construct_component_change_t<
            T> >, entt::basic_registry<> >;
        using update_reactive_storage_t = entt::change_reactive_mixin_t<entt::storage<entt::update_component_change_t<T> >,
            entt::basic_registry<> >;
        using destruct_reactive_storage_t = entt::change_reactive_mixin_t<entt::storage<entt::destruct_component_change_t<
            T> >, entt::basic_registry<> >;

    public:
        explicit component_monitor_t(static_entities_t &entities, entt::registry &registry,
                                     const entt::id_type id = entt::type_hash<T>::value())
            : base_component_monitor_t(id),
              entities(entities),
              storage(registry.storage<T>(id)),
              constructed_storage(id),
              updated_storage(id),
              destructed_storage(id) {
            constructed_storage.bind(registry);
            constructed_storage.on_construct();
            updated_storage.bind(registry);
            updated_storage.on_update();
            destructed_storage.bind(registry);
            destructed_storage.on_destroy();
        }

        std::unique_ptr<base_component_commit_t> commit() override {
            std::unique_ptr<component_commit_t<T> > commit = std::make_unique<component_commit_t<T> >(this->id);
            for (auto [entt, change]: constructed_storage.each()) {
                static_entity_t static_entity = this->entities.get_static_entity(entt);
                this->entities.increase_version(static_entity);
                commit->add_change(std::make_unique<construct_change_t<T> >(static_entity, change.value));
            }
            for (const auto [entt, change]: updated_storage.each()) {
                static_entity_t static_entity = this->entities.get_static_entity(entt);
                this->entities.increase_version(static_entity);
                commit->add_change(
                    std::make_unique<update_change_t<T> >(static_entity, change.old_value, change.new_value));
            }
            for (const auto [entt, change]: destructed_storage.each()) {
                static_entity_t static_entity = this->entities.get_static_entity(entt);
                this->entities.increase_version(static_entity);
                commit->add_change(std::make_unique<destruct_change_t<T> >(static_entity, change.old_value));
            }
            return std::move(commit);
        }

        void clear() {
            constructed_storage.clear();
            updated_storage.clear();
            destructed_storage.clear();
        }

    private:
        static_entities_t &entities;
        const entt::storage<T> &storage;
        construct_reactive_storage_t constructed_storage;
        update_reactive_storage_t updated_storage;
        destruct_reactive_storage_t destructed_storage;
    };
}

#endif //ECS_NET_MONITOR_HPP
