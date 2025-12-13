//
// Created by felix on 11/29/25.
//

#ifndef ECS_HISTORY_CHANGE_REACTIVE_MIXIN_H
#define ECS_HISTORY_CHANGE_REACTIVE_MIXIN_H
#include <memory>
#include <entt/entity/registry.hpp>
#include "static_entity.hpp"

namespace entt {
    /**
 * @brief Mixin type used to add _reactive_ support to storage types.
 * @tparam Type Underlying storage type.
 * @tparam Registry Basic registry type.
 */
template<typename Clazz, typename Type, typename Registry>
class change_reactive_mixin_t final: public Type {
    using underlying_type = Type;
    using owner_type = Registry;

    using alloc_traits = std::allocator_traits<typename underlying_type::allocator_type>;
    using basic_registry_type = basic_registry<typename owner_type::entity_type, typename owner_type::allocator_type>;
    using container_type = std::vector<connection, typename alloc_traits::template rebind_alloc<connection>>;

    static_assert(std::is_base_of_v<basic_registry_type, owner_type>, "Invalid registry type");

    [[nodiscard]] auto &owner_or_assert() const noexcept {
        ENTT_ASSERT(owner != nullptr, "Invalid pointer to registry");
        return static_cast<owner_type &>(*owner);
    }

    void emplace_construction(const Registry &, underlying_type::entity_type entity, Clazz value) {
        uint64_t static_entt = this->static_entities.get_static_entity(entity);
        if(!underlying_type::contains(entity)) {
            underlying_type::emplace(entity, std::make_shared<construct_change_t<Clazz>>(static_entt, value));
        }
    }
    void emplace_destruction(const Registry &, underlying_type::entity_type entity, Clazz old_value) {
        uint64_t static_entt = this->static_entities.get_static_entity(entity);
        if(!underlying_type::contains(entity)) {
            underlying_type::emplace(entity, std::make_shared<destruct_change_t<Clazz>>(static_entt, old_value));
        }
    }
    void emplace_update(const Registry &, underlying_type::entity_type entity, Clazz old_value, Clazz new_value) {
        uint64_t static_entt = this->static_entities.get_static_entity(entity);
        if(!underlying_type::contains(entity)) {
            underlying_type::emplace(entity, std::make_shared<update_change_t<Clazz>>(static_entt, old_value, new_value));
        }
    }

    void bind_any(any value) noexcept {
        owner = any_cast<basic_registry_type>(&value);

        if constexpr(!std::is_same_v<registry_type, basic_registry_type>) {
            if(owner == nullptr) {
                owner = any_cast<registry_type>(&value);
            }
        }

        underlying_type::bind_any(std::move(value));
    }

public:
    /*! @brief Allocator type. */
    using allocator_type = underlying_type::allocator_type;
    /*! @brief Underlying entity identifier. */
    using entity_type = underlying_type::entity_type;
    /*! @brief Expected registry type. */
    using registry_type = owner_type;

    /*! @brief Default constructor. */
    explicit change_reactive_mixin_t(static_entities_t& static_entities, const id_type id = type_hash<Type>::value())
        : change_reactive_mixin_t{static_entities, allocator_type{}, id} {}

    /**
     * @brief Constructs an empty storage with a given allocator.
     * @param allocator The allocator to use.
     * @param id Optional name used to map the storage within the registry.
     */
    explicit change_reactive_mixin_t(static_entities_t& static_entities, const allocator_type &allocator, const id_type id = type_hash<Type>::value())
        : underlying_type{allocator},
          owner{},
          conn{allocator},
          id(id),
          static_entities(static_entities) {
    }

    /*! @brief Default copy constructor, deleted on purpose. */
    change_reactive_mixin_t(const change_reactive_mixin_t &) = delete;

    /**
     * @brief Move constructor.
     * @param other The instance to move from.
     */
    // NOLINTBEGIN(bugprone-use-after-move)
    change_reactive_mixin_t(change_reactive_mixin_t &&other) noexcept
        : underlying_type{std::move(other)},
          owner{other.owner},
          conn{},
          id{other.id},
          static_entities(other.static_entities) {
    }
    // NOLINTEND(bugprone-use-after-move)

    /**
     * @brief Allocator-extended move constructor.
     * @param other The instance to move from.
     * @param allocator The allocator to use.
     */
    // NOLINTBEGIN(bugprone-use-after-move)
    change_reactive_mixin_t(change_reactive_mixin_t &&other, const allocator_type &allocator)
        : underlying_type{std::move(other), allocator},
          owner{other.owner},
          conn{allocator},
          id{other.id},
          static_entities(other.static_entities) {
    }
    // NOLINTEND(bugprone-use-after-move)

    /*! @brief Default destructor. */
    ~change_reactive_mixin_t() override = default;

    /**
     * @brief Default copy assignment operator, deleted on purpose.
     * @return This mixin.
     */
    change_reactive_mixin_t &operator=(const change_reactive_mixin_t &) = delete;

    /**
     * @brief Move assignment operator.
     * @param other The instance to move from.
     * @return This mixin.
     */
    change_reactive_mixin_t &operator=(change_reactive_mixin_t &&other) noexcept {
        underlying_type::swap(other);
        return *this;
    }

    /**
     * @brief Makes storage _react_ to creation of objects of the given type.
     * @tparam Candidate Function to use to _react_ to the event.
     * @return This mixin.
     */
    template<auto Candidate = &change_reactive_mixin_t::emplace_construction>
    change_reactive_mixin_t &on_construct() {
        auto curr = owner_or_assert().template storage<Clazz>(id).on_construct().template connect<Candidate>(*this);
        conn.push_back(std::move(curr));
        return *this;
    }

    /**
     * @brief Makes storage _react_ to update of objects of the given type.
     * @tparam Candidate Function to use to _react_ to the event.
     * @return This mixin.
     */
    template<auto Candidate = &change_reactive_mixin_t::emplace_update>
    change_reactive_mixin_t &on_update() {
        auto curr = owner_or_assert().template storage<Clazz>(id).on_update().template connect<Candidate>(*this);
        conn.push_back(std::move(curr));
        return *this;
    }

    /**
     * @brief Makes storage _react_ to destruction of objects of the given type.
     * @tparam Candidate Function to use to _react_ to the event.
     * @return This mixin.
     */
    template<auto Candidate = &change_reactive_mixin_t::emplace_destruction>
    change_reactive_mixin_t &on_destroy() {
        auto curr = owner_or_assert().template storage<Clazz>(id).on_destroy().template connect<Candidate>(*this);
        conn.push_back(std::move(curr));
        return *this;
    }

    /**
     * @brief Checks if a mixin refers to a valid registry.
     * @return True if the mixin refers to a valid registry, false otherwise.
     */
    [[nodiscard]] explicit operator bool() const noexcept {
        return (owner != nullptr);
    }

    /**
     * @brief Returns a pointer to the underlying registry, if any.
     * @return A pointer to the underlying registry, if any.
     */
    [[nodiscard]] const registry_type &registry() const noexcept {
        return owner_or_assert();
    }

    /*! @copydoc registry */
    [[nodiscard]] registry_type &registry() noexcept {
        return owner_or_assert();
    }

    /**
     * @brief Returns a view that is filtered by the underlying storage.
     * @tparam Get Types of elements used to construct the view.
     * @tparam Exclude Types of elements used to filter the view.
     * @return A newly created view.
     */
    template<typename... Get, typename... Exclude>
    [[nodiscard]] basic_view<get_t<const change_reactive_mixin_t, typename basic_registry_type::template storage_for_type<const Get>...>, exclude_t<typename basic_registry_type::template storage_for_type<const Exclude>...>>
    view(exclude_t<Exclude...> = exclude_t{}) const {
        const owner_type &parent = owner_or_assert();
        basic_view<get_t<const change_reactive_mixin_t, typename basic_registry_type::template storage_for_type<const Get>...>, exclude_t<typename basic_registry_type::template storage_for_type<const Exclude>...>> elem{};
        [&elem](const auto *...curr) { ((curr ? elem.storage(*curr) : void()), ...); }(parent.template storage<std::remove_const_t<Exclude>>()..., parent.template storage<std::remove_const_t<Get>>()..., this);
        return elem;
    }

    /*! @copydoc view */
    template<typename... Get, typename... Exclude>
    [[nodiscard]] basic_view<get_t<const change_reactive_mixin_t, typename basic_registry_type::template storage_for_type<Get>...>, exclude_t<typename basic_registry_type::template storage_for_type<Exclude>...>>
    view(exclude_t<Exclude...> = exclude_t{}) {
        std::conditional_t<((std::is_const_v<Get> && ...) && (std::is_const_v<Exclude> && ...)), const owner_type, owner_type> &parent = owner_or_assert();
        return {*this, parent.template storage<std::remove_const_t<Get>>()..., parent.template storage<std::remove_const_t<Exclude>>()...};
    }

    /*! @brief Releases all connections to the underlying registry, if any. */
    void reset() {
        for(auto &&curr: conn) {
            curr.release();
        }

        conn.clear();
    }

private:
    basic_registry_type *owner;
    container_type conn;
    id_type id;
    static_entities_t& static_entities;
};
}

#endif //ECS_HISTORY_CHANGE_REACTIVE_MIXIN_H