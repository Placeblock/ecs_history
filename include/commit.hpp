//
// Created by felix on 17.12.25.
//

#ifndef ECS_NET_COMMIT_HPP
#define ECS_NET_COMMIT_HPP

#include <entt/entt.hpp>
#include "change.hpp"


class base_component_commit_t {
public:
    virtual void supply(any_component_change_supplier_t &supplier) = 0;

    virtual ~base_component_commit_t() = default;
};

template<typename T>
class component_commit_t final : public base_component_commit_t {
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

    void add_change(std::unique_ptr<component_change_t<T> > change) {
        this->changes.emplace_back(std::move(change));
    }

    void supply(component_change_supplier_t<T> &supplier) {
        for (const auto &change: this->changes) {
            change->apply(supplier);
        }
    }

    class meta_component_change_supplier_t final : public component_change_supplier_t<T> {
        any_component_change_supplier_t &any_supplier;

    public:
        explicit meta_component_change_supplier_t(any_component_change_supplier_t &any_supplier)
            : any_supplier(any_supplier) {
        }

        void apply(const construct_change_t<T> &c) override {
            entt::meta_any value{c.value};
            any_supplier.apply_construct(value);
        }

        void apply(const update_change_t<T> &c) override {
            entt::meta_any old_value{c.old_value};
            entt::meta_any new_value{c.new_value};
            any_supplier.apply_update(old_value, new_value);
        }

        void apply(const destruct_change_t<T> &c) override {
            entt::meta_any old_value{c.old_value};
            any_supplier.apply_destruct(old_value);
        }
    };

    void supply(any_component_change_supplier_t &any_supplier) override {
        meta_component_change_supplier_t meta_supplier{any_supplier};
        for (const auto &change: this->changes) {
            change->apply(meta_supplier);
        }
    }

private:
    entt::id_type id;
};

#endif //ECS_NET_COMMIT_HPP
