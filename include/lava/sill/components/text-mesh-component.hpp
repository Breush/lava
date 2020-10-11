#pragma once

#include <lava/sill/components/i-component.hpp>

#include <lava/core/alignment.hpp>
#include <lava/core/anchor.hpp>
#include <lava/core/u8string.hpp>

#include <string>

namespace lava::sill {
    class TextMeshComponent final : public IComponent {
    public:
        TextMeshComponent(Entity& entity);

        // IComponent
        static std::string hrid() { return "text-mesh"; }
        void update(float dt) final;

        void text(const u8string& u8Text);
        void font(const std::string& hrid);
        void anchor(Anchor anchor);
        void alignment(Alignment alignment);

    private:
        u8string m_u8Text;                          //< UTF-8 encoded string.
        std::string m_fontHrid = "default";         //< Font used for rendering.
        Anchor m_anchor = Anchor::Center;           //< Anchor of the mesh.
        Alignment m_alignment = Alignment::Start;   //< Alignment of the text for the reading axis.
        bool m_dirty = false;                       //< Whether the mesh should be recreated during next update.
    };
}
