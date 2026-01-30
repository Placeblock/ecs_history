# Ecs History - History for EnTT

This is a small library I developed to track changes in an EnTT Registry.
It records changes and creates commits.
You can store the commits somewhere or send them over the network,
apply changes to a registry, invert and apply changes to undo them or
pretty much anything you want.

## Changes

The component_change_t struct contains the change of one component of one entity.

- The construct_change_t stores the data of a newly constructed component.
- The update_change_t stores the previous data and the new data of an updated component.
- The destruct_change_t stores the previous data of a destructed component.

These changes are templates with the component type as a template parameter.
I could have used EnTT's meta system, however for my use case I needed
minimal memory overhead so I went with templates.
However, we will see later that using the meta system is still possible.

### Using Changes

At some point you will probably use these changes, for example serialize them
or apply them to a registry.

You may have a vector of different changes

```c++
std::vector<std::unique_ptr<component_change_t<T>>> changes;
```

*(We use a unique_ptr here, because component_change_t<T> is pure virtual)*

If you have heard of the visitor pattern you may recognize the **following approach**:
<br>
The component_change_supplier_t is an interface with the following methods:

```c++
    template<typename T>
    class component_change_supplier_t {
    public:
        virtual void apply(const construct_change_t<T> &c) = 0;

        virtual void apply(const update_change_t<T> &c) = 0;

        virtual void apply(const destruct_change_t<T> &c) = 0;

        virtual ~component_change_supplier_t() = default;
    };
```

The component changes implement the apply method:

```c++
        virtual void apply(entity_change_supplier_t &applier) const = 0;
```

Because the different component change implementations implement the apply method,
when calling apply with any entity_change_supplier_t the correct method in the supplier will be called.
This allows you to implement implementation agnostic functionality without
using dynamic casts or type checks. You could call it inheritance for composition?

## Commits

The component_commit_t is a collection of component changes of the same type.

You can iterate over a component_commit_t by using the visitor pattern.

## Performance

I strongly advice running the performance test (in /test) yourself, but here are my results:

> Creating 1.000.000 Entities: 158ms
>
> Adding Components to 1.000.000 Entities: 38ms
>
> Creating commit of 1.000.000 Entities with 1 created component each: 185ms
>
> Serializing commit of 1.000.000 Entities with 1 component created each: 365ms
>
> Deserializing commit of 1.000.000 Entities with 1 component created each: 548ms
>
> Applying commit of 1.000.000 Entities with 1 component created each: 207ms
>
> Replacing Components on 1.000.000 Entities: 32ms
>
> Creating commit of 1.000.000 Entities with 1 component replaced each: 180ms
>
> Serializing commit of 1.000.000 Entities with 1 component replaced each: 342ms
>
> Deserializing commit of 1.000.000 Entities with 1 component replaced each: 537ms
>
> Applying commit of 1.000.000 Entities with 1 component replaced each: 75ms