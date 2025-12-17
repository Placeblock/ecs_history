//
// Created by felix on 13.12.25.
//

#ifndef ECS_HISTORY_STATIC_ENTITY_HPP
#define ECS_HISTORY_STATIC_ENTITY_HPP
#include <cstdint>
#include <map>
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
    std::map<static_entity_t, entt::entity> entities;
    static_entity_t next = 0;

public:
    [[nodiscard]] uint64_t create(entt::entity entt);

    void create(entt::entity entt, static_entity_t static_entity);

    void remove(entt::entity entt);

    [[nodiscard]] static_entity_t get_static_entity(entt::entity entt) const;

    [[nodiscard]] entt::entity get_entity(static_entity_t static_entity) const;
};

#endif //ECS_HISTORY_STATIC_ENTITY_HPP
