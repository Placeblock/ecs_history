//
// Created by felix on 12/20/25.
//

#ifndef ECS_HISTORY_ENTITY_VERSION_HPP
#define ECS_HISTORY_ENTITY_VERSION_HPP

#include <cstdint>
#include <unordered_map>

#include "ecs_history/static_entity.hpp"

#ifndef ENTITY_VERSION_TYPE
#define ENTITY_VERSION_TYPE uint16_t
#endif

namespace ecs_history {
using entity_version_t = ENTITY_VERSION_TYPE;

class entity_version_handler_t {
    std::unordered_map<static_entity_t, entity_version_t> versions;

public:
    [[nodiscard]] entity_version_t get_version(static_entity_t entity);

    void set_version(static_entity_t entity, entity_version_t version);

    entity_version_t increment_version(static_entity_t entity);

    void remove_entity(static_entity_t entity);

    void add_entity(static_entity_t entity, entity_version_t version);
};
}


#endif //ECS_HISTORY_ENTITY_VERSION_HPP