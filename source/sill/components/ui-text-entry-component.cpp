#include <lava/sill/components/ui-text-entry-component.hpp>

#include <lava/sill/components/flat-component.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>
#include <lava/sill/makers/quad-flat.hpp>
#include <lava/sill/makers/text-flat.hpp>

using namespace lava::sill;

UiTextEntryComponent::UiTextEntryComponent(Entity& entity)
    : UiTextEntryComponent(entity, std::wstring())
{
}

UiTextEntryComponent::UiTextEntryComponent(Entity& entity, const std::wstring& text)
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

void UiTextEntryComponent::textEntered(Key key, wchar_t code, bool& propagate)
{
    if (key == Key::Escape) return;
    propagate = false;

    if (key == Key::Backspace) {
        // Remove previous character
        if (m_cursorPosition != 0u) {
            m_text.erase(m_text.begin() + m_cursorPosition - 1u);
            m_cursorPosition -= 1u;
            m_textDirty = true;
        }
    }
    else if (key == Key::Delete) {
        // Remove next character
        if (m_cursorPosition < m_text.size()) {
            m_text.erase(m_text.begin() + m_cursorPosition);
            m_textDirty = true;
        }
    }
    else if (key == Key::Left) {
        if (m_cursorPosition != 0u) {
            m_cursorPosition -= 1u;
            updateCursor();
        }
    }
    else if (key == Key::Right) {
        if (m_cursorPosition < m_text.size()) {
            m_cursorPosition += 1u;
            updateCursor();
        }
    }
    else if (code != 0u) {
        m_text.insert(m_text.begin() + m_cursorPosition, code);
        m_cursorPosition += 1u;
        m_textDirty = true;
    }
}

// ----- Internal

void UiTextEntryComponent::updateText()
{
    m_flatComponent.removeNode("text");

    if (!m_text.empty()) {
        makers::TextFlatOptions options;
        options.fontSize = m_fontSize;
        options.horizontalAnchor = Anchor::Start;
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
        auto glyphInfos = m_entity.engine().font().get("default", m_fontSize).glyphsInfos(m_text, false);

        for (auto i = 0u; i < m_cursorPosition; ++i) {
            const auto& glyphInfo = glyphInfos[i];
            translation.x += glyphInfo.xOffset * m_fontSize;
        }

        translation.x += glyphInfos[m_cursorPosition - 1u].advance * m_fontSize;
    }

    auto& node = m_flatComponent.node("cursor");
    auto transform = glm::mat3(1.f);
    transform = glm::translate(transform, translation);
    transform = glm::scale(transform, {1.f, 0.95f * m_extent.y});
    node.transform(transform);
}
