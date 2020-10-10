#include <lava/sill/components/ui-label-component.hpp>

#include <lava/sill/components/flat-component.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>
#include <lava/sill/makers/quad-flat.hpp>
#include <lava/sill/makers/text-flat.hpp>

using namespace lava::sill;

UiLabelComponent::UiLabelComponent(Entity& entity)
    : UiLabelComponent(entity, std::wstring())
{
}

UiLabelComponent::UiLabelComponent(Entity& entity, const std::wstring& text)
    : IUiComponent(entity)
    , m_flatComponent(entity.ensure<FlatComponent>())
    , m_text(text)
{
    auto& scene2d = entity.engine().scene2d();

    m_extent = {200.f, m_fontSize};
    m_hoveringExtent = m_extent;

    // Background setup
    {
        auto& node = makers::quadFlatMaker(1.f)(m_flatComponent);
        node.name = "background";
        node.flatGroup->primitive(0u).material(scene2d.makeMaterial("ui.quad"));
        node.transform(glm::scale(glm::mat3(1.f), m_extent));
    }

    m_textDirty = !m_text.empty();

    updateHovered();
}

void UiLabelComponent::update(float /* dt */)
{
    if (m_textDirty) {
        updateText();
    }
}

// ----- IUiComponent

void UiLabelComponent::hovered(bool hovered)
{
    if (m_hovered == hovered) return;
    m_hovered = hovered;
    updateHovered();
}

void UiLabelComponent::dragEnd(const glm::ivec2& mousePosition)
{
    m_beingClicked = false;

    // If still hovered, call callback
    if (checkHovered(mousePosition) &&
        m_clickedCallback != nullptr) {
        m_clickedCallback();
    }
}

// ----- Internal

void UiLabelComponent::updateText()
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

    m_textDirty = false;
}

void UiLabelComponent::updateHovered()
{
    auto& backgroundMaterial = *m_flatComponent.node("background").flatGroup->primitive(0u).material();

    if (m_hovered) {
        backgroundMaterial.set("topColor", glm::vec4{0.5, 0.6, 0.8, 1});
        backgroundMaterial.set("bottomColor", glm::vec4{0.5, 0.6, 0.9, 1});
    } else {
        backgroundMaterial.set("topColor", m_backgroundColor);
        backgroundMaterial.set("bottomColor", m_backgroundColor);
    }
}
