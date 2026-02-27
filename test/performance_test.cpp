//
// Created by felix on 1/30/26.
//

#include "ecs_history/serialization/serialization.hpp"
#include "ecs_history/history.hpp"
#include "ecs_history/component/default_component.hpp"
#include "ecs_history/entt/change_mixin.hpp"

#include <spdlog/stopwatch.h>

using namespace entt::literals;
using std::chrono::duration_cast;
using std::chrono::milliseconds;

struct vector_t {
    float x, y, z, w;
};

struct bounding_box_t {
    vector_t pos_size;

    bounding_box_t() : pos_size({0, 0, 0, 0}) {

    }
};

template<>
struct entt::storage_type<bounding_box_t> {
    /*! @brief Type-to-storage conversion result. */
    using type = change_storage_t<bounding_box_t>;
};

template<typename Archive>
void serialize(Archive &archive, bounding_box_t &box) {
    archive(box.pos_size.x, box.pos_size.y, box.pos_size.z, box.pos_size.w);
}

int main() {
    spdlog::set_level(spdlog::level::info);

    std::unique_ptr<ecs_history::context::component_t> component = std::make_unique<
        ecs_history::default_component_t<bounding_box_t> >();
    ecs_history::context::register_component<bounding_box_t>(component);

    int amount = 1000000;

    entt::registry reg;
    auto entities = ecs_history::static_entities_t{};
    auto version_handler = ecs_history::entity_version_handler_t{};
    entt::storage_type_t<bounding_box_t> &storage = reg.storage<bounding_box_t>();
    std::unique_ptr<ecs_history::base_storage_monitor_t> monitor = std::make_unique<
        ecs_history::storage_monitor_t<bounding_box_t> >(
        entities,
        storage);
    std::vector<std::unique_ptr<ecs_history::base_storage_monitor_t> > monitors;
    monitors.push_back(std::move(monitor));
    auto history = std::make_unique<ecs_history::history_t>(reg, monitors);

    entt::registry reg2;
    reg2.ctx().emplace<ecs_history::static_entities_t>();
    reg2.ctx().emplace<ecs_history::entity_version_handler_t>();

    const spdlog::stopwatch create_entities_sw;
    for (int i = 0; i < amount; ++i) {
        ecs_history::static_entity_t static_entity = entities.create();
        version_handler.add_entity(static_entity, 0);
    }
    spdlog::info("Creating 1.000.000 Entities: {}",
                 duration_cast<milliseconds>(create_entities_sw.elapsed()));

    const spdlog::stopwatch emplace_components_sw;
    for (uint32_t i = 0; i < amount; ++i) {
        storage.emplace(entt::entity{i});
    }
    spdlog::info("Adding Components to 1.000.000 Entities: {}",
                 duration_cast<milliseconds>(emplace_components_sw.elapsed()));

    const spdlog::stopwatch create_commit_sw;
    auto commit = ecs_history::create_commit(monitors, version_handler);
    spdlog::info("Creating commit of 1.000.000 Entities with 1 created component each: {}",
                 duration_cast<milliseconds>(create_commit_sw.elapsed()));

    const spdlog::stopwatch serialize_commit_sw;
    std::stringstream oss{};
    cereal::PortableBinaryOutputArchive archive(oss);
    ecs_history::serialization::serialize_commit(archive, *commit);
    auto commit_string = oss.str();
    spdlog::info("Serializing commit of 1.000.000 Entities with 1 component created each: {}",
                 duration_cast<milliseconds>(serialize_commit_sw.elapsed()));

    const spdlog::stopwatch deserialize_commit_sw;
    std::istringstream commit_is(commit_string);
    cereal::PortableBinaryInputArchive commit_archive(commit_is);
    auto deserialized_commit = ecs_history::serialization::deserialize_commit(commit_archive);
    spdlog::info("Deserializing commit of 1.000.000 Entities with 1 component created each: {}",
                 duration_cast<milliseconds>(deserialize_commit_sw.elapsed()));

    const spdlog::stopwatch apply_commit_sw;
    ecs_history::apply_commit(reg2, monitors, *deserialized_commit);
    spdlog::info("Applying commit of 1.000.000 Entities with 1 component created each: {}",
                 duration_cast<milliseconds>(apply_commit_sw.elapsed()));

    const spdlog::stopwatch replace_components_sw;
    for (uint32_t i = 0; i < amount; ++i) {
        reg.replace<bounding_box_t>(entt::entity{i});
    }
    spdlog::info("Replacing Components on 1.000.000 Entities: {}",
                 duration_cast<milliseconds>(replace_components_sw.elapsed()));

    const spdlog::stopwatch create_replace_commit_sw;
    auto replace_commit = ecs_history::create_commit(monitors, version_handler);
    spdlog::info("Creating commit of 1.000.000 Entities with 1 component replaced each: {}",
                 duration_cast<milliseconds>(create_replace_commit_sw.elapsed()));

    const spdlog::stopwatch serialize_replace_commit_sw;
    std::stringstream replace_oss{};
    cereal::PortableBinaryOutputArchive replace_archive(replace_oss);
    ecs_history::serialization::serialize_commit(replace_archive, *replace_commit);
    auto replace_commit_string = replace_oss.str();
    spdlog::info("Serializing commit of 1.000.000 Entities with 1 component replaced each: {}",
                 duration_cast<milliseconds>(serialize_replace_commit_sw.elapsed()));

    const spdlog::stopwatch deserialize_replace_commit_sw;
    std::istringstream replace_commit_is(replace_commit_string);
    cereal::PortableBinaryInputArchive replace_commit_archive(replace_commit_is);
    auto deserialized_replace_commit = ecs_history::serialization::deserialize_commit(
        replace_commit_archive);
    spdlog::info("Deserializing commit of 1.000.000 Entities with 1 component replaced each: {}",
                 duration_cast<milliseconds>(deserialize_replace_commit_sw.elapsed()));

    const spdlog::stopwatch apply_replace_commit_sw;
    ecs_history::apply_commit(reg2, monitors, *deserialized_replace_commit);
    spdlog::info("Applying commit of 1.000.000 Entities with 1 component replaced each: {}",
                 duration_cast<milliseconds>(apply_replace_commit_sw.elapsed()));

    const spdlog::stopwatch delete_components_sw;
    for (uint32_t i = 0; i < amount; ++i) {
        reg.remove<bounding_box_t>(entt::entity{i});
    }
    spdlog::info("Removing 1 component on 1.000.000 Entities: {}",
                 duration_cast<milliseconds>(delete_components_sw.elapsed()));

    const spdlog::stopwatch create_delete_commit_sw;
    auto delete_commit = ecs_history::create_commit(monitors, version_handler);
    spdlog::info("Creating commit of 1.000.000 Entities with 1 component removed each: {}",
                 duration_cast<milliseconds>(create_delete_commit_sw.elapsed()));

    const spdlog::stopwatch serialize_delete_commit_sw;
    std::stringstream delete_oss{};
    cereal::PortableBinaryOutputArchive delete_archive(delete_oss);
    ecs_history::serialization::serialize_commit(delete_archive, *delete_commit);
    auto delete_commit_string = delete_oss.str();
    spdlog::info("Serializing commit of 1.000.000 Entities with 1 component removed each: {}",
                 duration_cast<milliseconds>(serialize_delete_commit_sw.elapsed()));

    const spdlog::stopwatch deserialize_delete_commit_sw;
    std::istringstream delete_commit_is(delete_commit_string);
    cereal::PortableBinaryInputArchive delete_commit_archive(delete_commit_is);
    auto deserialized_delete_commit = ecs_history::serialization::deserialize_commit(
        delete_commit_archive);
    spdlog::info("Deserializing commit of 1.000.000 Entities with 1 component removed each: {}",
                 duration_cast<milliseconds>(deserialize_delete_commit_sw.elapsed()));

    const spdlog::stopwatch apply_delete_commit_sw;
    ecs_history::apply_commit(reg2, monitors, *deserialized_delete_commit);
    spdlog::info("Applying commit of 1.000.000 Entities with 1 component removed each: {}",
                 duration_cast<milliseconds>(apply_delete_commit_sw.elapsed()));

    return 0;
}