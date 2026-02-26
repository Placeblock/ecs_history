//
// Created by felix on 13.12.25.
//

#ifndef ECS_HISTORY_CHANGE_APPLIER_H
#define ECS_HISTORY_CHANGE_APPLIER_H

#include <entt/entity/registry.hpp>

#include "change.hpp"
#include <entt/meta/meta.hpp>

using namespace entt::literals;

namespace ecs_history {
class any_change_applier_t final : public any_change_supplier_t {
    entt::registry &reg;
    static_entities_t &static_entities;

public:
    static void apply(entt::registry &reg, entt::entity entt, const entt::meta_any &value);

    explicit any_change_applier_t(entt::registry &reg, static_entities_t &static_entities);

    void apply_construct(static_entity_t static_entity, entt::meta_any &value) override;

    void apply_update(static_entity_t static_entity,
                      entt::meta_any &old_value,
                      entt::meta_any &new_value) override;

    void apply_destruct(static_entity_t static_entity, entt::meta_any &old_value) override;
};
}


#endif //ECS_HISTORY_CHANGE_APPLIER_H
