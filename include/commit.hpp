//
// Created by felix on 17.12.25.
//

#ifndef ECS_NET_COMMIT_HPP
#define ECS_NET_COMMIT_HPP

#include <entt/entt.hpp>
#include "change.hpp"

template<typename T>
class component_commit_t final {
    std::vector<std::unique_ptr<component_change_t<T> > > changes;

public:
    explicit component_commit_t(const entt::id_type id = entt::type_hash<T>::value())
        : id(id) {
    }

    [[nodiscard]] size_t size() const {
        size_t size = 0;
        for (const auto &change: this->changes) {
            size += change->size();
        }
        return size;
    }

    [[nodiscard]] std::unique_ptr<component_commit_t> invert() const {
        auto new_commit = component_commit_t{id};

        for (const auto &change: this->changes) {
            new_commit.add_change(change->invert());
        }

        return std::move(new_commit);
    }

    void add_change(const std::shared_ptr<component_change_t<T> > &change) {
        this->changes.emplace_back(std::move(change));
    }

    void supply(component_change_supplier_t<T> &supplier) {
        for (const auto &change: this->changes) {
            change->apply(supplier);
        }
    }

private:
    entt::id_type id;
};

#endif //ECS_NET_COMMIT_HPP
