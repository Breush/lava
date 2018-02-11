#pragma once

#include <lava/sill/components/text-mesh-component.hpp>

#include <lava/core/extent.hpp>

#include "../font.hpp"
#include "./component-impl.hpp"

namespace lava::sill {
    class TextMeshComponent::Impl : public ComponentImpl {
    public:
        Impl(GameEntity& entity);

        // IComponent
        void update() override final;

        void text(const std::wstring& u16Text);
        void font(const std::string& hrid);

        void horizontalAnchor(Anchor horizontalAnchor);
        void verticalAnchor(Anchor verticalAnchor);
        void alignment(Alignment alignment);

    protected:
        FloatExtent2d glyphsExtent(const std::vector<Font::GlyphInfo>& glyphsInfos);

    private:
        std::wstring m_text;                        //< UTF-16 encoded string.
        std::string m_fontHrid = "default";         //< Font used for rendering.
        Anchor m_horizontalAnchor = Anchor::CENTER; //< Anchor of the mesh for the horizontal axis.
        Anchor m_verticalAnchor = Anchor::CENTER;   //< Anchor of the mesh for the vertical axis.
        Alignment m_alignment = Alignment::START;   //< Alignment of the text for the reading axis.
        bool m_dirty = false;                       //< Whether the mesh should be recreated during next update.
    };
}
