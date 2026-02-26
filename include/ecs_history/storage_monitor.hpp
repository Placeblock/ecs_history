//
// Created by felix on 17.12.25.
//

#ifndef ECS_HISTORY_MONITOR_HPP
#define ECS_HISTORY_MONITOR_HPP

#include "ecs_history/change.hpp"
#include "ecs_history/static_entity.hpp"
#include "ecs_history/change_set.hpp"

namespace ecs_history {
class base_storage_monitor_t {
public:
    const entt::id_type id;

    explicit base_storage_monitor_t(const entt::id_type id) : id(id) {
    };

    virtual std::unique_ptr<base_change_set_t> commit() = 0;

    virtual void disable() = 0;

    virtual void enable() = 0;

    virtual void clear() = 0;

    virtual ~base_storage_monitor_t() = default;
};

template<typename T>
class storage_monitor_t final : public base_storage_monitor_t {

public:
    explicit storage_monitor_t(static_entities_t &entities,
                               entt::storage_type_t<T> &storage)
        : base_storage_monitor_t(storage.info().hash()),
          entities(entities),
          storage(storage) {
        this->enable();
    }

    std::unique_ptr<base_change_set_t> commit() override {
        std::unique_ptr<base_change_set_t> change_set = std::make_unique<change_set_t<T> >(
            this->changes,
            this->id);
        return change_set;
    }

    void clear() override {
        this->changes.clear();
    }

    void enable() override {
        this->storage.on_construct().template connect<&storage_monitor_t::on_construct>(this);
        this->storage.on_update().template connect<&storage_monitor_t::on_update>(this);
        this->storage.on_destroy().template connect<&storage_monitor_t::on_destruct>(this);
    }

    void on_construct(const entt::entity entity,
                      const T &value) {
        static_entity_t static_entity = this->entities.get_static_entity(entity);
        this->changes.emplace_back(
            std::make_unique<construct_change_t<T> >(static_entity, value));
    }

    void on_destruct(const entt::entity entity,
                     const T &old_value) {
        static_entity_t static_entity = this->entities.get_static_entity(entity);
        this->changes.emplace_back(
            std::make_unique<destruct_change_t<T> >(static_entity, old_value));
    }

    void on_update(const entt::entity entity,
                   const T &old_value,
                   const T &new_value) {
        static_entity_t static_entity = this->entities.get_static_entity(entity);
        this->changes.emplace_back(std::make_unique<update_change_t<T> >(
            static_entity,
            old_value,
            new_value));
    }

    void disable() override {
        this->storage.on_construct().template disconnect<&storage_monitor_t::on_construct>(this);
        this->storage.on_update().template disconnect<&storage_monitor_t::on_update>(this);
        this->storage.on_destroy().template disconnect<&storage_monitor_t::on_destruct>(this);
    }

private:
    static_entities_t &entities;
    entt::storage_type_t<T> &storage;
    std::vector<std::unique_ptr<change_t<T> > > changes;
};
}

#endif //ECS_HISTORY_MONITOR_HPP
