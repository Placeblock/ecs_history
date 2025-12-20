//
// Created by felix on 13.12.25.
//

#ifndef ECS_HISTORY_CHANGE_APPLIER_H
#define ECS_HISTORY_CHANGE_APPLIER_H

#include <entt/entity/registry.hpp>

#include "change.hpp"

namespace ecs_history {
    template<typename T>
    class component_change_applier_t final : public component_change_supplier_t<T> {
        entt::registry &reg;
        static_entities_t &static_entities;

    public:
        explicit component_change_applier_t(static_entities_t &static_entities, entt::registry &reg)
            : reg(reg), static_entities(static_entities) {
        }

        void apply(const construct_change_t<T> &c) override {
            const static_entity_t static_entity = c.static_entity;
            const entt::entity entt = static_entities.get_entity(static_entity);
            reg.emplace<T>(entt, c.value);
        }

        void apply(const update_change_t<T> &c) override {
            const static_entity_t static_entity = c.static_entity;
            const entt::entity entt = static_entities.get_entity(static_entity);
            reg.replace<T>(entt, c.new_value);
        }

        void apply(const destruct_change_t<T> &c) override {
            const static_entity_t static_entity = c.static_entity;
            const entt::entity entt = static_entities.get_entity(static_entity);
            reg.remove<T>(entt);
        }
    };

    class entity_change_supplier_t {
    public:
        virtual ~entity_change_supplier_t() = default;

        virtual void apply(const entity_create_change_t &c) = 0;

        virtual void apply(const entity_destroy_change_t &c) = 0;
    };

    class entity_change_applier_t final : public entity_change_supplier_t {
        entt::registry &reg;
        static_entities_t &static_entities;

    public:
        explicit entity_change_applier_t(static_entities_t &static_entities, entt::registry &reg)
            : reg(reg), static_entities(static_entities) {
        }

        void apply(const entity_create_change_t &c) override;

        void apply(const entity_destroy_change_t &c) override;
    };
}


#endif //ECS_HISTORY_CHANGE_APPLIER_H
