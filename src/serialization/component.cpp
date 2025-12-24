//
// Created by felix on 12/24/25.
//

#include "ecs_history/serialization/component.hpp"

void ecs_history::serialization::initialize_component_meta_types() {
    SERIALIZE_SIMPLE(uint64_t);
    SERIALIZE_SIMPLE(uint32_t);
    SERIALIZE_SIMPLE(uint16_t);
    SERIALIZE_SIMPLE(uint8_t);
    SERIALIZE_SIMPLE(int64_t);
    SERIALIZE_SIMPLE(int32_t);
    SERIALIZE_SIMPLE(int16_t);
    SERIALIZE_SIMPLE(int8_t);
    SERIALIZE_SIMPLE(char);
    SERIALIZE_SIMPLE(std::string);
}