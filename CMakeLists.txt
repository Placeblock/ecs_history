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

add_library(ecs_history
        include/ecs_history/change.hpp
        include/ecs_history/change_applier.hpp
        include/ecs_history/gather_strategy/change_mixin.hpp
        include/ecs_history/gather_strategy/change_reactive_mixin.hpp
        include/ecs_history/change_set.hpp
        include/ecs_history/static_entity.hpp
        include/ecs_history/gather_strategy/monitor.hpp
        include/ecs_history/gather_strategy/registry.hpp)
target_include_directories(ecs_history PUBLIC include)
target_sources(ecs_history PUBLIC
        src/change.cpp
        src/change_applier.cpp
        src/change_set.cpp
        src/static_entity.cpp)
target_link_libraries(ecs_history EnTT::EnTT)

