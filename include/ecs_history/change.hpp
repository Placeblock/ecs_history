//
// Created by felix on 11.12.25.
//

#ifndef ECS_HISTORY_CHANGE_H
#define ECS_HISTORY_CHANGE_H
#include <entt/meta/meta.hpp>

#include "static_entity.hpp"

namespace ecs_history {
template<typename T>
class change_supplier_t;

template<typename T>
struct change_t {
    const static_entity_t static_entity;

    explicit change_t(const static_entity_t static_entity) : static_entity(static_entity) {
    }

    [[nodiscard]] virtual size_t size() const {
        return sizeof(static_entity_t);
    }

    [[nodiscard]] virtual std::unique_ptr<change_t> invert() const = 0;

    virtual void apply(change_supplier_t<T> &applier) const = 0;

    virtual ~change_t() = default;
};

template<typename T>
struct destruct_change_t;

template<typename T>
struct construct_change_t final : change_t<T> {
    const T value;

    explicit construct_change_t(const static_entity_t static_entity, T value)
        : change_t<T>(static_entity), value(value) {
    }

    [[nodiscard]] size_t size() const override {
        return sizeof(T) + change_t<T>::size();
    }

    [[nodiscard]] std::unique_ptr<change_t<T> > invert() const override {
        return std::make_unique<destruct_change_t<T> >(this->static_entity, value);
    }

    void apply(change_supplier_t<T> &applier) const override {
        applier.apply(*this);
    }
};

template<typename T>
struct update_change_t final : change_t<T> {
    const T old_value;
    const T new_value;

    explicit update_change_t(const static_entity_t static_entity, T old_value, T new_value)
        : change_t<T>(static_entity), old_value(old_value), new_value(new_value) {
    }

    [[nodiscard]] size_t size() const override {
        return 2 * sizeof(T) + change_t<T>::size();
    }

    [[nodiscard]] std::unique_ptr<change_t<T> > invert() const override {
        return std::make_unique<update_change_t>(this->static_entity, new_value, old_value);
    }

    void apply(change_supplier_t<T> &applier) const override {
        applier.apply(*this);
    }
};

template<typename T>
struct destruct_change_t final : change_t<T> {
    const T old_value;

    explicit destruct_change_t(const static_entity_t static_entity, T old_value)
        : change_t<T>(static_entity), old_value(old_value) {
    }

    [[nodiscard]] size_t size() const override {
        return sizeof(T) + change_t<T>::size();
    }

    [[nodiscard]] std::unique_ptr<change_t<T> > invert() const override {
        return std::make_unique<construct_change_t<T> >(this->static_entity, old_value);
    }

    void apply(change_supplier_t<T> &applier) const override {
        applier.apply(*this);
    }
};

template<typename T>
class change_supplier_t {
public:
    virtual void apply(const construct_change_t<T> &c) = 0;

    virtual void apply(const update_change_t<T> &c) = 0;

    virtual void apply(const destruct_change_t<T> &c) = 0;

    virtual ~change_supplier_t() = default;
};

class any_change_supplier_t {
public:
    virtual void apply_construct(static_entity_t static_entity, entt::meta_any &value) = 0;

    virtual void apply_update(static_entity_t static_entity,
                              entt::meta_any &old_value,
                              entt::meta_any &new_value) = 0;

    virtual void apply_destruct(static_entity_t static_entity, entt::meta_any &old_value) = 0;

    virtual ~any_change_supplier_t() = default;
};
}

#endif //ECS_HISTORY_CHANGE_H
