#include <lava/sill/components/ui-button-component.hpp>

#include <lava/sill/components/flat-component.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>
#include <lava/sill/input-manager.hpp>
#include <lava/sill/makers/quad-flat.hpp>
#include <lava/sill/makers/text-flat.hpp>

using namespace lava::sill;

UiButtonComponent::UiButtonComponent(GameEntity& entity)
    : UiButtonComponent(entity, L"Ok")
{
}

UiButtonComponent::UiButtonComponent(GameEntity& entity, const std::wstring& text)
    : IComponent(entity)
    , m_transformComponent(entity.ensure<TransformComponent>())
    , m_flatComponent(entity.ensure<FlatComponent>())
{
    auto& scene2d = entity.engine().scene2d();

    // Background setup
    auto& node = makers::quadFlatMaker(1.f)(m_flatComponent);
    node.name = "background";
    node.flatGroup->primitive(0u).material(scene2d.make<magma::Material>("ui.button"));
    updateHovered();

    // Affecting text
    this->text(text);
}

void UiButtonComponent::update(float /* dt */)
{
    auto& input = m_entity.engine().input();
    const auto& coords = input.mouseCoordinates();

    if (m_textDirty) {
        updateText();
    }

    const auto& position = m_transformComponent.translation2d();
    bool hovered = (coords.x >= position.x - m_extent.x / 2.f &&
                    coords.x <= position.x + m_extent.x / 2.f &&
                    coords.y >= position.y - m_extent.y / 2.f &&
                    coords.y <= position.y + m_extent.y / 2.f);

    this->hovered(hovered);

    // We were clicked and button is released.
    if (m_beingClicked &&
        input.justUp("ui.main-click")) {
        beingClicked(false);
        if (m_hovered && m_clickedCallback != nullptr) {
            m_clickedCallback();
        }
    }

    if (!m_hovered) return;

    if (m_clickedCallback != nullptr &&
        input.justDown("ui.main-click")) {
        beingClicked(true);
    }
}

void UiButtonComponent::text(const std::wstring& text)
{
    if (m_text == text) return;
    m_text = text;
    m_textDirty = true;
}

void UiButtonComponent::hovered(bool hovered)
{
    if (m_hovered == hovered) return;
    m_hovered = hovered;
    updateHovered();
}

void UiButtonComponent::beingClicked(bool beingClicked)
{
    if (m_beingClicked == beingClicked) return;
    m_beingClicked = beingClicked;
    updateBeingClicked();
}

// ----- Internal

void UiButtonComponent::updateText()
{
    m_flatComponent.removeNode("text");

    makers::TextFlatOptions options;
    options.fontSize = 30u;
    auto& node = makers::textFlatMaker(m_text, options)(m_flatComponent);
    node.name = "text";

    // @todo We're missing the bounding box of flat - that would be useful here!
    m_extent = {options.fontSize * m_text.size() * 0.5f, options.fontSize};
    m_flatComponent.node("background").transform(glm::scale(glm::mat3(1.f), m_extent));

    // @fixme :TransformNeeded This update is needed to refresh the flat position!
    // But shouldn't -- we should be able to refresh our positions after a flat is added in FlatCOmponent
    m_transformComponent.translation2d(m_transformComponent.translation2d());

    m_textDirty = false;
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
