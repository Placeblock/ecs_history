//
// Created by felix on 1/30/26.
//

#include "ecs_history/gather_strategy/reactive/change_mixin.hpp"
#include "ecs_history/gather_strategy/reactive/reactive_gather_strategy.hpp"
#include "ecs_history/serialization/component.hpp"
#include "ecs_history/serialization/serialization.hpp"
#include "ecs_history/gather_strategy/gather_strategy.hpp"
#include "ecs_history/history.hpp"

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

template<>
struct fmt::formatter<bounding_box_t> : formatter<std::string> {
    auto format(bounding_box_t box,
                format_context &ctx) const -> decltype(ctx.out()) {
        return format_to(ctx.out(), "[{}, {}]", box.pos_size.x, box.pos_size.y);
    }
};

template<typename T>
void emplace(entt::registry &registry, const entt::entity entt, const T &value) {
    registry.emplace_or_replace<T>(entt, value);
}

int main() {
    ecs_history::serialization::initialize_component_meta_types();
    entt::meta_factory<bounding_box_t>{}
        .func<ecs_history::serialization::deserialize_change_set<
            cereal::PortableBinaryInputArchive, bounding_box_t> >("deserialize_change_set"_hs)
        .func<emplace<bounding_box_t> >("emplace"_hs)
        .data<&bounding_box_t::pos_size, entt::as_ref_t>("pos_size"_hs);
    entt::meta_factory<vector_t>{}
        .data<&vector_t::x, entt::as_ref_t>("x"_hs)
        .data<&vector_t::y, entt::as_ref_t>("y"_hs)
        .data<&vector_t::z, entt::as_ref_t>("z"_hs)
        .data<&vector_t::w, entt::as_ref_t>("w"_hs);

    entt::registry reg;
    auto &entities = reg.ctx().emplace<ecs_history::static_entities_t>();
    auto &version_handler = reg.ctx().emplace<ecs_history::entity_version_handler_t>();
    auto gather_strategy = std::make_shared<ecs_history::reactive_gather_strategy>(reg);
    gather_strategy->record_changes<entt::entity>();
    gather_strategy->record_changes<bounding_box_t>();
    reg.ctx().emplace<std::shared_ptr<ecs_history::gather_strategy_t> >(gather_strategy);
    auto history = std::make_unique<ecs_history::history_t>(reg);

    entt::registry reg2;
    reg2.ctx().emplace<ecs_history::static_entities_t>();
    reg2.ctx().emplace<ecs_history::entity_version_handler_t>();
    auto gather_strategy2 = std::make_shared<ecs_history::reactive_gather_strategy>(reg2);

    const spdlog::stopwatch create_entities_sw;
    for (int i = 0; i < 1000000; ++i) {
        const auto entt = reg.create();
        const ecs_history::static_entity_t static_entity = entities.create(entt);
        version_handler.add_entity(static_entity, 0);
    }
    spdlog::info("Creating 1.000.000 Entities: {}",
                 duration_cast<milliseconds>(create_entities_sw.elapsed()));

    const spdlog::stopwatch emplace_components_sw;
    for (uint32_t i = 0; i < 1000000; ++i) {
        reg.emplace<bounding_box_t>(entt::entity{i});
    }
    spdlog::info("Adding Components to 1.000.000 Entities: {}",
                 duration_cast<milliseconds>(emplace_components_sw.elapsed()));

    const spdlog::stopwatch create_commit_sw;
    auto commit = ecs_history::create_commit(*gather_strategy, version_handler);
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
    ecs_history::apply_commit(reg2, *gather_strategy2, *deserialized_commit);
    spdlog::info("Applying commit of 1.000.000 Entities with 1 component created each: {}",
                 duration_cast<milliseconds>(apply_commit_sw.elapsed()));

    const spdlog::stopwatch replace_components_sw;
    for (uint32_t i = 0; i < 1000000; ++i) {
        reg.replace<bounding_box_t>(entt::entity{i});
    }
    spdlog::info("Replacing Components on 1.000.000 Entities: {}",
                 duration_cast<milliseconds>(replace_components_sw.elapsed()));

    const spdlog::stopwatch create_replace_commit_sw;
    auto replace_commit = ecs_history::create_commit(*gather_strategy, version_handler);
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
    ecs_history::apply_commit(reg2, *gather_strategy2, *deserialized_replace_commit);
    spdlog::info("Applying commit of 1.000.000 Entities with 1 component replaced each: {}",
                 duration_cast<milliseconds>(apply_replace_commit_sw.elapsed()));
    return 0;
}