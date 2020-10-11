#include <lava/sill/components/ui-text-entry-component.hpp>

#include <lava/sill/components/flat-component.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>
#include <lava/sill/makers/quad-flat.hpp>
#include <lava/sill/makers/text-flat.hpp>

using namespace lava::sill;

UiTextEntryComponent::UiTextEntryComponent(Entity& entity)
    : UiTextEntryComponent(entity, u8string())
{
}

UiTextEntryComponent::UiTextEntryComponent(Entity& entity, const u8string& text)
    : IUiComponent(entity)
    , m_flatComponent(entity.ensure<FlatComponent>())
    , m_text(text)
{
    auto& scene2d = entity.engine().scene2d();

    m_extent = {300.f, m_fontSize};
    m_hoveringExtent = m_extent;

    // Background setup
    {
        auto& node = makers::quadFlatMaker(1.f)(m_flatComponent);
        node.name = "background";
        node.flatGroup->primitive(0u).material(scene2d.makeMaterial("ui.quad"));
        node.transform(glm::scale(glm::mat3(1.f), m_extent));
    }

    // Cursor setup
    {
        auto& node = makers::quadFlatMaker(1.f)(m_flatComponent);
        node.name = "cursor";
        node.flatGroup->primitive(0u).material(scene2d.makeMaterial("ui.quad"));
        updateCursor();
    }

    m_textDirty = !m_text.empty();
    m_cursorPosition = m_text.size();
}

void UiTextEntryComponent::update(float /* dt */)
{
    if (m_textDirty) {
        updateText();
    }
}

// ----- IUiComponent

void UiTextEntryComponent::hovered(bool hovered)
{
    if (m_hovered == hovered) return;
    m_hovered = hovered;
    updateHovered();
}

void UiTextEntryComponent::keyPressed(Key key, bool& propagate)
{
    if (key == Key::Escape) return;
    propagate = false;

    if (key == Key::Backspace) {
        // Remove previous character
        if (m_cursorPosition != 0u) {
            uint8_t bytesLength = chamber::utf8CodepointBytesLengthBackwards(m_text.c_str() + m_cursorPosition);
            m_text.erase(m_cursorPosition - bytesLength, bytesLength);
            m_cursorPosition -= bytesLength;
            m_textDirty = true;
        }
    }
    else if (key == Key::Delete) {
        // Remove next character
        if (m_cursorPosition < m_text.size()) {
            uint8_t bytesLength = chamber::utf8CodepointBytesLength(m_text.c_str() + m_cursorPosition);
            m_text.erase(m_cursorPosition, bytesLength);
            m_textDirty = true;
        }
    }
    else if (key == Key::Left) {
        if (m_cursorPosition != 0u) {
            uint8_t bytesLength = chamber::utf8CodepointBytesLengthBackwards(m_text.c_str() + m_cursorPosition);
            m_cursorPosition -= bytesLength;
            updateCursor();
        }
    }
    else if (key == Key::Right) {
        if (m_cursorPosition < m_text.size()) {
            uint8_t bytesLength = chamber::utf8CodepointBytesLength(m_text.c_str() + m_cursorPosition);
            m_cursorPosition += bytesLength;
            updateCursor();
        }
    }
}

void UiTextEntryComponent::textEntered(uint32_t codepoint, bool& propagate)
{
    propagate = false;

    auto u8Character = chamber::codepointToU8String(codepoint);
    m_text.insert(m_cursorPosition, u8Character);
    m_cursorPosition += u8Character.size();
    m_textDirty = true;
}

// ----- Internal

void UiTextEntryComponent::updateText()
{
    m_flatComponent.removeNode("text");

    if (!m_text.empty()) {
        makers::TextFlatOptions options;
        options.fontSize = m_fontSize;
        options.anchor = Anchor::Left;
        auto& node = makers::textFlatMaker(m_text, options)(m_flatComponent);
        node.transform(glm::translate(glm::mat3(1.f), {-m_extent.x / 2.f + m_horizontalPadding, 0.f}));
        node.name = "text";
    }

    updateCursor();
    m_textDirty = false;

    // @todo And what if the text goes outside?
    // Do we want some clipping system or we just hide the flat primitives?

    for (const auto& callback : m_textChangedCallbacks) {
        callback(m_text);
    }
}

// @todo :UiFocus Have concept of focusing, because typing only on hovered is a bit strange
void UiTextEntryComponent::updateHovered()
{
    auto& cursorMaterial = *m_flatComponent.node("cursor").flatGroup->primitive(0u).material();
    cursorMaterial.set("topColor", glm::vec4{0, 0, 0, (m_hovered) ? 1 : 0});
    cursorMaterial.set("bottomColor", glm::vec4{0, 0, 0, (m_hovered) ? 1 : 0});
}

void UiTextEntryComponent::updateCursor()
{
    auto translation = glm::vec2{-m_extent.x / 2.f + m_horizontalPadding, 0.f};

    // Find the position to place the cursor
    if (m_cursorPosition > 0u) {
        uint8_t bytesLength = chamber::utf8CodepointBytesLengthBackwards(m_text.c_str() + m_cursorPosition);
        auto glyphInfo = m_entity.engine().font().get("default", m_fontSize).glyphInfoAtByte(m_text, m_cursorPosition - bytesLength);

        translation.x += (glyphInfo.xOffset + glyphInfo.advance) * m_fontSize;
    }

    auto& node = m_flatComponent.node("cursor");
    auto transform = glm::mat3(1.f);
    transform = glm::translate(transform, translation);
    transform = glm::scale(transform, {1.f, 0.95f * m_extent.y});
    node.transform(transform);
}
