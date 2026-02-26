//
// Created by felix on 1/16/26.
//

#ifndef ECS_HISTORY_HISTORY_HPP
#define ECS_HISTORY_HISTORY_HPP
#include "ecs_history/commit.hpp"
#include <spdlog/spdlog.h>

namespace ecs_history {
const commit_id FIRST_BASE_ID{0, 0};

class history_t {
    entt::registry &reg;
    std::vector<std::unique_ptr<base_storage_monitor_t> > &monitors;

public:
    struct history_commit_t {
        commit_id base_id;
        commit_id id;
        std::unique_ptr<commit_t> commit;
    };

    std::list<history_commit_t> commits{};

    explicit history_t(entt::registry &reg,
                       std::vector<std::unique_ptr<base_storage_monitor_t> > &monitors)
        : reg(reg),
          monitors(monitors) {
    }

    void apply_commit(const commit_id base_id,
                      const commit_id id,
                      std::unique_ptr<commit_t> &commit) {
        spdlog::debug("applying commit {} -> {}", base_id, id);
        auto rview = this->commits | std::views::reverse;
        const auto rit = std::ranges::find_if(rview,
                                              [&base_id](const history_commit_t &cmt) {
                                                  return cmt.id == base_id;
                                              });
        // a base_id of {0, 0} means that this was an initial commit
        if (rit == rview.end() && base_id.part1 != 0 && base_id.part2 != 0) {
            spdlog::warn("commit with id == base_id not found. cannot apply commit");
            return;
            throw std::runtime_error("commit with id == base_id not found. cannot apply commit");
        }
        if (const auto it = rit.base(); it == this->commits.end()) {
            // The new commit's base_id is the last commit's id -> just insert
            spdlog::debug("commit is recent. applying");
            ecs_history::apply_commit(this->reg, this->monitors, *commit);
            this->commits.emplace_back(base_id, id, std::move(commit));
        } else {
            const auto base_it = std::prev(it); // The commit with the provided base_id
            // Rollback commits after commit with base_id = id
            spdlog::debug("rolling back {} commits", std::distance(it, this->commits.end()));
            for (auto rollback_it = --this->commits.end(); rollback_it != base_it; --rollback_it) {
                spdlog::debug("rolling back {}{}", rollback_it->id.part1, rollback_it->id.part2);
                ecs_history::apply_commit(this->reg,
                                          this->monitors,
                                          *rollback_it->commit->invert());
            }
            // Insert the new commit
            if (!can_apply_commit(this->reg, *commit)) {
                throw std::runtime_error("Failed to apply commit received from parent");
            }
            spdlog::debug("applying commit");
            ecs_history::apply_commit(this->reg, this->monitors, *commit);
            auto inserted_it = this->commits.insert(it, {base_id, id, std::move(commit)});
            // Try to reapply rolledback commits
            auto applyagain_it = ++inserted_it;
            for (; applyagain_it != this->commits.end(); ++applyagain_it) {
                spdlog::debug("trying to rebase {}{}",
                              applyagain_it->id.part1,
                              applyagain_it->id.part2);
                if (can_apply_commit(this->reg, *applyagain_it->commit)) {
                    spdlog::debug("rebased {}{}",
                                  applyagain_it->id.part1,
                                  applyagain_it->id.part2);
                    ecs_history::apply_commit(this->reg,
                                              this->monitors,
                                              *applyagain_it->commit);
                } else {
                    break;
                }
            }
            spdlog::debug("rebased {} commits", std::distance(inserted_it, applyagain_it));
            spdlog::debug("removing {} commits (could not be rebased)",
                          std::distance(applyagain_it, this->commits.end()));
            // Remove commits we could not apply
            this->commits.erase(applyagain_it, this->commits.end());
        }
    }

    commit_id push_commit(const commit_id id,
                          std::unique_ptr<commit_t> &commit) {
        spdlog::debug("pushing commit {}{}", id.part1, id.part2);
        if (this->commits.empty()) {
            this->commits.push_back({FIRST_BASE_ID, id, std::move(commit)});
            ecs_history::apply_commit(this->reg,
                                      this->monitors,
                                      *this->commits.back().commit);
            return {0, 0};
        }
        const auto new_base_id = commits.back().id;
        this->commits.push_back({new_base_id, id, std::move(commit)});
        ecs_history::apply_commit(this->reg, this->monitors, *this->commits.back().commit);
        return new_base_id;
    }

    void add_commit(const commit_id base_id,
                    const commit_id id,
                    std::unique_ptr<commit_t> &commit) {
        this->commits.push_back({base_id, id, std::move(commit)});
    }

    commit_id add_commit(const commit_id id,
                         std::unique_ptr<commit_t> &commit) {
        if (this->commits.empty()) {
            this->commits.push_back({FIRST_BASE_ID, id, std::move(commit)});
            return {0, 0};
        }
        const auto new_base_id = commits.back().id;
        this->commits.push_back({new_base_id, id, std::move(commit)});
        return new_base_id;
    }

    bool is_known_commit(const commit_id id) {
        return std::ranges::any_of(this->commits,
                                   [&id](const auto &commit) {
                                       return commit.id == id;
                                   });
    }
};
}

#endif //ECS_HISTORY_HISTORY_HPP