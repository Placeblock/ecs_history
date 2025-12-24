//
// Created by felix on 12/24/25.
//

#ifndef ECS_HISTORY_REACTIVE_GATHER_STRATEGY_HPP
#define ECS_HISTORY_REACTIVE_GATHER_STRATEGY_HPP
#include "monitor.hpp"
#include "ecs_history/gather_strategy/gather_strategy.hpp"

namespace ecs_history {
class reactive_gather_strategy final : public gather_strategy {
    using reactive_entity_storage = entt::storage_type_t<entt::reactive, entt::entity>;
    using component_monitor_list_t = std::vector<std::shared_ptr<base_component_monitor_t> >;

    static constexpr entt::id_type created_entities_storage_id = entt::hashed_string{
        "created_entities_storage"};
    static constexpr entt::id_type destroyed_entities_storage_id = entt::hashed_string{
        "destroyed_entities_storage"};

    entt::registry &reg;
    const static_entities_t &static_entities;

public:
    explicit reactive_gather_strategy(entt::registry &reg);

    std::vector<std::unique_ptr<base_change_set_t> > get_change_sets() override;

    std::vector<static_entity_t> get_created_entities() override;

    std::vector<static_entity_t> get_destroyed_entities() override;

    template<typename T>
    void record_changes(const entt::id_type id = entt::type_hash<T>::value()) {
        if constexpr (std::is_same_v<T, entt::entity>) {
            auto &created_storage = this->reg.ctx().emplace_as<reactive_entity_storage>(
                created_entities_storage_id);
            auto &destroyed_storage = this->reg.ctx().emplace_as<reactive_entity_storage>(
                destroyed_entities_storage_id);
            created_storage.bind(this->reg);
            destroyed_storage.bind(this->reg);
            created_storage.on_construct<entt::entity>();
            destroyed_storage.on_destroy<entt::entity>();
        } else {
            auto &entities = this->reg.ctx().get<static_entities_t>();
            std::shared_ptr<component_monitor_t<T> > monitor = std::make_shared<component_monitor_t<
                T> >(entities, this->reg, id);
            if (this->reg.ctx().contains<component_monitor_list_t>()) {
                auto &list = this->reg.ctx().get<component_monitor_list_t>();
                list.emplace_back(monitor);
            } else {
                auto &list = this->reg.ctx().emplace<component_monitor_list_t>();
                list.emplace_back(monitor);
            }
        }
    }

    void disable() override;

    void enable() override;
};
}

#endif //ECS_HISTORY_REACTIVE_GATHER_STRATEGY_HPP