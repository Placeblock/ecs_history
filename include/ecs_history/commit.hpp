//
// Created by felix on 12/20/25.
//

#ifndef ECS_HISTORY_COMMIT_HPP
#define ECS_HISTORY_COMMIT_HPP

#include <vector>
#include <random>

#include "ecs_history/change_set.hpp"
#include "entity_version.hpp"
#include "ecs_history/gather_strategy/gather_strategy.hpp"

namespace ecs_history {

struct commit_id {
    uint64_t part1;
    uint64_t part2;

    commit_id(uint64_t part1, uint64_t part2);

    bool operator==(const commit_id &other) const;

    bool operator!=(const commit_id &other) const;

    template<class Archive>
    void serialize(Archive &ar) {
        ar(part1, part2);
    }
};

class commit_id_generator_t {
    std::random_device rd;

    std::mt19937_64 gen;

public:
    commit_id_generator_t();

    commit_id next();
};

struct commit_t {
    bool undo = false;
    std::unordered_map<static_entity_t, entity_version_t> entity_versions;
    std::vector<static_entity_t> created_entities;
    std::vector<std::unique_ptr<base_change_set_t> > change_sets;
    std::vector<static_entity_t> destroyed_entities;

    commit_t() = default;

    commit_t(std::unordered_map<static_entity_t, entity_version_t> entity_versions,
             std::vector<static_entity_t> created_entities,
             std::vector<std::unique_ptr<base_change_set_t> > change_sets,
             std::vector<static_entity_t> destroyed_entities);

    commit_t(commit_t &commit) = delete;

    commit_t &operator=(commit_t &commit) = delete;

    commit_t(commit_t &&commit) = default;

    std::unique_ptr<commit_t> invert();
};

std::unique_ptr<commit_t> create_commit(gather_strategy &gather_strategy,
                                        entity_version_handler_t &version_handler);


bool can_apply_commit(entt::registry &reg, const commit_t &commit);

void apply_commit(entt::registry &reg, gather_strategy &gather_strategy, const commit_t &commit);

}

#endif //ECS_HISTORY_COMMIT_HPP