//
// Created by felix on 12/24/25.
//

#include "../include/ecs_history/entity_version.hpp"

using namespace ecs_history;

entity_version_t entity_version_handler_t::get_version(
    const static_entity_t entity) {
    return this->versions[entity];
}

void entity_version_handler_t::set_version(const static_entity_t entity,
                                           const entity_version_t version) {
    this->versions[entity] = version;
}

entity_version_t entity_version_handler_t::increment_version(
    const static_entity_t entity) {
    if (!this->versions.contains(entity)) {
        throw std::runtime_error("entity does not exist in version handler");
    }
    return this->versions.at(entity)++;
}

void entity_version_handler_t::remove_entity(const static_entity_t entity) {
    if (!this->versions.contains(entity)) {
        throw std::runtime_error("entity does not exist in version handler");
    }
    this->versions.erase(entity);
}

void entity_version_handler_t::
add_entity(const static_entity_t entity, const entity_version_t version) {
    this->versions[entity] = version;
}
