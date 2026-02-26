//
// Created by felix on 12/24/25.
//

#ifndef ECS_HISTORY_COMPONENT_HPP
#define ECS_HISTORY_COMPONENT_HPP
#include <entt/entt.hpp>
#include <cereal/types/string.hpp>
#include <cereal/archives/portable_binary.hpp>


namespace ecs_history::serialization {

constexpr entt::id_type custom_serialize_func_id = entt::hashed_string{"serialize"};
constexpr entt::id_type custom_deserialize_func_id = entt::hashed_string{"deserialize"};

/**
 *
 * @tparam Archive The type of Archive to write to / read from
 * @tparam Serialize Whether to serialize or deserialize
 * @param archive The archive
 * @param value The value
 */
template<typename Archive, bool Serialize>
void serialize_component(Archive &archive, entt::meta_any value) {
    const entt::meta_type type = value.type();

    constexpr auto funcId = Serialize ? custom_serialize_func_id : custom_deserialize_func_id;
    if (auto func = type.func(funcId); func) {
        if (!func.invoke({}, entt::forward_as_meta(archive), value.as_ref())) {
            throw std::runtime_error("Could not (de)serialize component with func");
        }
        return;
    }

    throw std::runtime_error("No (de-)serialization function found");
}
}

#endif //ECS_HISTORY_COMPONENT_HPP