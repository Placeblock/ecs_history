//
// Created by felix on 13.12.25.
//

#include "ecs_history/change_applier.hpp"

using namespace ecs_history;

void entity_change_applier_t::apply(const entity_create_change_t &c) {
    const static_entity_t static_entity = c.entt;
    const entt::entity entt = reg.create();
    static_entities.create(entt, static_entity);
}

void entity_change_applier_t::apply(const entity_destroy_change_t &c) {
    const static_entity_t static_entity = c.entt;
    const entt::entity entt = static_entities.get_entity(static_entity);
    static_entities.remove(entt);
    reg.destroy(entt);
}
