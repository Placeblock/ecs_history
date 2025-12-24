//
// Created by felix on 13.12.25.
//

#ifndef ECS_HISTORY_CHANGE_APPLIER_H
#define ECS_HISTORY_CHANGE_APPLIER_H

#include <entt/entity/registry.hpp>

#include "change.hpp"

using namespace entt::literals;

namespace ecs_history {
template<typename T>
class change_applier_t final : public change_supplier_t<T> {
    entt::registry &reg;
    static_entities_t &static_entities;

public:
    explicit change_applier_t(static_entities_t &static_entities, entt::registry &reg)
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

class any_change_applier_t final : public any_change_supplier_t {
    entt::registry &reg;
    static_entities_t &static_entities;

    static void apply(entt::registry &reg, entt::entity entt, const entt::meta_any &value);

public:
    explicit any_change_applier_t(entt::registry &reg, static_entities_t &static_entities);

    void apply_construct(static_entity_t static_entity, entt::meta_any &value) override;

    void apply_update(static_entity_t static_entity,
                      entt::meta_any &old_value,
                      entt::meta_any &new_value) override;

    void apply_destruct(static_entity_t static_entity, entt::meta_any &old_value) override;
};
}


#endif //ECS_HISTORY_CHANGE_APPLIER_H
