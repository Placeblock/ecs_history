//
// Created by felix on 12/24/25.
//

#ifndef ECS_HISTORY_CHANGE_HPP
#define ECS_HISTORY_CHANGE_HPP
#include "ecs_history/change.hpp"

namespace ecs_history::serialization {

template<typename Archive, typename Type>
std::unique_ptr<change_t<Type> > deserialize_change(Archive &archive) {
    static_entity_t static_entity;
    archive(static_entity);
    change_type_t change_type;
    archive(change_type);
    switch (change_type) {
    case change_type_t::CONSTRUCT: {
        Type value;
        archive(value);
        return std::make_unique<construct_change_t<Type> >(static_entity, value);
    }
    case change_type_t::UPDATE: {
        Type old_value;
        archive(old_value);
        Type new_value;
        archive(new_value);
        return std::make_unique<update_change_t<Type> >(static_entity, old_value, new_value);
    }
    case change_type_t::UPDATE_ONLY_NEW: {
        Type new_value;
        archive(new_value);
        return std::make_unique<update_change_t<Type> >(static_entity, Type{}, new_value);
    }
    case change_type_t::DESTRUCT: {
        Type old_value;
        archive(old_value);
        return std::make_unique<destruct_change_t<Type> >(static_entity, old_value);
    }
    case change_type_t::DESTRUCT_ONLY_NEW: {
        return std::make_unique<destruct_change_t<Type> >(static_entity, Type{});
    }
    default:
        throw std::runtime_error("Invalid change type while deserializing change");
    }
}
}

#endif //ECS_HISTORY_CHANGE_HPP