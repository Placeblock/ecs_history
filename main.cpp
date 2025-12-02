#include <chrono>
#include <entt/entt.hpp>

#include "change_mixin.hpp"
#include "change_reactive_mixin.hpp"

template<typename T>
using reactive_storage_t = entt::change_reactive_mixin_t<T, entt::storage<std::shared_ptr<change_t>>, entt::basic_registry<>>;

template<typename T>
using change_storage_t = entt::change_mixin_t<entt::storage<T>, entt::basic_registry<>>;

template<>
struct entt::storage_type<char> {
    /*! @brief Type-to-storage conversion result. */
    using type = change_storage_t<char>;
};

struct base_component_monitor_t {
    virtual void write_to_history(std::vector<std::unique_ptr<change_t>> &changes) = 0;
    virtual ~base_component_monitor_t() = default;
};

template<typename T>
class component_monitor_t final : public base_component_monitor_t {
public:
    explicit component_monitor_t(entt::registry &registry, const entt::id_type id = entt::type_hash<T>::value())
        : id(id), storage(registry.storage<T>(id)), changed_storage(id) {
        changed_storage.bind(registry);
        changed_storage.on_construct().on_update().on_destroy();
    }

    void write_to_history(std::vector<std::unique_ptr<change_t>> &changes) override {
        for (const auto& change : changed_storage) {
            changes.emplace_back(std::make_unique<change_t>(*change));
        }
    }

private:
    const entt::id_type id;
    const entt::storage<T> &storage;
    reactive_storage_t<T> changed_storage;
};

class history_t {
public:
    explicit history_t(entt::registry &registry) : registry(registry) {
    }

    void write_changes() {
        std::vector<std::unique_ptr<change_t>>& change = changes.emplace_back();
        for (const auto& monitor : this->component_monitors) {
            monitor->write_to_history(change);
        }
    }

    template<typename T>
    void monitor_component(const entt::id_type id = entt::type_hash<T>::value()) {
        std::unique_ptr<base_component_monitor_t> monitor = std::make_unique<component_monitor_t<T>>(registry, id);
        this->component_monitors.push_back(std::move(monitor));
    }
private:
    entt::registry &registry;
    std::vector<std::vector<std::unique_ptr<change_t>>> changes;
    std::vector<std::unique_ptr<base_component_monitor_t>> component_monitors;
};

int main() {
    entt::registry registry;
    history_t history{registry};
    history.monitor_component<char>();

    auto& storage = registry.storage<char>();

    for (int i = 0; i < 1000000; ++i) {
        const auto& entt = registry.create();
        storage.emplace(entt, 1);
    }
    history.write_changes();
    /*const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000000; ++i) {
        for (const auto& entt : registry.storage<entt::entity>()) {
            storage.patch(entt, [](char &value) { value++; });
        }
        history.write_changes();
    }
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;*/

    for (int i = 0; i < 10; ++i) {
        for (const auto& entt : registry.storage<entt::entity>()) {
            storage.patch(entt, [](char &value) { value++; });
        }
        history.write_changes();
    }

    return 0;
}