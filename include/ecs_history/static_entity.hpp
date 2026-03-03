//
// Created by felix on 13.12.25.
//

#ifndef ECS_HISTORY_STATIC_ENTITY_HPP
#define ECS_HISTORY_STATIC_ENTITY_HPP
#include <cstdint>
#include <random>
#include <entt/entity/registry.hpp>
#include <entt/entity/storage.hpp>

#ifndef STATIC_ENTITY_TYPE
#define STATIC_ENTITY_TYPE uint64_t
#endif

#ifndef ENTITY_VERSION_TYPE
#define ENTITY_VERSION_TYPE uint16_t
#endif

namespace ecs_history {
using static_entity_t = STATIC_ENTITY_TYPE;
using entity_version_t = ENTITY_VERSION_TYPE;

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

    entt::storage<entt::entity> entity_storage;
    std::unordered_map<static_entity_t, entity_container> entities;
    entt::storage<static_entity_container_t> static_entities;
    std::unordered_map<static_entity_t, entity_version_t> versions;
    static_entity_t next;

public:
    explicit static_entities_t() : next(random_entity_start()) {
    }

    entt::entity create();

    void create(static_entity_t static_entity, entity_version_t version);

    bool has_entity(static_entity_t static_entity) const;

    [[nodiscard]] static_entity_t increase_ref(entt::entity entity);

    [[nodiscard]] entt::entity increase_ref(static_entity_t static_entity);

    entt::entity decrease_ref(static_entity_t static_entity);

    [[nodiscard]] static_entity_t get_static_entity(entt::entity entt) const;

    [[nodiscard]] entt::entity get_entity(static_entity_t static_entity) const;

    [[nodiscard]] entity_version_t get_version(static_entity_t entity) const;

    void set_version(static_entity_t entity, entity_version_t version);

    entity_version_t increment_version(static_entity_t entity);

    const std::unordered_map<static_entity_t, entity_version_t> &get_versions() const {
        return this->versions;
    }
};
}

#endif //ECS_HISTORY_STATIC_ENTITY_HPP
