include(FetchContent)

cmake_minimum_required(VERSION 3.15)
project(ecs_history)

include(CTest)

set(CMAKE_CXX_STANDARD 20)

find_package(entt QUIET)
if (NOT entt_FOUND)
    FetchContent_Declare(
            entt
            GIT_REPOSITORY https://github.com/skypjack/entt.git
            GIT_TAG v3.16.0
    )
    FetchContent_MakeAvailable(entt)
endif ()

set(JUST_INSTALL_CEREAL ON)
find_package(cereal QUIET)
if (NOT cereal_FOUND)
    FetchContent_Declare(
            cereal
            GIT_REPOSITORY https://github.com/USCiLab/cereal
            GIT_TAG v1.3.2
    )
    FetchContent_MakeAvailable(cereal)
endif ()

find_package(spdlog QUIET)
if (NOT spdlog_FOUND)
    FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.15.3
    )
    FetchContent_MakeAvailable(spdlog)
endif ()

find_package(fmt REQUIRED)

option(ECS_HISTORY_SHARED "Build as shared lib")
if (ECS_HISTORY_SHARED)
    set(ECS_HISTORY_LIBRARY_TYPE SHARED)
else ()
    set(ECS_HISTORY_LIBRARY_TYPE STATIC)
endif ()

add_library(ecs_history ${ECS_HISTORY_LIBRARY_TYPE}
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
        src/static_entity.cpp
        include/ecs_history/history.hpp)
target_include_directories(ecs_history PUBLIC include)
target_link_libraries(ecs_history PUBLIC EnTT::EnTT cereal spdlog::spdlog fmt::fmt)

install(TARGETS ecs_history)

# Tests
add_executable(test_performance test/performance_test.cpp)
target_include_directories(test_performance BEFORE PRIVATE /usr/include)
target_link_libraries(test_performance ecs_history)
add_test(NAME test_performance COMMAND test_performance)