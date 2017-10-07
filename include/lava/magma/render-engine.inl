#pragma once

namespace lava::magma {
    template <class TMaterial>
    uint32_t RenderEngine::registerMaterial()
    {
        const auto materialId = registerMaterial(TMaterial::shaderImplementation());
        TMaterial::materialId(materialId);
        return materialId;
    }

    template <class T, class... Arguments>
    inline T& RenderEngine::make(Arguments&&... arguments)
    {
        auto pResource = std::make_unique<T>(*this, std::forward<Arguments>(arguments)...);
        auto& resource = *pResource;
        add(std::move(pResource));
        return resource;
    }

    template <class T, class... Arguments>
    inline T& RenderEngine::make(std::function<void(T&)> maker, Arguments&&... arguments)
    {
        auto& resource = make<T>(std::forward<Arguments>(arguments)...);
        maker(resource);
        return resource;
    }
}
