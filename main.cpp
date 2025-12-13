#include <chrono>
#include <iostream>
#include <entt/entt.hpp>

#include "change_applier.hpp"
#include "change_mixin.hpp"
#include "change_reactive_mixin.hpp"

static_entities_t static_entities;

template<typename T>
using reactive_storage_t = entt::change_reactive_mixin_t<T, entt::storage<std::shared_ptr<component_change_t<T>>>, entt::basic_registry<>>;

template<typename T>
using change_storage_t = entt::change_mixin_t<entt::storage<T>, entt::basic_registry<>>;

class base_commit_t {

public:
    [[nodiscard]] virtual size_t size() const = 0;

    [[nodiscard]] virtual std::unique_ptr<base_commit_t> invert() const = 0;

    virtual void apply(entt::registry &reg) const = 0;

    virtual ~base_commit_t() = default;
};

class entity_commit_t final : public base_commit_t {
    std::vector<std::shared_ptr<entity_change_t>> changes;

public:
    [[nodiscard]] size_t size() const override  {
        size_t size = 0;
        for (const auto& change : this->changes) {
            size += change->size();
        }
        return size;
    }

    [[nodiscard]] std::unique_ptr<base_commit_t> invert() const override {
        auto new_commit = std::make_unique<entity_commit_t>();

        for (const auto& change : this->changes) {
            new_commit->changes.emplace_back(change->invert());
        }

        return std::move(new_commit);
    }

    void add_change(const std::shared_ptr<entity_change_t>& change) {
        this->changes.emplace_back(change);
    }

    void apply(entt::registry &reg) const override {
        entity_change_applier_t applier{static_entities, reg};
        for (const auto& change : this->changes) {
            change->apply(applier);
        }
    }
};

class base_component_commit_t : public base_commit_t {
protected:
    entt::id_type id;

public:
    explicit base_component_commit_t(const entt::id_type id) : id(id) {}
};

template<typename T>
class component_commit_t final : public base_component_commit_t {
    std::vector<std::shared_ptr<component_change_t<T>>> changes;

public:
    explicit component_commit_t(const entt::id_type id = entt::type_hash<T>::value())
        : base_component_commit_t(id) {}

    [[nodiscard]] size_t size() const override {
        size_t size = 0;
        for (const auto& change : this->changes) {
            size += change->size();
        }
        return size;
    }

    [[nodiscard]] std::unique_ptr<base_commit_t> invert() const override {
        auto new_commit = std::make_unique<component_commit_t>(this->id);

        for (const auto& change : this->changes) {
            new_commit->changes.emplace_back(change->invert());
        }

        return std::move(new_commit);
    }

    void add_change(const std::shared_ptr<component_change_t<T>>& change) {
        this->changes.emplace_back(change);
    }

    void apply(entt::registry &reg) const override {
        component_change_applier_t<T> applier{static_entities, reg};
        for (const auto& change : this->changes) {
            change->apply(applier);
        }
    }
};

template<>
struct entt::storage_type<char> {
    /*! @brief Type-to-storage conversion result. */
    using type = change_storage_t<char>;
};

struct base_component_monitor {
    virtual std::unique_ptr<base_commit_t> commit() = 0;
    virtual ~base_component_monitor() = default;
};

template<typename T>
class component_monitor_t final : public base_component_monitor {
public:
    explicit component_monitor_t(entt::registry &registry, const entt::id_type id = entt::type_hash<T>::value())
        : id(id), storage(registry.storage<T>(id)), changed_storage(static_entities, id) {
        changed_storage.bind(registry);
        changed_storage.on_construct().on_update().on_destroy();
    }

    std::unique_ptr<base_commit_t> commit() override {
        std::unique_ptr<component_commit_t<T>> commit = std::make_unique<component_commit_t<T>>(this->id);
        for (const auto& change : changed_storage) {
            commit->add_change(change);
        }
        changed_storage.clear();
        return std::move(commit);
    }

private:
    const entt::id_type id;
    const entt::storage<T> &storage;
    reactive_storage_t<T> changed_storage;
};

class entity_monitor_t final {
    using reactive_storage = entt::reactive_mixin<entt::storage<entt::entity>>;
public:
    explicit entity_monitor_t(entt::registry &reg) {
        created_storage.bind(reg);
        destroyed_storage.bind(reg);
    }

    std::unique_ptr<base_commit_t> commit_created() {
        auto commit = std::make_unique<entity_commit_t>();
        for (const auto& entt : created_storage) {
            static_entity_t static_entity = static_entities.get_static_entity(entt);
            commit->add_change(std::make_unique<entity_create_change_t>(static_entity));
        }
        created_storage.clear();
        return std::move(commit);
    }
    std::unique_ptr<base_commit_t> commit_destroyed() {
        auto commit = std::make_unique<entity_commit_t>();
        for (const auto& entt : destroyed_storage) {
            static_entity_t static_entity = static_entities.get_static_entity(entt);
            commit->add_change(std::make_unique<entity_destroy_change_t>(static_entity));
        }
        destroyed_storage.clear();
        return std::move(commit);
    }

    reactive_storage created_storage;
    reactive_storage destroyed_storage;
};

class history_t {
public:
    std::vector<std::vector<std::unique_ptr<base_commit_t>>> commits;

    explicit history_t(entt::registry &registry)
        : registry(registry), entity_monitor(registry) {
    }

    void commit() {
        std::vector<std::unique_ptr<base_commit_t>>& new_commits = commits.emplace_back();
        new_commits.push_back(entity_monitor.commit_created());
        for (const auto& monitor : this->component_monitors) {
            new_commits.push_back(monitor->commit());
        }
        new_commits.push_back(entity_monitor.commit_destroyed());
    }

    template<typename T>
    void monitor_component(const entt::id_type id = entt::type_hash<T>::value()) {
        std::unique_ptr<base_component_monitor> monitor = std::make_unique<component_monitor_t<T>>(registry, id);
        this->component_monitors.push_back(std::move(monitor));
    }
private:
    entt::registry &registry;
    std::vector<std::unique_ptr<base_component_monitor>> component_monitors;
    entity_monitor_t entity_monitor;
};

class history_controller_t {
    void remove_commit() {
        this->history.commits.erase(this->history.commits.begin());
        this->removed_commits++;
    }
public:
    explicit history_controller_t(history_t &history)
        : history(history) {
    }

    [[nodiscard]] size_t commit() const {
        history.commit();
        return history.commits.size() + removed_commits;
    }

private:
    history_t &history;
    size_t removed_commits = 0;
};

int main() {
    entt::registry registry;
    history_t history{registry};
    history.monitor_component<char>();

    auto& storage = registry.storage<char>();

    const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000000; ++i) {
        const auto& entt = registry.create();
        storage.emplace(entt, 1);
    }
    history.commit();
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;
    /*const std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    for (int i = 0; i < 1000000; ++i) {
        for (const auto& entt : registry.storage<entt::entity>()) {
            storage.patch(entt, [](char &value) { value++; });
        }
        history.write_changes();
    }
    const std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count() << "[µs]" << std::endl;*/

    /*
    for (int i = 0; i < 10; ++i) {
        for (const auto& entt : registry.storage<entt::entity>()) {
            storage.patch(entt, [](char &value) { value++; });
        }
        history.commit();
    }*/

    return 0;
}