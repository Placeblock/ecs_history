//
// Created by felix on 11.12.25.
//

#ifndef ECS_HISTORY_CHANGE_H
#define ECS_HISTORY_CHANGE_H
#include <entt/entity/entity.hpp>
#include "static_entity.hpp"

template<typename T>
class component_change_supplier_t;
class entity_change_supplier_t;

struct entity_destroy_change_t;

struct entity_change_t {
    const static_entity_t entt;

    explicit entity_change_t(const static_entity_t entt) : entt(entt) {}

    [[nodiscard]] virtual std::unique_ptr<entity_change_t> invert() const = 0;
    [[nodiscard]] size_t size() const {
        return sizeof(static_entity_t);
    }
    virtual void apply(entity_change_supplier_t &applier) const = 0;

    virtual ~entity_change_t() = default;
};

struct entity_create_change_t final : entity_change_t {
    explicit entity_create_change_t(const static_entity_t entt) : entity_change_t(entt) {}

    [[nodiscard]] std::unique_ptr<entity_change_t> invert() const override;
    void apply(entity_change_supplier_t &applier) const override;
};

struct entity_destroy_change_t final : entity_change_t {
    explicit entity_destroy_change_t(const static_entity_t entt) : entity_change_t(entt) {}

    [[nodiscard]] std::unique_ptr<entity_change_t> invert() const override;
    void apply(entity_change_supplier_t &applier) const override;
};

template<typename T>
struct component_change_t {
    const static_entity_t entt;

    explicit component_change_t(const static_entity_t entt) : entt(entt) {}

    [[nodiscard]] virtual size_t size() const {
        return sizeof(static_entity_t);
    }

    [[nodiscard]] virtual std::unique_ptr<component_change_t> invert() const = 0;
    virtual void apply(component_change_supplier_t<T> &applier) const = 0;

    virtual ~component_change_t() = default;
};

template<typename T>
struct destruct_change_t;

template<typename T>
struct construct_change_t final : component_change_t<T> {
    const T value;

    explicit construct_change_t(const static_entity_t entt, T value)
        : component_change_t<T>(entt), value(value) {
    }

    [[nodiscard]] size_t size() const override {
        return sizeof(T) + component_change_t<T>::size();
    }

    [[nodiscard]] std::unique_ptr<component_change_t<T>> invert() const override {
        return std::make_unique<destruct_change_t<T>>(this->entt, value);
    }

    void apply(component_change_supplier_t<T> &applier) const override {
        applier.apply(*this);
    }
};

template<typename T>
struct update_change_t final  : component_change_t<T> {
    const T old_value;
    const T new_value;

    explicit update_change_t(const static_entity_t entt, T old_value, T new_value)
        : component_change_t<T>(entt), old_value(old_value), new_value(new_value) {
    }

    [[nodiscard]] size_t size() const override {
        return 2 * sizeof(T) + component_change_t<T>::size();
    }

    [[nodiscard]] std::unique_ptr<component_change_t<T>> invert() const override {
        return std::make_unique<update_change_t>(this->entt, new_value, old_value);
    }

    void apply(component_change_supplier_t<T> &applier) const override {
        applier.apply(*this);
    }
};

template<typename T>
struct destruct_change_t final  : component_change_t<T> {
    const T old_value;

    explicit destruct_change_t(const static_entity_t entt, T old_value)
        : component_change_t<T>(entt), old_value(old_value) {
    }

    [[nodiscard]] size_t size() const override {
        return sizeof(T) + component_change_t<T>::size();
    }

    [[nodiscard]] std::unique_ptr<component_change_t<T>> invert() const override {
        return std::make_unique<construct_change_t<T>>(this->entt, old_value);
    }

    void apply(component_change_supplier_t<T> &applier) const override {
        applier.apply(*this);
    }
};

template<typename T>
class component_change_supplier_t {
public:
    virtual ~component_change_supplier_t() = default;

    virtual void apply(const construct_change_t<T>& c) = 0;
    virtual void apply(const update_change_t<T>& c) = 0;
    virtual void apply(const destruct_change_t<T>& c) = 0;
};

#endif //ECS_HISTORY_CHANGE_H