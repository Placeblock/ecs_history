#include <iostream>

#include <entt/entt.hpp>

using reactive_storage_t = entt::reactive_mixin<entt::storage<void>>;

struct component_state_t {};
struct component_state_destroyed_t : component_state_t {};
template<typename T>
struct component_state_value_t : component_state_t {
    T value;

    explicit component_state_value_t(const T& value) : value(value) {}
};

class base_history_entity_component {
public:
    virtual ~base_history_entity_component() = default;
};

template<typename T>
class history_entity_component final : public base_history_entity_component{
    std::vector<std::unique_ptr<component_state_t>> values;
    size_t shift = 0;

public:
    component_state_t* get_value(const size_t index) {
        if (index < this->shift) {
            throw std::out_of_range("history element does not exist anymore");
        }
        if (index > this->shift + this->values.size()) {
            throw std::out_of_range("history index out of range");
        }
        return this->values[index - this->shift].get();
    }

    size_t write_value(T &value) {
        this->values.push_back(std::make_unique<component_state_value_t<T>>(value));
        return this->shift + this->values.size() - 1;
    }

    size_t write_destroyed() {
        this->values.push_back(std::make_unique<component_state_destroyed_t>());
        return this->shift + this->values.size() - 1;
    }

    ~history_entity_component() override = default;
};

class history_entity_t {
    std::map<entt::id_type, std::shared_ptr<base_history_entity_component>> components;

public:
    template<typename T>
    T &get_component_value(const size_t index, const entt::id_type id = entt::type_hash<T>::value()) {
        if (!this->components.contains(id)) {
            this->components.emplace(id, std::make_shared<history_entity_component<T>>());
        }
        return static_cast<history_entity_component<T>*>(this->components[id].get())->get_value(index);
    }

    template<typename T>
    size_t write_component_value(T value, const entt::id_type id = entt::type_hash<T>::value()) {
        if (!this->components.contains(id)) {
            this->components.emplace(id, std::make_shared<history_entity_component<T>>());
        }
        return static_cast<history_entity_component<T>*>(this->components[id].get())->write_value(value);
    }

    template<typename T>
    size_t write_component_destroyed(const entt::id_type id = entt::type_hash<T>::value()) {
        if (!this->components.contains(id)) {
            this->components.emplace(id, std::make_shared<history_entity_component<T>>());
        }
        return static_cast<history_entity_component<T>*>(this->components[id].get())->write_destroyed();
    }
};

struct base_component_monitor_t {
    virtual void write_to_history(entt::storage<history_entity_t>& history_entities) = 0;

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

    void write_to_history(entt::storage<history_entity_t>& history_entities) override {
        for (const auto& entity : this->changed_storage) {
            history_entity_t &history_entity = history_entities.get(entity);
            history_entity.write_component_value<T>(this->storage.get(entity), this->id);
        }
        for (const auto& entity : this->destroyed_storage) {
            history_entity_t &history_entity = history_entities.get(entity);
            history_entity.write_component_destroyed<T>(this->id);
        }
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
        for (const auto& entity : this->entity_create_storage) {
            if (!this->history_entities.contains(entity)) {
                this->history_entities.emplace(entity);
            }
        }
        for (const auto& monitor : this->component_monitors) {
            monitor->write_to_history(this->history_entities);
        }
    }

    template<typename T>
    void monitor_component(const entt::id_type id = entt::type_hash<T>::value()) {
        std::unique_ptr<base_component_monitor_t> monitor = std::make_unique<component_monitor_t<T>>(registry, id);
        this->component_monitors.push_back(std::move(monitor));
    }
private:
    entt::registry &registry;
    entt::storage<history_entity_t> history_entities;
    reactive_storage_t entity_create_storage;
    reactive_storage_t entity_destroy_storage;
    std::vector<std::unique_ptr<base_component_monitor_t>> component_monitors;
};

int main() {
    entt::registry registry;
    //history_t history{registry};
    //history.monitor_component<char>();
    //history.monitor_component<uint8_t>();

    for (int i = 0; i < 1000000; ++i) {
        const auto entity = registry.create();
        registry.emplace<char>(entity, 2);
        registry.emplace<uint8_t>(entity, 2);
    }

    //history.write_changes();

    return 0;
}