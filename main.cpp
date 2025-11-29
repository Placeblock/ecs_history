#include <entt/entt.hpp>

using reactive_storage_t = entt::reactive_mixin<entt::storage<void>>;

struct base_component_monitor_t {
    virtual void write_to_history(entt::registry &history_registry) = 0;

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

    void write_to_history(entt::registry &history_registry) override {
        for (const auto& entt : this->changed_storage) {
            const T &value = this->storage.get(entt);
            entt::storage<std::vector<size_t>> &history_storage = history_registry.storage<std::vector<size_t>>(this->id);
            this->history_data.push_back(value);
            const size_t data_ref = this->history_data.size() - 1;
            if (history_storage.contains(entt)) {
                history_storage.patch(entt, [&data_ref](std::vector<size_t> &references) {
                    references.push_back(data_ref);
                });
            } else {
                std::vector references = {data_ref};
                std::vector<size_t> &data = history_storage.emplace(entt, references);
                data.shrink_to_fit();
            }
        }
    }

private:
    const entt::id_type id;
    const entt::storage<T> &storage;
    reactive_storage_t changed_storage;
    reactive_storage_t destroyed_storage;
    std::vector<T> history_data;
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
            monitor->write_to_history(this->history_registry);
        }
    }

    template<typename T>
    void monitor_component(const entt::id_type id = entt::type_hash<T>::value()) {
        std::unique_ptr<base_component_monitor_t> monitor = std::make_unique<component_monitor_t<T>>(registry, id);
        this->component_monitors.push_back(std::move(monitor));
    }
private:
    entt::registry &registry;
    entt::registry history_registry;
    reactive_storage_t entity_create_storage;
    reactive_storage_t entity_destroy_storage;
    std::vector<std::unique_ptr<base_component_monitor_t>> component_monitors;
};

int main() {
    entt::registry registry;
    history_t history{registry};
    for (int i = 0; i < 10; ++i) {
        history.monitor_component<char>(i);
    }

    for (int i = 0; i < 1000000; ++i) {
        const auto entity = registry.create();
        for (int j = 0; j < 10; ++j) {
            registry.storage<char>(j).emplace(entity, 1);
        }
    }

    history.write_changes();

    for (int i = 0; i < 100; ++i) {
        for (int j = 0; j < 10; ++j) {
            for (const auto& entt : registry.storage<entt::entity>()) {
                registry.storage<char>(j).patch(entt, [](char &value) { value++; });
            }
        }
        history.write_changes();
    }

    return 0;
}