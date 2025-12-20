//
// Created by felix on 11.12.25.
//

#include "ecs_history/change.hpp"
#include "ecs_history/change_applier.hpp"

using namespace ecs_history;

std::unique_ptr<entity_change_t> entity_create_change_t::invert() const {
    return std::make_unique<entity_destroy_change_t>(this->static_entity);
}

void entity_create_change_t::apply(entity_change_supplier_t &applier) const {
    applier.apply(*this);
}

std::unique_ptr<entity_change_t> entity_destroy_change_t::invert() const {
    return std::make_unique<entity_create_change_t>(this->static_entity);
}

void entity_destroy_change_t::apply(entity_change_supplier_t &applier) const {
    applier.apply(*this);
}
