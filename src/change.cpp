//
// Created by felix on 11.12.25.
//

#include "change.hpp"
#include "change_applier.hpp"

std::unique_ptr<entity_change_t> entity_create_change_t::invert() const {
    std::unique_ptr<entity_change_t> change = std::make_unique<entity_destroy_change_t>(this->entt);
    return change;
}

void entity_create_change_t::apply(entity_change_supplier_t &applier) const {
    applier.apply(*this);
}

std::unique_ptr<entity_change_t> entity_destroy_change_t::invert() const {
    return std::make_unique<entity_create_change_t>(this->entt);
}

void entity_destroy_change_t::apply(entity_change_supplier_t &applier) const {
    applier.apply(*this);
}
