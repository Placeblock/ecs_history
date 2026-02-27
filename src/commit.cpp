//
// Created by felix on 12/24/25.
//

#include "ecs_history/commit.hpp"

using namespace ecs_history;

commit_id::commit_id(const uint64_t part1, const uint64_t part2) : part1{part1}, part2{part2} {
}

commit_id::commit_id() : part1(0), part2(0) {
}

bool commit_id::operator==(const commit_id &other) const {
    return part1 == other.part1 && part2 == other.part2;
}

bool commit_id::operator!=(const commit_id &other) const {
    return !(*this == other);
}

commit_id_generator_t::commit_id_generator_t() : gen{rd()} {
}

commit_id commit_id_generator_t::next() {
    return commit_id{gen(), gen()};
}

commit_t::commit_t(
    std::unordered_map<static_entity_t, entity_version_t> entity_versions,
    std::vector<std::unique_ptr<base_change_set_t> > change_sets)
    : entity_versions(std::move(entity_versions)),
      change_sets(std::move(change_sets)) {
}

std::unique_ptr<commit_t> commit_t::invert() {
    auto inverted_commit = std::make_unique<commit_t>();
    for (const auto &base_change_set : this->change_sets) {
        inverted_commit->change_sets.push_back(base_change_set->invert());
    }
    inverted_commit->undo = !this->undo;
    if (this->undo) {
        for (auto [entity, version] : this->entity_versions) {
            inverted_commit->entity_versions[entity] = --version;
        }
    } else {
        for (auto [entity, version] : this->entity_versions) {
            inverted_commit->entity_versions[entity] = ++version;
        }
    }
    return inverted_commit;
}

size_t commit_t::size() const {
    size_t size = 0;
    for (const auto &change_set : this->change_sets) {
        size += change_set->size();
    }
    return size;
}

std::unique_ptr<commit_t> ecs_history::create_commit(
    std::vector<std::unique_ptr<base_storage_monitor_t> > &monitors,
    entity_version_handler_t &version_handler) {
    auto commit = std::make_unique<commit_t>();
    for (const auto &monitor : monitors) {
        commit->change_sets.push_back(monitor->commit());
    }

    std::unordered_set<static_entity_t> commit_entities;
    for (const auto &change_set : commit->change_sets) {
        change_set->for_entity(
            [&commit_entities](const static_entity_t &static_entity) {
                commit_entities.emplace(static_entity);
            });
    }
    for (const static_entity_t &static_entity : commit_entities) {
        commit->entity_versions[static_entity] = version_handler.increment_version(
            static_entity);
    }

    return std::move(commit);
}

bool ecs_history::can_apply_commit(entt::registry &reg, const commit_t &commit) {
    auto version_handler = reg.ctx().get<entity_version_handler_t>();
    return std::ranges::all_of(commit.entity_versions,
                               [&](const auto &pair) {
                                   return pair.second == version_handler.get_version(
                                              pair.first);
                               });
}

void ecs_history::apply_commit(entt::registry &reg,
                               const std::vector<std::unique_ptr<base_storage_monitor_t> > &
                               monitors,
                               const commit_t &commit) {
    auto &version_handler = reg.ctx().get<entity_version_handler_t>();
    auto &static_entities = reg.ctx().get<static_entities_t>();
    for (auto &monitor : monitors) {
        monitor->disable();
    }

    for (const auto &change_set : commit.change_sets) {
        change_set->apply(reg, static_entities);
    }
    for (const auto &[entity, version] : commit.entity_versions) {
        commit.undo
            ? version_handler.set_version(entity, version - 1)
            : version_handler.set_version(entity, version + 1);
    }

    for (auto &monitor : monitors) {
        monitor->enable();
    }
}