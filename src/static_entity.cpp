//
// Created by felix on 17.12.25.
//

#include "ecs_history/static_entity.hpp"

using namespace ecs_history;

uint64_t static_entities_t::create(const entt::entity entt) {
    static_entity_t static_entity = this->next++;
    this->static_entities.emplace(entt, static_entity);
    this->entities[static_entity] = {entt};
    return static_entity;
}

void static_entities_t::create(const entt::entity entt, static_entity_t static_entity) {
    if (this->static_entities.contains(entt)) {
        throw std::runtime_error("static entity already exists");
    }
    this->static_entities.emplace(entt, static_entity);
    this->entities[static_entity] = {entt};
}

entt::entity static_entities_t::remove(const static_entity_t static_entity) {
    if (!this->entities.contains(static_entity)) {
        throw std::runtime_error("static entity does not exist exists");
    }
    const entt::entity entt = this->entities.at(static_entity).entt;
    this->entities.erase(static_entity);
    this->static_entities.erase(entt);
    return entt;
}

static_entity_t static_entities_t::get_static_entity(const entt::entity entt) const {
    if (!this->static_entities.contains(entt)) {
        throw std::runtime_error("entity does not exist");
    }
    return this->static_entities.get(entt).id;
}

entt::entity static_entities_t::get_entity(const static_entity_t static_entity) const {
    if (!this->entities.contains(static_entity)) {
        throw std::runtime_error("static entity does not exist");
    }
    return this->entities.at(static_entity).entt;
}
