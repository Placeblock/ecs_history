//
// Created by felix on 17.12.25.
//

#include "ecs_history/static_entity.hpp"

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

using namespace ecs_history;

entt::entity static_entities_t::create() {
    const auto entity = this->entity_storage.generate();
    static_entity_t static_entity = this->next++;
    this->static_entities.emplace(entity, static_entity);
    this->entities[static_entity] = {entity, 0};
    this->versions.emplace(static_entity, 0);
    spdlog::debug("created new entity");
    return entity;
}

void static_entities_t::create(static_entity_t static_entity, entity_version_t version) {
    const auto entity = this->entity_storage.generate();
    this->entities[static_entity] = {entity, 0};
    this->static_entities.emplace(entity, static_entity);
    this->versions.emplace(static_entity, version);
}

bool static_entities_t::has_entity(const static_entity_t static_entity) const {
    return this->entities.contains(static_entity);
}

static_entity_t static_entities_t::increase_ref(const entt::entity entity) {
    const static_entity_t static_entity = this->static_entities.get(entity).id;
    this->entities.at(static_entity).ref_count++;
    spdlog::debug("increasing reference count of entity");
    return static_entity;
}

entt::entity static_entities_t::increase_ref(const static_entity_t static_entity) {
    auto &[entt, ref_count] = this->entities.at(static_entity);
    ref_count++;
    spdlog::debug("increasing reference count of entity");
    return entt;
}

entt::entity static_entities_t::decrease_ref(const static_entity_t static_entity) {
    auto &[entt, ref_count] = this->entities.at(static_entity);
    ref_count--;
    if (ref_count == 0) {
        this->entity_storage.erase(entt);
        this->static_entities.erase(entt);
        this->versions.erase(static_entity);
        this->entities.erase(static_entity);
        spdlog::debug("destroying entity without components");
    }
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

entity_version_t static_entities_t::get_version(
    const static_entity_t entity) const {
    return this->versions.at(entity);
}

void static_entities_t::set_version(const static_entity_t entity,
                                    const entity_version_t version) {
    this->versions[entity] = version;
}

entity_version_t static_entities_t::increment_version(
    const static_entity_t entity) {
    if (!this->versions.contains(entity)) {
        throw std::runtime_error("entity does not exist in version handler");
    }
    return this->versions.at(entity)++;
}