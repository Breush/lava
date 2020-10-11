#include <lava/sill/components/ui-select-component.hpp>

#include <lava/sill/components/flat-component.hpp>
#include <lava/sill/components/transform-component.hpp>
#include <lava/sill/game-engine.hpp>
#include <lava/sill/entity.hpp>
#include <lava/sill/makers/quad-flat.hpp>
#include <lava/sill/makers/text-flat.hpp>

using namespace lava::sill;

UiSelectComponent::UiSelectComponent(Entity& entity)
    : UiSelectComponent(entity, {}, 0xFF)
{
}

UiSelectComponent::UiSelectComponent(Entity& entity, std::vector<u8string> options, uint8_t index)
    : IUiComponent(entity)
    , m_flatComponent(entity.ensure<FlatComponent>())
    , m_options(std::move(options))
    , m_index(index)
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

    m_textDirty = (m_index != 0xFF);
}

void UiSelectComponent::update(float /* dt */)
{
    if (m_textDirty) {
        updateText();
    }
}

// ----- IUiComponent

void UiSelectComponent::hovered(bool hovered)
{
    if (m_hovered == hovered) return;
    m_hovered = hovered;

    m_hoveringExtent = m_extent;
    m_hoveringOffset = glm::vec2(0.f);
    if (m_hovered) {
        m_hoveringExtent.y = (m_options.size() + 1) * m_fontSize;
        m_hoveringOffset.y = m_options.size() * m_fontSize / 2.f;
    }

    updateHovered();
}

void UiSelectComponent::dragEnd(const glm::ivec2& mousePosition)
{
    m_beingClicked = false;

    if (checkHovered(mousePosition)) {
        auto position = topLeftRelativePosition(mousePosition);
        auto clickedRow = static_cast<uint8_t>(position.y / m_fontSize);
        if (clickedRow > m_options.size()) {
            clickedRow = static_cast<uint8_t>(m_options.size());
        }

        if (clickedRow > 0) {
            hovered(false);

            if (m_index != clickedRow - 1u) {
                m_index = clickedRow - 1u;
                updateText();

                for (const auto& callback : m_indexChangedCallbacks) {
                    callback(m_index, m_options[m_index]);
                }

            }
        }
    }
}

// ----- Internal

void UiSelectComponent::updateText()
{
    m_flatComponent.removeNode("text");
    m_textDirty = false;

    if (m_index == 0xFF) return;

    const auto& text = m_options[m_index];
    if (!text.empty()) {
        makers::TextFlatOptions options;
        options.fontSize = m_fontSize;
        options.anchor = Anchor::Left;
        auto& node = makers::textFlatMaker(text, options)(m_flatComponent);
        node.transform(glm::translate(glm::mat3(1.f), {-m_extent.x / 2.f + m_horizontalPadding, 0.f}));
        node.name = "text";
    }
}

void UiSelectComponent::updateHovered()
{
    if (m_beingClicked) return;

    m_flatComponent.removeNode("options-background");
    m_flatComponent.removeNodes("options-text");

    if (!m_hovered || m_options.empty()) return;

    auto& scene2d = m_entity.engine().scene2d();

    // Background setup
    {
        auto& node = makers::quadFlatMaker(1.f)(m_flatComponent);
        node.name = "options-background";
        node.flatGroup->primitive(0u).material(scene2d.makeMaterial("ui.quad"));
        auto yOffset = (m_options.size() + 1u) * m_fontSize / 2.f;
        node.transform(glm::scale(glm::translate(glm::mat3(1.f), {0.f, yOffset}), {m_extent.x, m_options.size() * m_fontSize}));
    }

    // Texts
    for (auto i = 0u; i < m_options.size(); ++i) {
        const auto& text = m_options[i];
        makers::TextFlatOptions options;
        options.fontSize = m_fontSize;
        options.anchor = Anchor::Left;
        auto& node = makers::textFlatMaker(text, options)(m_flatComponent);
        node.transform(glm::translate(glm::mat3(1.f), {-m_extent.x / 2.f + m_horizontalPadding, (i + 1) * m_fontSize}));
        node.name = "options-text";
    }
}
