//
// Created by felix on 17.12.25.
//

#ifndef ECS_HISTORY_CHANGE_SET_HPP
#define ECS_HISTORY_CHANGE_SET_HPP

#include <entt/entt.hpp>
#include "change.hpp"

namespace ecs_history {

class base_change_set_t {
public:
    entt::id_type id;

    explicit base_change_set_t(entt::id_type id);

    virtual void for_entity(std::function<void(static_entity_t static_entity)> callback) const = 0;

    [[nodiscard]] virtual std::unique_ptr<base_change_set_t> invert() const = 0;

    virtual void apply(entt::registry &reg, static_entities_t &entities) const = 0;

    virtual void serialize(cereal::PortableBinaryOutputArchive &archive) const = 0;

    [[nodiscard]]

    virtual size_t size() const = 0;

    [[nodiscard]] virtual size_t count() const = 0;

    virtual ~base_change_set_t() = default;
};

template<typename T>
class change_set_t final : public base_change_set_t {
    std::vector<std::unique_ptr<change_t<T> > > changes;

public:
    explicit change_set_t(const entt::id_type id = entt::type_hash<T>::value())
        : base_change_set_t(id) {
    }

    explicit change_set_t(std::vector<std::unique_ptr<change_t<T> > > &changes,
                          const entt::id_type id = entt::type_hash<T>::value())
        : base_change_set_t(id), changes(std::move(changes)) {
    }

    [[nodiscard]] size_t size() const override {
        size_t size = 0;
        for (const auto &change : this->changes) {
            size += change->size();
        }
        return size;
    }

    [[nodiscard]] std::unique_ptr<base_change_set_t> invert() const override {
        auto new_commit = std::make_unique<change_set_t>(this->id);

        for (const auto &change : this->changes | std::views::reverse) {
            new_commit->add_change(change->invert());
        }

        return std::move(new_commit);
    }

    void add_change(std::unique_ptr<change_t<T> > change) {
        this->changes.emplace_back(std::move(change));
    }

    void for_entity(
        const std::function<void(static_entity_t static_entity)> callback) const override {
        for (const auto &change : this->changes) {
            callback(change->static_entity);
        }
    }

    [[nodiscard]] size_t count() const override {
        return this->changes.size();
    }

    void apply(entt::registry &reg, static_entities_t &entities) const override {
        change_applier_t<T> applier(reg.storage<T>(id), entities);
        for (const auto &change : this->changes) {
            change->apply(applier);
        }
    }

    void serialize(cereal::PortableBinaryOutputArchive &archive) const override {
        change_serializer_t<T> serializer{archive};
        for (const auto &change : this->changes) {
            change->apply(serializer);
        }
    }
};
}

#endif //ECS_HISTORY_CHANGE_SET_HPP
