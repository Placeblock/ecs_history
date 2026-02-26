//
// Created by felix on 12/24/25.
//

#ifndef ECS_HISTORY_SERIALIZATION_HPP
#define ECS_HISTORY_SERIALIZATION_HPP
#include "change.hpp"
#include "component.hpp"
#include "ecs_history/change_applier.hpp"
#include "ecs_history/commit.hpp"
#include "ecs_history/static_entity.hpp"
#include <entt/entt.hpp>

namespace ecs_history::serialization {
template<typename Archive>
void serialize_storage_info(Archive &archive, entt::id_type &&type, size_t &&size) {
    archive(type);
    archive(static_cast<uint32_t>(size));
}

template<typename Archive, bool Serialize>
void serialize_entity_component(Archive &archive,
                                static_entity_t &static_entity,
                                entt::meta_any value) {
    archive(static_entity);
    serialize_component<Archive, Serialize>(archive, value.as_ref());
}

template<typename Archive>
void serialize_storage(Archive &archive,
                       const entt::basic_sparse_set<> &storage,
                       const static_entities_t &static_entities) {
    const auto meta = entt::resolve(storage.info().hash());
    serialize_storage_info(archive, storage.info().hash(), storage.size());
    for (const auto &entt : storage) {
        static_entity_t static_entity = static_entities.get_static_entity(entt);
        const void *raw_value = storage.value(entt);
        entt::meta_any value = meta.from_void(raw_value);
        serialize_entity_component<Archive, true>(archive, static_entity, value);
    }
}

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
        entt::meta_type type = entt::resolve(id);
        if (!type) {
            spdlog::error("invalid component type {} while deserializing registry", id);
            throw std::runtime_error("invalid component type while deserializing regisdtry");
        }
        uint32_t storage_size;
        archive(storage_size);
        for (int j = 0; j < storage_size; ++j) {
            entt::meta_any value = type.construct();
            if (!value) {
                throw std::runtime_error("construction of component failed");
            }
            static_entity_t static_entity;
            serialize_entity_component<Archive, false>(archive, static_entity, value.as_ref());
            any_change_applier_t::apply(reg, static_entities.get_entity(static_entity), value);
        }
    }
}

template<typename Archive>
void serialize_registry(Archive &archive, entt::registry &reg) {
    auto &static_entities = reg.ctx().get<static_entities_t>();
    auto &version_handler = reg.ctx().get<entity_version_handler_t>();

    archive(static_cast<uint32_t>(version_handler.versions.size()));
    for (const auto &[static_entity, version] : version_handler.versions) {
        archive(static_entity);
        archive(version);
    }

    uint16_t storages = std::distance(reg.storage().begin(), reg.storage().end());
    archive(storages);

    for (auto [id, storage] : reg.storage()) {
        if (const auto meta = entt::resolve(id); !meta) {
            continue;
        }
        serialize_storage(archive, storage, static_entities);
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
    change_serializer<Archive> serializer{archive};
    for (const auto &change_set : commit.change_sets) {
        archive(change_set->id);
        uint32_t count = change_set->count();
        archive(count);
        change_set->supply(serializer);
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

template<typename Archive, typename Type>
std::unique_ptr<base_change_set_t> deserialize_change_set(Archive &archive) {
    auto change_set = std::make_unique<change_set_t<Type> >();
    uint32_t count;
    archive(count);
    for (uint32_t i = 0; i < count; ++i) {
        std::unique_ptr<change_t<Type> > change = deserialize_change<
            Archive, Type>(archive);
        change_set->add_change(std::move(change));
    }
    return std::move(change_set);
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
        entt::meta_type type = entt::resolve(id);
        auto deserialize_changes_func = type.func(deserialize_change_set_func);
        if (!deserialize_changes_func) {
            const auto type_name = std::string{type.info().name()};
            throw std::runtime_error(
                "could not find deserialize change set function for type " + type_name);
        }
        auto change_set = deserialize_changes_func.invoke({}, entt::forward_as_meta(archive));
        if (!change_set) {
            throw std::runtime_error("failed to deserialize change set");
        }
        std::unique_ptr<base_change_set_t> &changes = change_set.template cast<
            std::unique_ptr<base_change_set_t> &>();
        change_sets.push_back(std::move(changes));
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
