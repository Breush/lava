#pragma once

namespace lava::sill {
    template <class ComponentClass, class... Arguments>
    inline ComponentClass& Entity::make(Arguments&&... arguments)
    {
        auto pComponent = std::make_unique<ComponentClass>(*this, std::forward<Arguments>(arguments)...);
        auto& component = *pComponent;
        add(ComponentClass::hrid(), std::move(pComponent));
        return component;
    }

    template <class ComponentClass>
    inline ComponentClass& Entity::ensure()
    {
        if (!has<ComponentClass>()) {
            return make<ComponentClass>();
        }
        return get<ComponentClass>();
    }
}
