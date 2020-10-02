#include <lava/sill/components/ui-button-component.hpp>

#include <lava/sill/components/flat-component.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>
#include <lava/sill/makers/quad-flat.hpp>
#include <lava/sill/makers/text-flat.hpp>

using namespace lava::sill;

UiButtonComponent::UiButtonComponent(Entity& entity)
    : UiButtonComponent(entity, L"Ok")
{
}

UiButtonComponent::UiButtonComponent(Entity& entity, const std::wstring& text)
    : IUiComponent(entity)
    , m_flatComponent(entity.ensure<FlatComponent>())
{
    auto& scene2d = entity.engine().scene2d();

    // Background setup
    auto& node = makers::quadFlatMaker(1.f)(m_flatComponent);
    node.name = "background";
    node.flatGroup->primitive(0u).material(scene2d.makeMaterial("ui.quad"));
    updateHovered();

    // Affecting text
    this->text(text);
}

void UiButtonComponent::update(float /* dt */)
{
    if (m_textDirty) {
        updateText();
    }
}

// ----- Configuration

void UiButtonComponent::text(const std::wstring& text)
{
    if (m_text == text) return;
    m_text = text;
    m_textDirty = true;
}

// ----- UI manager interaction

void UiButtonComponent::hovered(bool hovered)
{
    if (m_hovered == hovered) return;
    m_hovered = hovered;
    updateHovered();
}

void UiButtonComponent::dragEnd(const glm::ivec2& mousePosition)
{
    beingClicked(false);

    // If still hovered, call callback
    if (checkHovered(mousePosition) &&
        m_clickedCallback != nullptr) {
        m_clickedCallback();
    }
}

// ----- Internal

void UiButtonComponent::updateText()
{
    m_flatComponent.removeNode("text");

    FloatExtent2d extent;
    makers::TextFlatOptions options;
    options.fontSize = 30u;
    options.extentPtr = &extent;
    auto& node = makers::textFlatMaker(m_text, options)(m_flatComponent);
    node.name = "text";

    // @todo :UiPaddingMerge Merge this padding configurations with all UI components?
    constexpr float horizontalPadding = 5.f;
    m_extent = {2.f * horizontalPadding + extent.width, extent.height};
    m_hoveringExtent = m_extent;
    m_flatComponent.node("background").transform(glm::scale(glm::mat3(1.f), m_extent));

    m_textDirty = false;
    warnExtentChanged();
}

void UiButtonComponent::updateHovered()
{
    // @todo Have this kind of feature!
    // m_engine.window().mouseCursor(MouseCursor::Pointer);

    if (m_beingClicked) return;

    m_flatComponent.primitive(0u, 0u).material()->set("topColor", {0.9f, 0.9f, 0.9f, 1.f});
    m_flatComponent.primitive(0u, 0u).material()->set("bottomColor", (m_hovered) ? glm::vec4{0.75, 0.75, 0.75, 1.f} :
                                                                                   glm::vec4{0.8f, 0.8f, 0.8f, 1.f});
}

void UiButtonComponent::updateBeingClicked()
{
    if (!m_beingClicked) {
        updateHovered();
        return;
    }

    m_flatComponent.primitive(0u, 0u).material()->set("topColor", {0.8, 0.8, 0.8, 1.f});
    m_flatComponent.primitive(0u, 0u).material()->set("bottomColor", {0.9f, 0.9f, 0.9f, 1.f});
}

void UiButtonComponent::beingClicked(bool beingClicked)
{
    if (m_beingClicked == beingClicked) return;
    m_beingClicked = beingClicked;
    updateBeingClicked();
}
