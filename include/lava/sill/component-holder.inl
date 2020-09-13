#pragma once

namespace lava::sill {
    template <class ComponentClass>
    inline bool ComponentHolder::has() const
    {
        return hasComponent(ComponentClass::hrid());
    }

    template <class ComponentClass>
    inline ComponentClass& ComponentHolder::get()
    {
        auto& component = getComponent(ComponentClass::hrid());
        return reinterpret_cast<ComponentClass&>(component);
    }

    template <class ComponentClass>
    const ComponentClass& ComponentHolder::get() const
    {
        const auto& component = getComponent(ComponentClass::hrid());
        return reinterpret_cast<const ComponentClass&>(component);
    }

    template <class ComponentClass>
    inline void ComponentHolder::remove()
    {
        removeComponent(ComponentClass::hrid());
    }
}
