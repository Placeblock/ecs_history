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

add_executable(ecs_history main.cpp
        change_mixin.hpp
        change_reactive_mixin.hpp
        change.cpp
        change.hpp
        change_applier.cpp
        change_applier.hpp
        static_entity.hpp
)
target_link_libraries(ecs_history EnTT::EnTT)
