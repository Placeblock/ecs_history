//
// Created by felix on 13.12.25.
//

#ifndef ECS_HISTORY_STATIC_ENTITY_HPP
#define ECS_HISTORY_STATIC_ENTITY_HPP
#include <cstdint>
#include <entt/entity/storage.hpp>

#ifndef STATIC_ENTITY_TYPE
#define STATIC_ENTITY_TYPE uint64_t
#endif

using static_entity_t = STATIC_ENTITY_TYPE;

class static_entities_t {
    struct static_entity_container_t {
        static_entity_t id;
    };

    entt::storage<static_entity_container_t> static_entities;
    static_entity_t next = 0;

public:
    [[nodiscard]] uint64_t create(const entt::entity entt) {
        static_entity_t id = this->next++;
        this->static_entities.emplace(entt, id);
        return id;
    }

    [[nodiscard]] static_entity_t get_static_entity(const entt::entity entt) {
        if (!this->static_entities.contains(entt)) {
            std::printf("Created new static entity for entity %d\n", entt);
            return this->create(entt);
        }
        return this->static_entities.get(entt).id;
    }
};

#endif //ECS_HISTORY_STATIC_ENTITY_HPP