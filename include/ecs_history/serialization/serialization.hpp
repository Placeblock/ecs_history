//
// Created by felix on 12/24/25.
//

#ifndef ECS_HISTORY_SERIALIZATION_HPP
#define ECS_HISTORY_SERIALIZATION_HPP
#include "ecs_history/commit.hpp"
#include "ecs_history/component/component_context.hpp"
#include "ecs_history/static_entity.hpp"
#include <entt/entt.hpp>

namespace ecs_history::serialization {

template<typename Archive>
void deserialize_registry(Archive &archive, entt::registry &reg) {
    auto &static_entities = reg.ctx().get<static_entities_t>();
    auto &version_handler = reg.ctx().get<entity_version_handler_t>();
    uint32_t entities;
    archive(entities);
    for (int i = 0; i < entities; ++i) {
        static_entity_t static_entity;
        archive(static_entity);
        entity_version_t version;
        archive(version);
        version_handler.add_entity(static_entity, version);
    }

    uint16_t storage_count;
    archive(storage_count);
    for (int i = 0; i < storage_count; ++i) {
        entt::id_type id;
        archive(id);
        const std::unique_ptr<base_change_set_t> change_set = context::deserialize_change_set(
            id,
            archive);
        change_set->apply(reg, static_entities);
    }
}

template<typename Archive>
void serialize_registry(Archive &archive, entt::registry &reg) {
    const auto &static_entities = reg.ctx().get<static_entities_t>();
    auto &version_handler = reg.ctx().get<entity_version_handler_t>();

    archive(static_cast<uint32_t>(version_handler.versions.size()));
    for (const auto &[static_entity, version] : version_handler.versions) {
        archive(static_entity);
        archive(version);
    }

    uint16_t storages = std::distance(reg.storage().begin(), reg.storage().end());
    archive(storages);

    for (auto [id, storage] : reg.storage()) {
        archive(storage.info().hash());
        archive(static_cast<uint32_t>(storage.size()));
        for (const auto &entt : storage) {
            static_entity_t static_entity = static_entities.get_static_entity(entt);
            archive(static_entity);
            archive(change_type_t::CONSTRUCT);
            const void *raw_value = storage.value(entt);
            context::serialize_raw(id, raw_value, archive);
        }
    }
}

template<typename Archive>
void serialize_commit(Archive &archive, commit_t &commit) {
    uint32_t entity_version_count = commit.entity_versions.size();
    archive(entity_version_count);
    for (const auto &[static_entity, version] : commit.entity_versions) {
        archive(static_entity);
        archive(version);
    }
    uint16_t change_sets = commit.change_sets.size();
    archive(change_sets);
    for (const auto &change_set : commit.change_sets) {
        archive(change_set->id);
        uint32_t count = change_set->count();
        archive(count);
        change_set->serialize(archive);
    }
}

template<typename Archive>
std::unordered_map<static_entity_t, entity_version_t>
deserialize_commit_entity_versions(Archive &archive) {
    std::unordered_map<static_entity_t, entity_version_t> entity_versions;
    uint32_t entity_version_count;
    archive(entity_version_count);
    for (int i = 0; i < entity_version_count; ++i) {
        static_entity_t static_entity;
        archive(static_entity);
        entity_version_t version;
        archive(version);
        entity_versions[static_entity] = version;
    }
    return entity_versions;
}

constexpr entt::id_type deserialize_change_set_func = entt::hashed_string{"deserialize_change_set"};

template<typename Archive>
std::vector<std::unique_ptr<base_change_set_t> > deserialize_commit_changes(
    Archive &archive) {
    std::vector<std::unique_ptr<base_change_set_t> > change_sets;
    uint16_t change_set_count;
    archive(change_set_count);
    for (uint16_t i = 0; i < change_set_count; ++i) {
        entt::id_type id;
        archive(id);
        change_sets.push_back(context::deserialize_change_set(id, archive));
    }
    return change_sets;
}

template<typename Archive>
std::unique_ptr<commit_t> deserialize_commit(Archive &archive) {
    auto entity_versions = serialization::deserialize_commit_entity_versions(archive);
    auto changes = serialization::deserialize_commit_changes(archive);
    return std::make_unique<commit_t>(
        std::move(entity_versions),
        std::move(changes));
}
}

#endif //ECS_HISTORY_SERIALIZATION_HPP
