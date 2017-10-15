#pragma once

namespace lava::sill {
    template <class ComponentClass>
    inline bool GameEntity::has() const
    {
        return hasComponent(ComponentClass::hrid());
    }

    template <class ComponentClass>
    inline ComponentClass& GameEntity::get()
    {
        auto& component = getComponent(ComponentClass::hrid());
        return reinterpret_cast<ComponentClass&>(component);
    }

    template <class ComponentClass>
    const ComponentClass& GameEntity::get() const
    {
        const auto& component = getComponent(ComponentClass::hrid());
        return reinterpret_cast<const ComponentClass&>(component);
    }

    template <class ComponentClass>
    inline ComponentClass& GameEntity::make()
    {
        auto pComponent = std::make_unique<ComponentClass>(*this);
        auto& component = *pComponent;
        add(ComponentClass::hrid(), std::move(pComponent));
        return component;
    }

    template <class ComponentClass>
    inline ComponentClass& GameEntity::ensure()
    {
        if (!has<ComponentClass>()) {
            return make<ComponentClass>();
        }
        return get<ComponentClass>();
    }

    template <class ComponentClass>
    inline void GameEntity::remove()
    {
        removeComponent(ComponentClass::hrid());
    }
}
