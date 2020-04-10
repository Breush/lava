#include <lava/sill/components/ui-button-component.hpp>

#include <lava/sill/components/flat-component.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/game-entity.hpp>
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
    entity.engine().ui().registerEntity(entity);

    auto& scene2d = entity.engine().scene2d();

    // Background setup
    auto& node = makers::quadFlatMaker(1.f)(m_flatComponent);
    node.name = "background";
    node.flatGroup->primitive(0u).material(scene2d.make<magma::Material>("ui.button"));
    updateHovered();

    // Affecting text
    this->text(text);
}

UiButtonComponent::~UiButtonComponent()
{
    m_entity.engine().ui().unregisterEntity(m_entity);
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

bool UiButtonComponent::checkHovered(const glm::ivec2& mousePosition)
{
    const auto& position = m_transformComponent.translation2d();
    bool hovered = (mousePosition.x >= position.x - m_extent.x / 2.f &&
                    mousePosition.x <= position.x + m_extent.x / 2.f &&
                    mousePosition.y >= position.y - m_extent.y / 2.f &&
                    mousePosition.y <= position.y + m_extent.y / 2.f);
    this->hovered(hovered);
    return hovered;
}

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

    makers::TextFlatOptions options;
    options.fontSize = 30u;
    auto& node = makers::textFlatMaker(m_text, options)(m_flatComponent);
    node.name = "text";

    // @todo We're missing the bounding box of flat - that would be useful here!
    m_extent = {options.fontSize * m_text.size() * 0.4f, options.fontSize};
    m_flatComponent.node("background").transform(glm::scale(glm::mat3(1.f), m_extent));

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

void UiButtonComponent::beingClicked(bool beingClicked)
{
    if (m_beingClicked == beingClicked) return;
    m_beingClicked = beingClicked;
    updateBeingClicked();
}
