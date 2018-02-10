#pragma once

#include <lava/sill/components/i-component.hpp>

#include <lava/core/alignment.hpp>
#include <lava/core/anchor.hpp>

#include <string>

namespace lava::sill {
    class TextMeshComponent final : public IComponent {
    public:
        TextMeshComponent(GameEntity& entity);
        ~TextMeshComponent();

        // IComponent
        static std::string hrid() { return "text-mesh"; }
        void update() override final;
        void postUpdate() override final;

        void text(const std::wstring& u16Text);
        void font(const std::string& hrid);

        void horizontalAnchor(Anchor horizontalAnchor);
        void verticalAnchor(Anchor verticalAnchor);
        void alignment(Alignment alignment);

    public:
        class Impl;
        Impl& impl() { return *m_impl; }

    private:
        Impl* m_impl = nullptr;
    };
}
