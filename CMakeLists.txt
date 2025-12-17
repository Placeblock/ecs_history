include(FetchContent)

cmake_minimum_required(VERSION 4.0)
project(ecs_history)

set(CMAKE_CXX_STANDARD 20)

FetchContent_Declare(
        entt
        GIT_REPOSITORY https://github.com/skypjack/entt.git
        GIT_TAG v3.15.0
)
FetchContent_MakeAvailable(entt)

add_library(ecs-history
        include/change.hpp
        include/change_applier.hpp
        include/change_mixin.hpp
        include/change_reactive_mixin.hpp
        include/commit.hpp
        include/static_entity.hpp
        include/monitor.hpp
        include/registry.hpp)
target_include_directories(ecs-history PUBLIC include)
target_sources(ecs-history PUBLIC
        src/change.cpp
        src/change_applier.cpp
        src/commit.cpp
        src/static_entity.cpp)
target_link_libraries(ecs-history EnTT::EnTT)

