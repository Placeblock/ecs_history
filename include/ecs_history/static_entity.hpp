//
// Created by felix on 13.12.25.
//

#ifndef ECS_HISTORY_STATIC_ENTITY_HPP
#define ECS_HISTORY_STATIC_ENTITY_HPP
#include <cstdint>
#include <random>
#include <entt/entity/storage.hpp>

#ifndef STATIC_ENTITY_TYPE
#define STATIC_ENTITY_TYPE uint64_t
#endif

namespace ecs_history {
using static_entity_t = STATIC_ENTITY_TYPE;

inline uint64_t random_entity_start() {
    static std::mt19937_64 rng{std::random_device{}()};
    return (rng() & 0xFFFFULL) << 48;
}

class static_entities_t {
    struct static_entity_container_t {
        static_entity_t id;
    };

    struct entity_container {
        entt::entity entt;
        uint16_t ref_count;
    };

    entt::storage<entt::entity> entity_generator;
    entt::basic_storage<entity_container, static_entity_t> entities;
    entt::storage<static_entity_container_t> static_entities;
    static_entity_t next;

public:
    explicit static_entities_t() {
        this->next = random_entity_start();
    }

    [[nodiscard]] static_entity_t increase_ref(entt::entity entity);

    entt::entity create_entity_or_inc_ref(static_entity_t static_entity);

    static_entity_t create();

    entt::entity decrease_ref(static_entity_t static_entity);

    [[nodiscard]] static_entity_t get_static_entity(entt::entity entt) const;

    [[nodiscard]] entt::entity get_entity(static_entity_t static_entity) const;
};
}

#endif //ECS_HISTORY_STATIC_ENTITY_HPP
