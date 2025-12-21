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

    class any_component_change_applier_t final : public any_component_change_supplier_t {
        entt::registry &reg;
        static_entities_t &static_entities;

        static void apply(entt::registry &reg, const entt::entity entt, const entt::meta_any &value) {
            const auto emplaceFunc = value.type().func("emplace"_hs);
            if (!emplaceFunc) {
                const std::string type_name{value.base().type().name()};
                throw std::runtime_error("cannot find emplace function for deserialized change " + type_name);
            }
            emplaceFunc.invoke({}, entt::forward_as_meta(reg), entt, value.as_ref());
        }
    public:
        explicit any_component_change_applier_t(entt::registry &reg, static_entities_t &static_entities)
            : reg(reg), static_entities(static_entities) {
        }

        void apply_construct(const static_entity_t static_entity, entt::meta_any &value) override {
            const auto entt = this->static_entities.get_entity(static_entity);
            apply(reg, entt, value);
        }

        void apply_update(const static_entity_t static_entity, entt::meta_any &old_value, entt::meta_any &new_value) override {
            const auto entt = this->static_entities.get_entity(static_entity);
            apply(reg, entt, new_value);
        }

        void apply_destruct(const static_entity_t static_entity, entt::meta_any &old_value) override {
            const auto entt = this->static_entities.get_entity(static_entity);
            const auto storage = this->reg.storage(old_value.type().id());
            storage->remove(entt);
        }
    };
}


#endif //ECS_HISTORY_CHANGE_APPLIER_H
