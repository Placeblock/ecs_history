include(FetchContent)

cmake_minimum_required(VERSION 4.0)
project(ecs_history)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.16.0
)
FetchContent_MakeAvailable(entt)

set(SKIP_PERFORMANCE_COMPARISON ON)
FetchContent_Declare(
        cereal
        GIT_REPOSITORY https://github.com/USCiLab/cereal
        GIT_TAG v1.3.2
)
FetchContent_MakeAvailable(cereal)

add_library(ecs_history
        include/ecs_history/change.hpp
        include/ecs_history/change_applier.hpp
        include/ecs_history/gather_strategy/reactive/change_mixin.hpp
        include/ecs_history/gather_strategy/reactive/change_reactive_mixin.hpp
        include/ecs_history/change_set.hpp
        include/ecs_history/static_entity.hpp
        include/ecs_history/gather_strategy/reactive/monitor.hpp
        include/ecs_history/commit.hpp
        include/ecs_history/entity_version.hpp
        include/ecs_history/gather_strategy/gather_strategy.hpp
        include/ecs_history/gather_strategy/reactive/reactive_gather_strategy.hpp
        include/ecs_history/serialization/component.hpp
        include/ecs_history/serialization/change.hpp
        include/ecs_history/serialization/serialization.hpp
        src/gather_strategy/reactive/reactive_gather_strategy.cpp
        src/commit.cpp
        src/gather_strategy/reactive/monitor.cpp
        src/entity_version.cpp
        src/serialization/component.cpp
        src/change_applier.cpp
        src/change_set.cpp
        src/static_entity.cpp)
target_include_directories(ecs_history PUBLIC include)
target_link_libraries(ecs_history EnTT::EnTT cereal)

