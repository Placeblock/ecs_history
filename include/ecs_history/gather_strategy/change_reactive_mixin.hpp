//
// Created by felix on 11/29/25.
//

#ifndef ECS_HISTORY_CHANGE_REACTIVE_MIXIN_H
#define ECS_HISTORY_CHANGE_REACTIVE_MIXIN_H
#include <memory>
#include <entt/entity/registry.hpp>


namespace entt {
    struct component_change_t {
    };

    template<typename T>
    struct construct_component_change_t : component_change_t {
        using component_type = T;

        T value;

        explicit construct_component_change_t(const T value) : value(value) {
        }
    };

    template<typename T>
    struct update_component_change_t : component_change_t {
        using component_type = T;

        T old_value;
        T new_value;

        explicit update_component_change_t(const T old_value, const T new_value) : old_value(old_value),
            new_value(new_value) {
        }
    };

    template<typename T>
    struct destruct_component_change_t : component_change_t {
        using component_type = T;

        T old_value;

        explicit destruct_component_change_t(const T old_value) : old_value(old_value) {
        }
    };

    /**
 * @brief Mixin type used to add _reactive_ support to storage types.
 * @tparam Type Underlying storage type.
 * @tparam Registry Basic registry type.
 */
    template<typename Type, typename Registry>
    class change_reactive_mixin_t final : public Type {
        using underlying_type = Type;
        using owner_type = Registry;

        using alloc_traits = std::allocator_traits<typename underlying_type::allocator_type>;
        using basic_registry_type = basic_registry<typename owner_type::entity_type, typename
            owner_type::allocator_type>;
        using container_type = std::vector<connection, typename alloc_traits::template rebind_alloc<connection> >;

        using change_type = Type::value_type;
        using value_type = Type::value_type::component_type;

        static_assert(std::is_base_of_v<basic_registry_type, owner_type>, "Invalid registry type");
        static_assert(std::is_base_of_v<component_change_t, change_type>, "Invalid change type");

        [[nodiscard]] auto &owner_or_assert() const noexcept {
            ENTT_ASSERT(owner != nullptr, "Invalid pointer to registry");
            return static_cast<owner_type &>(*owner);
        }


        void emplace_creation(const Registry &, underlying_type::entity_type entity, const value_type &value)
            requires std::same_as<change_type, construct_component_change_t<value_type> > {
            underlying_type::emplace(entity, construct_component_change_t<value_type>{value});
        }

        void emplace_update(const Registry &, underlying_type::entity_type entity, const value_type &old_value,
                            const value_type &new_value)
            requires std::same_as<change_type, update_component_change_t<value_type> > {
            underlying_type::emplace(entity, update_component_change_t<value_type>{old_value, new_value});
        }

        void emplace_destruction(const Registry &, underlying_type::entity_type entity, const value_type &old_value)
            requires std::same_as<change_type, destruct_component_change_t<value_type> > {
            underlying_type::emplace(entity, destruct_component_change_t<value_type>{old_value});
        }

        void bind_any(any value) noexcept {
            owner = any_cast<basic_registry_type>(&value);

            if constexpr (!std::is_same_v<registry_type, basic_registry_type>) {
                if (owner == nullptr) {
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
        explicit change_reactive_mixin_t(const id_type id = type_hash<Type>::value())
            : change_reactive_mixin_t{allocator_type{}, id} {
        }

        /**
         * @brief Constructs an empty storage with a given allocator.
         * @param allocator The allocator to use.
         * @param id Optional name used to map the storage within the registry.
         */
        explicit change_reactive_mixin_t(const allocator_type &allocator, const id_type id = type_hash<Type>::value())
            : underlying_type{allocator},
              owner{},
              id(id) {
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
              id{other.id} {
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
              id{other.id} {
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

        void connect() {
            this->disconnect();
            if constexpr (std::same_as<change_type, construct_component_change_t<value_type> >) {
                this->conn = owner_or_assert().template storage<value_type>(id).on_construct().template connect<&
                    change_reactive_mixin_t::emplace_creation>(*this);
            } else if constexpr (std::same_as<change_type, update_component_change_t<value_type> >) {
                this->conn = owner_or_assert().template storage<value_type>(id).on_update().template connect<&
                    change_reactive_mixin_t::emplace_update>(*this);
            } else if constexpr (std::same_as<change_type, destruct_component_change_t<value_type> >) {
                this->conn = owner_or_assert().template storage<value_type>(id).on_destroy().template connect<&
                    change_reactive_mixin_t::emplace_destruction>(*this);
            }
        }

        void disconnect() {
            this->reset();
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
        [[nodiscard]] basic_view<get_t<const change_reactive_mixin_t, typename basic_registry_type::template
            storage_for_type<const Get>...>, exclude_t<typename basic_registry_type::template storage_for_type<const
            Exclude>...> >
        view(exclude_t<Exclude...> = exclude_t{}) const {
            const owner_type &parent = owner_or_assert();
            basic_view<get_t<const change_reactive_mixin_t, typename basic_registry_type::template storage_for_type<
                        const Get>...>, exclude_t<typename basic_registry_type::template storage_for_type<const Exclude>
                        ...> >
                    elem{};
            [&elem](const auto *... curr) { ((curr ? elem.storage(*curr) : void()), ...); }(
                parent.template storage<std::remove_const_t<Exclude> >()...,
                parent.template storage<std::remove_const_t<Get> >()..., this);
            return elem;
        }

        /*! @copydoc view */
        template<typename... Get, typename... Exclude>
        [[nodiscard]] basic_view<get_t<const change_reactive_mixin_t, typename basic_registry_type::template
            storage_for_type<Get>...>, exclude_t<typename basic_registry_type::template storage_for_type<Exclude>...> >
        view(exclude_t<Exclude...> = exclude_t{}) {
            std::conditional_t<((std::is_const_v<Get> && ...) && (std::is_const_v<Exclude> && ...)), const owner_type,
                owner_type> &parent = owner_or_assert();
            return {
                *this, parent.template storage<std::remove_const_t<Get> >()...,
                parent.template storage<std::remove_const_t<Exclude> >()...
            };
        }

        /*! @brief Releases all connections to the underlying registry, if any. */
        void reset() {
            if (this->conn) {
                this->conn.release();
            }
        }

    private:
        basic_registry_type *owner;
        connection conn;
        id_type id;
    };
}

#endif //ECS_HISTORY_CHANGE_REACTIVE_MIXIN_H
