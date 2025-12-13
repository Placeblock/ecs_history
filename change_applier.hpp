//
// Created by felix on 13.12.25.
//

#ifndef ECS_HISTORY_CHANGE_APPLIER_H
#define ECS_HISTORY_CHANGE_APPLIER_H

#include "change.hpp"

template<typename T>
class component_change_applier_t final : public component_change_supplier_t<T> {
    entt::registry &reg;

public:
    explicit component_change_applier_t(entt::registry &reg)
        : reg(reg) {
    }

    void apply(const construct_change_t<T> &c) override {
    }

    void apply(const update_change_t<T> &c) override {
    }

    void apply(const destruct_change_t<T> &c) override {
    }
};

class entity_change_supplier_t {
public:
    virtual ~entity_change_supplier_t() = default;

    virtual void apply(const entity_create_change_t& c) = 0;
    virtual void apply(const entity_destroy_change_t& c) = 0;
};

class entity_change_applier_t final : public entity_change_supplier_t {
    entt::registry &reg;

public:
    explicit entity_change_applier_t(entt::registry &reg)
        : reg(reg) {
    }

    void apply(const entity_create_change_t &c) override {
    }

    void apply(const entity_destroy_change_t &c) override {

    }
};


#endif //ECS_HISTORY_CHANGE_APPLIER_H