//
// Created by felix on 12/24/25.
//

#ifndef ECS_HISTORY_GATHER_STRATEGY_HPP
#define ECS_HISTORY_GATHER_STRATEGY_HPP
#include <memory>
#include <vector>

#include "ecs_history/change_set.hpp"

namespace ecs_history {
class gather_strategy {
public:
    virtual std::vector<std::unique_ptr<base_change_set_t> > get_change_sets() = 0;

    virtual std::vector<static_entity_t> get_created_entities() = 0;

    virtual std::vector<static_entity_t> get_destroyed_entities() = 0;

    virtual void disable() = 0;
    virtual void enable() = 0;

    virtual ~gather_strategy() = default;
};
}

#endif //ECS_HISTORY_GATHER_STRATEGY_HPP