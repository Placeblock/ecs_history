//
// Created by felix on 13.12.25.
//

#include "ecs_history/change_applier.hpp"

using namespace ecs_history;

void any_change_applier_t::apply(entt::registry &reg,
                                 const entt::entity entt,
                                 const entt::meta_any &value) {
    const auto emplaceFunc = value.type().func("emplace"_hs);
    if (!emplaceFunc) {
        const std::string type_name{value.base().info().name()};
        throw std::runtime_error(
            "cannot find emplace function for deserialized change " + type_name);
    }
    if (!emplaceFunc.invoke({},
                            entt::forward_as_meta(reg),
                            entt::forward_as_meta(entt),
                            value.as_ref())) {
        throw std::runtime_error("failed to apply component to entity from change");
    }
}

any_change_applier_t::
any_change_applier_t(entt::registry &reg, static_entities_t &static_entities)
    : reg(reg), static_entities(static_entities) {
}

void any_change_applier_t::apply_construct(const static_entity_t static_entity,
                                           entt::meta_any &value) {
    const auto entt = this->static_entities.get_entity(static_entity);
    apply(reg, entt, value);
}

void any_change_applier_t::apply_update(const static_entity_t static_entity,
                                        entt::meta_any &old_value,
                                        entt::meta_any &new_value) {
    const auto entt = this->static_entities.get_entity(static_entity);
    apply(reg, entt, new_value);
}

void any_change_applier_t::apply_destruct(const static_entity_t static_entity,
                                          entt::meta_any &old_value) {
    const auto entt = this->static_entities.get_entity(static_entity);
    const auto storage = this->reg.storage(old_value.type().id());
    storage->remove(entt);
}