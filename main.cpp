#include <chrono>
#include <iostream>
#include <entt/entt.hpp>

using reactive_storage_t = entt::reactive_mixin<entt::storage<void>>;

struct change_t {

};
struct remove_change_t : change_t {

};
template<typename T>
struct data_change_t : change_t {
    T value;

    explicit data_change_t(T value) : value(value) {}
};
struct entity_history_t {
    std::unordered_map<entt::id_type, std::vector<std::unique_ptr<change_t>>> changes{};

    void record_change(const entt::id_type component_id, std::unique_ptr<change_t> change) {
        this->changes[component_id].push_back(std::move(change));
    }
};

struct entities_history_t {
    std::map<entt::entity, entity_history_t> entity_histories{};

    void record_change(const entt::entity entity, const entt::id_type component_id, std::unique_ptr<change_t> change) {
        /*if (!this->entity_histories.contains(entity)) {
            this->entity_histories.emplace(entity, entity_history_t{});
        }*/
        this->entity_histories[entity].record_change(component_id, std::move(change));
    }
};

struct base_component_monitor_t {
    virtual void write_to_history(entities_history_t &history) = 0;
    virtual ~base_component_monitor_t() = default;
};

template<typename T>
class component_monitor_t final : public base_component_monitor_t {
public:
    explicit component_monitor_t(entt::registry &registry, const entt::id_type id = entt::type_hash<T>::value())
        : id(id), storage(registry.storage<T>(id)) {
        changed_storage.bind(registry);
        destroyed_storage.bind(registry);
        changed_storage.template on_construct<T>(id).template on_update<T>(id);
        destroyed_storage.template on_destroy<T>(id);
    }

    void write_to_history(entities_history_t &history) override {
        for (const auto& entt : this->changed_storage) {
            const T &value = this->storage.get(entt);
            std::unique_ptr<change_t> change = std::make_unique<data_change_t<T>>(value);
            history.record_change(entt, this->id, std::move(change));
        }
        for (const auto& entt : this->destroyed_storage) {
            auto change = std::make_unique<remove_change_t>();
            history.record_change(entt, this->id, std::move(change));
        }
        this->changed_storage.clear();
        this->destroyed_storage.clear();
    }

private:
    const entt::id_type id;
    const entt::storage<T> &storage;
    reactive_storage_t changed_storage;
    reactive_storage_t destroyed_storage;
};

class history_t {
public:
    explicit history_t(entt::registry &registry) : registry(registry) {
        this->entity_create_storage.bind(registry);
        this->entity_destroy_storage.bind(registry);
        this->entity_create_storage.on_construct<entt::entity>();
        this->entity_destroy_storage.on_destroy<entt::entity>();
    }

    void write_changes() {
        for (const auto& monitor : this->component_monitors) {
            monitor->write_to_history(this->history_data);
        }
    }

    template<typename T>
    void monitor_component(const entt::id_type id = entt::type_hash<T>::value()) {
        std::unique_ptr<base_component_monitor_t> monitor = std::make_unique<component_monitor_t<T>>(registry, id);
        this->component_monitors.push_back(std::move(monitor));
    }
private:
    entt::registry &registry;
    entities_history_t history_data;;
    reactive_storage_t entity_create_storage;
    reactive_storage_t entity_destroy_storage;
    std::vector<std::unique_ptr<base_component_monitor_t>> component_monitors;
};

int main() {
    entt::registry registry;
    history_t history{registry};
    history.monitor_component<char>();

    auto& storage = registry.storage<char>();

    for (int i = 0; i < 100; ++i) {
        const auto& entt = registry.create();
        storage.emplace(entt, 1);
    }
    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000000; ++i) {
        for (const auto& entt : registry.storage<entt::entity>()) {
            storage.patch(entt, [](char &value) { value++; });
        }
        history.write_changes();
    }
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;

    /*for (int i = 0; i < 10; ++i) {
        for (const auto& entt : registry.storage<entt::entity>()) {
            storage.patch(entt, [](char &value) { value++; });
        }
        history.write_changes();
    }*/

    return 0;
}