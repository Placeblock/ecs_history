//
// Created by felix on 17.12.25.
//

#include "ecs_history/static_entity.hpp"

#include <entt/entity/registry.hpp>
#include <spdlog/spdlog.h>

using namespace ecs_history;

static_entity_t static_entities_t::increase_ref(const entt::entity entity) {
    const static_entity_t static_entity = this->static_entities.get(entity).id;
    this->entities.get(static_entity).ref_count++;
    spdlog::debug("increasing reference count of entity");
    return static_entity;
}

entt::entity static_entities_t::create_entity_or_inc_ref(const static_entity_t static_entity) {
    if (this->entities.contains(static_entity)) {
        auto &[entt, ref_count] = this->entities.get(static_entity);
        ref_count++;
        spdlog::debug("increasing reference count of entity");
        return entt;
    }
    const auto entity = this->entity_storage.generate();
    this->static_entities.emplace(entity, static_entity);
    this->entities.emplace(static_entity, entity, static_cast<uint16_t>(1));
    spdlog::debug("creating new entity and increased reference count");
    return entity;
}

static_entity_t static_entities_t::create() {
    static_entity_t static_entity = this->next++;
    entt::entity entity = this->entity_storage.generate();
    this->static_entities.emplace(entity, static_entity);
    this->entities.emplace(static_entity, entity, static_cast<uint16_t>(0));
    spdlog::debug("creating new entity");
    return static_entity;
}

entt::entity static_entities_t::decrease_ref(const static_entity_t static_entity) {
    auto &[entt, ref_count] = this->entities.get(static_entity);
    ref_count--;
    if (ref_count == 0) {
        this->entity_storage.erase(entt);
        this->entities.erase(static_entity);
        this->static_entities.erase(entt);
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
    return this->entities.get(static_entity).entt;
}
