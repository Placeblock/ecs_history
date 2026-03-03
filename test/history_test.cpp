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

struct bounding_box_t {
    uint8_t value;
};

template<>
struct entt::storage_type<bounding_box_t> {
    /*! @brief Type-to-storage conversion result. */
    using type = change_storage_t<bounding_box_t>;
};

template<typename Archive>
void serialize(Archive &archive, bounding_box_t &box) {
    archive(box.value);
}

int main() {
    spdlog::set_level(spdlog::level::info);

    std::unique_ptr<ecs_history::registry::component_t> component = std::make_unique<
        ecs_history::default_component_t<bounding_box_t> >();
    ecs_history::registry::component_registry_t registry;
    registry.register_component<bounding_box_t>(component);

    entt::registry reg;
    auto entities = ecs_history::static_entities_t{};
    entt::storage_type_t<bounding_box_t> &storage = reg.storage<bounding_box_t>();
    std::unique_ptr<ecs_history::base_storage_monitor_t> monitor = std::make_unique<
        ecs_history::storage_monitor_t<bounding_box_t> >(
        entities,
        storage);
    std::vector<std::unique_ptr<ecs_history::base_storage_monitor_t> > monitors;
    monitors.push_back(std::move(monitor));

    const entt::entity entity = entities.create();
    storage.emplace(entity);
    auto commit = ecs_history::create_commit(monitors, entities);

    entt::registry reg2;
    auto const &entities2 = reg2.ctx().emplace<ecs_history::static_entities_t>();
    std::vector<std::unique_ptr<ecs_history::base_storage_monitor_t> > monitors2;
    const auto history2 = std::make_unique<ecs_history::history_t>(reg2, monitors2);
    history2->apply_commit({0, 0}, {0, 1}, commit);

    const entt::entity entity2 = entities.create();
    storage.emplace(entity2);
    auto commit2 = ecs_history::create_commit(monitors, entities);

    history2->apply_commit({0, 0}, {0, 2}, commit2);

    assert(entities.get_versions().size() == 2);
    assert(entities2.get_versions().size() == 2);

    return 0;
}