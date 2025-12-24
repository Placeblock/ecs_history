//
// Created by felix on 11/29/25.
//

#ifndef ECS_HISTORY_CHANGE_MIXIN_HPP
#define ECS_HISTORY_CHANGE_MIXIN_HPP
#include <type_traits>
#include <entt/entity/registry.hpp>
#include <entt/signal/sigh.hpp>

namespace entt {
template<typename Type, typename Registry>
class change_mixin_t : public Type {
    using underlying_type = Type;
    using owner_type = Registry;

    using basic_registry_type = basic_registry<typename owner_type::entity_type, typename
                                               owner_type::allocator_type>;
    using construction_type = sigh<void(owner_type &,
                                        typename underlying_type::entity_type,
                                        const typename underlying_type::value_type &value), typename
                                   underlying_type::allocator_type>;
    using update_type = sigh<void(owner_type &,
                                  typename underlying_type::entity_type,
                                  const typename underlying_type::value_type &old_value,
                                  const typename underlying_type::value_type &new_value), typename
                             underlying_type::allocator_type>;
    using destruction_type = sigh<void(owner_type &,
                                       typename underlying_type::entity_type,
                                       const typename underlying_type::value_type &old_value),
                                  typename
                                  underlying_type::allocator_type>;
    using underlying_iterator = underlying_type::base_type::basic_iterator;

    static_assert(std::is_base_of_v<basic_registry_type, owner_type>, "Invalid registry type");

    [[nodiscard]] auto &owner_or_assert() const noexcept {
        ENTT_ASSERT(owner != nullptr, "Invalid pointer to registry");
        return static_cast<owner_type &>(*owner);
    }

private:
    void pop(underlying_iterator first, underlying_iterator last) final {
        if (auto &reg = owner_or_assert(); destruction.empty()) {
            underlying_type::pop(first, last);
        } else {
            for (; first != last; ++first) {
                const auto entt = *first;
                const auto &old_value = underlying_type::get(entt);
                destruction.publish(reg, entt, old_value);
                const auto it = underlying_type::find(entt);
                underlying_type::pop(it, it + 1u);
            }
        }
    }

    void pop_all() final {
        if (auto &reg = owner_or_assert(); !destruction.empty()) {
            if constexpr (std::is_same_v<typename underlying_type::element_type, entity_type>) {
                for (typename underlying_type::size_type pos{}, last = underlying_type::free_list();
                     pos < last; ++
                     pos) {
                    const auto &entt = underlying_type::base_type::operator[](pos);
                    const auto &old_value = this->get(entt);
                    destruction.publish(reg, entt, old_value);
                }
            } else {
                for (auto entt : static_cast<underlying_type::base_type &>(*this)) {
                    const auto &old_value = this->get(entt);
                    if constexpr (underlying_type::storage_policy == deletion_policy::in_place) {
                        if (entt != tombstone) {
                            destruction.publish(reg, entt, old_value);
                        }
                    } else {
                        destruction.publish(reg, entt, old_value);
                    }
                }
            }
        }

        underlying_type::pop_all();
    }

    underlying_iterator try_emplace(const underlying_type::entity_type entt,
                                    const bool force_back,
                                    const void *value) final {
        const auto it = underlying_type::try_emplace(entt, force_back, value);

        if (auto &reg = owner_or_assert(); it != underlying_type::base_type::end()) {
            const auto &comp_value = this->get(entt);
            construction.publish(reg, *it, comp_value);
        }

        return it;
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
    change_mixin_t()
        : change_mixin_t{allocator_type{}} {
    }

    /**
     * @brief Constructs an empty storage with a given allocator.
     * @param allocator The allocator to use.
     */
    explicit change_mixin_t(const allocator_type &allocator)
        : underlying_type{allocator},
          owner{},
          construction{allocator},
          destruction{allocator},
          update{allocator} {
    }

    /*! @brief Default copy constructor, deleted on purpose. */
    change_mixin_t(const change_mixin_t &) = delete;

    /**
     * @brief Move constructor.
     * @param other The instance to move from.
     */
    // NOLINTBEGIN(bugprone-use-after-move)
    change_mixin_t(change_mixin_t &&other) noexcept
        : underlying_type{std::move(other)},
          owner{other.owner},
          construction{std::move(other.construction)},
          destruction{std::move(other.destruction)},
          update{std::move(other.update)} {
    }

    // NOLINTEND(bugprone-use-after-move)

    /**
     * @brief Allocator-extended move constructor.
     * @param other The instance to move from.
     * @param allocator The allocator to use.
     */
    // NOLINTBEGIN(bugprone-use-after-move)
    change_mixin_t(change_mixin_t &&other, const allocator_type &allocator)
        : underlying_type{std::move(other), allocator},
          owner{other.owner},
          construction{std::move(other.construction), allocator},
          destruction{std::move(other.destruction), allocator},
          update{std::move(other.update), allocator} {
    }

    // NOLINTEND(bugprone-use-after-move)

    /*! @brief Default destructor. */
    ~change_mixin_t() override = default;

    /**
     * @brief Default copy assignment operator, deleted on purpose.
     * @return This mixin.
     */
    change_mixin_t &operator=(const change_mixin_t &) = delete;

    /**
     * @brief Move assignment operator.
     * @param other The instance to move from.
     * @return This mixin.
     */
    change_mixin_t &operator=(change_mixin_t &&other) noexcept {
        swap(other);
        return *this;
    }

    /**
     * @brief Exchanges the contents with those of a given storage.
     * @param other Storage to exchange the content with.
     */
    void swap(change_mixin_t &other) noexcept {
        using std::swap;
        swap(owner, other.owner);
        swap(construction, other.construction);
        swap(destruction, other.destruction);
        swap(update, other.update);
        underlying_type::swap(other);
    }

    /**
     * @brief Returns a sink object.
     *
     * The sink returned by this function can be used to receive notifications
     * whenever a new instance is created and assigned to an entity.<br/>
     * Listeners are invoked after the object has been assigned to the entity.
     *
     * @sa sink
     *
     * @return A temporary sink object.
     */
    [[nodiscard]] auto on_construct() noexcept {
        return sink{construction};
    }

    /**
     * @brief Returns a sink object.
     *
     * The sink returned by this function can be used to receive notifications
     * whenever an instance is explicitly updated.<br/>
     * Listeners are invoked after the object has been updated.
     *
     * @sa sink
     *
     * @return A temporary sink object.
     */
    [[nodiscard]] auto on_update() noexcept {
        return sink{update};
    }

    /**
     * @brief Returns a sink object.
     *
     * The sink returned by this function can be used to receive notifications
     * whenever an instance is removed from an entity and thus destroyed.<br/>
     * Listeners are invoked before the object has been removed from the entity.
     *
     * @sa sink
     *
     * @return A temporary sink object.
     */
    [[nodiscard]] auto on_destroy() noexcept {
        return sink{destruction};
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
     * @brief Creates a new identifier or recycles a destroyed one.
     * @return A valid identifier.
     */
    auto generate() {
        const auto entt = underlying_type::generate();
        construction.publish(owner_or_assert(), entt);
        return entt;
    }

    /**
     * @brief Creates a new identifier or recycles a destroyed one.
     * @param hint Required identifier.
     * @return A valid identifier.
     */
    entity_type generate(const entity_type hint) {
        const auto entt = underlying_type::generate(hint);
        const auto &value = this->get(entt);
        construction.publish(owner_or_assert(), entt, value);
        return entt;
    }

    /**
     * @brief Assigns each element in a range an identifier.
     * @tparam It Type of mutable forward iterator.
     * @param first An iterator to the first element of the range to generate.
     * @param last An iterator past the last element of the range to generate.
     */
    template<typename It>
    void generate(It first, It last) {
        underlying_type::generate(first, last);

        if (auto &reg = owner_or_assert(); !construction.empty()) {
            for (; first != last; ++first) {
                const auto &value = this->get(*first);
                construction.publish(reg, *first, value);
            }
        }
    }

    /**
     * @brief Assigns an entity to a storage and constructs its object.
     * @tparam Args Types of arguments to forward to the underlying storage.
     * @param entt A valid identifier.
     * @param args Parameters to forward to the underlying storage.
     * @return A reference to the newly created object.
     */
    template<typename... Args>
    decltype(auto) emplace(const entity_type entt, Args &&... args) {
        const auto &value = underlying_type::emplace(entt, std::forward<Args>(args)...);
        construction.publish(owner_or_assert(), entt, value);
        return value;
    }

    /**
     * @brief Updates the instance assigned to a given entity in-place.
     * @tparam Func Types of the function objects to invoke.
     * @param entt A valid identifier.
     * @param func Valid function objects.
     * @return A reference to the patched instance.
     */
    template<typename... Func>
    decltype(auto) patch(const entity_type entt, Func &&... func) {
        const auto old_value = this->get(entt);
        underlying_type::patch(entt, std::forward<Func>(func)...);
        const auto new_value = this->get(entt);
        update.publish(owner_or_assert(), entt, old_value, new_value);
        return new_value;
    }

    /**
     * @brief Assigns one or more entities to a storage and constructs their
     * objects from a given instance.
     * @tparam It Type of input iterator.
     * @tparam Args Types of arguments to forward to the underlying storage.
     * @param first An iterator to the first element of the range of entities.
     * @param last An iterator past the last element of the range of entities.
     * @param args Parameters to use to forward to the underlying storage.
     */
    template<typename It, typename... Args>
    void insert(It first, It last, Args &&... args) {
        auto from = underlying_type::size();
        underlying_type::insert(first, last, std::forward<Args>(args)...);

        if (auto &reg = owner_or_assert(); !construction.empty()) {
            // fine as long as insert passes force_back true to try_emplace
            for (const auto to = underlying_type::size(); from != to; ++from) {
                const auto &entt = underlying_type::operator[](from);
                const auto &value = this->get(entt);
                construction.publish(reg, underlying_type::operator[](from));
            }
        }
    }

private:
    basic_registry_type *owner;
    construction_type construction;
    update_type update;
    destruction_type destruction;
};


template<typename T>
using change_storage_t = entt::change_mixin_t<entt::storage<T>, entt::basic_registry<> >;
}


#endif //ECS_HISTORY_CHANGE_MIXIN_HPP
