//
// Created by felix on 2/27/26.
//

#ifndef ECS_HISTORY_COMPONENT_CONTEXT_HPP
#define ECS_HISTORY_COMPONENT_CONTEXT_HPP
#include "ecs_history/change_set.hpp"

namespace ecs_history::context {

struct component_t {
    virtual std::unique_ptr<base_change_set_t> deserialize_change_set(
        cereal::PortableBinaryInputArchive &archive) = 0;

    virtual void serialize_raw(const void *raw, cereal::PortableBinaryOutputArchive &archive) = 0;

    virtual ~component_t() = default;
};

inline std::unordered_map<entt::id_type, std::unique_ptr<component_t> > components{};

static std::unique_ptr<base_change_set_t> deserialize_change_set(const entt::id_type id,
                                                                 cereal::PortableBinaryInputArchive
                                                                 &archive) {
    component_t &component = *components[id];
    return component.deserialize_change_set(archive);
}

static void serialize_raw(const entt::id_type id,
                          const void *raw,
                          cereal::PortableBinaryOutputArchive &archive) {
    component_t &component = *components[id];
    component.serialize_raw(raw, archive);
}

template<typename T>
void register_component(std::unique_ptr<component_t> &component) {
    const entt::id_type id = entt::type_id<T>().hash();
    components[id] = std::move(component);
}

}


#endif //ECS_HISTORY_COMPONENT_CONTEXT_HPP