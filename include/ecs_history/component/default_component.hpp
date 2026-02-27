//
// Created by felix on 2/27/26.
//

#ifndef ECS_HISTORY_DEFAULT_COMPONENT_HPP
#define ECS_HISTORY_DEFAULT_COMPONENT_HPP
#include "component_context.hpp"
#include "ecs_history/serialization/change.hpp"
#include "ecs_history/serialization/serialization.hpp"

namespace ecs_history {
template<typename T>
struct default_component_t final : context::component_t {
    std::unique_ptr<base_change_set_t>
    deserialize_change_set(cereal::PortableBinaryInputArchive &archive) override {
        auto change_set = std::make_unique<change_set_t<T> >();
        uint32_t count;
        archive(count);
        for (uint32_t i = 0; i < count; ++i) {
            std::unique_ptr<change_t<T> > change = serialization::deserialize_change<
                cereal::PortableBinaryInputArchive, T>(archive);
            change_set->add_change(std::move(change));
        }
        return std::move(change_set);
    }

    void serialize_raw(void *raw, cereal::PortableBinaryOutputArchive &archive) override {
        archive(*static_cast<T *>(raw));
    }

};
}

#endif //ECS_HISTORY_DEFAULT_COMPONENT_HPP