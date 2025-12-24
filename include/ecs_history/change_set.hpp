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

    virtual void supply(any_change_supplier_t &supplier) const = 0;

    virtual void for_entity(std::function<void(static_entity_t static_entity)> callback) const = 0;

    [[nodiscard]] virtual std::unique_ptr<base_change_set_t> invert() const = 0;

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

    [[nodiscard]] size_t size() const {
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

    void supply(change_supplier_t<T> &supplier) {
        for (const auto &change : this->changes) {
            change->apply(supplier);
        }
    }

    class meta_change_supplier_t final : public change_supplier_t<T> {
        any_change_supplier_t &any_supplier;

    public:
        explicit meta_change_supplier_t(any_change_supplier_t &any_supplier)
            : any_supplier(any_supplier) {
        }

        void apply(const construct_change_t<T> &c) override {
            entt::meta_any value{c.value};
            any_supplier.apply_construct(c.static_entity, value);
        }

        void apply(const update_change_t<T> &c) override {
            entt::meta_any old_value{c.old_value};
            entt::meta_any new_value{c.new_value};
            any_supplier.apply_update(c.static_entity, old_value, new_value);
        }

        void apply(const destruct_change_t<T> &c) override {
            entt::meta_any old_value{c.old_value};
            any_supplier.apply_destruct(c.static_entity, old_value);
        }
    };

    void supply(any_change_supplier_t &any_supplier) const override {
        meta_change_supplier_t meta_supplier{any_supplier};
        for (const auto &change : this->changes) {
            change->apply(meta_supplier);
        }
    }

    void for_entity(std::function<void(static_entity_t static_entity)> callback) const override {
        for (const auto &change : this->changes) {
            callback(change->static_entity);
        }
    }

    [[nodiscard]] size_t count() const override {
        return this->changes.size();
    }

};
}

#endif //ECS_HISTORY_CHANGE_SET_HPP
