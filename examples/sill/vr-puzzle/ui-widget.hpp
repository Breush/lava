#pragma once

enum class UiWidgetKind {
    Unknown,
    TextEntry,
    ToggleButton,
};

class UiWidget
{
public:
    using TextEntryGetter = std::function<const std::string&()>;
    using TextEntrySetter = std::function<void(const std::string&)>;
    using ToggleButtonGetter = std::function<bool()>;
    using ToggleButtonSetter = std::function<void(bool)>;

    struct TextEntryInfo {
        TextEntryGetter getter = nullptr;
        TextEntrySetter setter = nullptr;
    };

    struct ToggleButtonInfo {
        std::string text;
        ToggleButtonGetter getter = nullptr;
        ToggleButtonSetter setter = nullptr;
    };

public:
    UiWidget(std::string id, TextEntryGetter getter, TextEntrySetter setter) {
        m_id = std::move(id);
        m_kind = UiWidgetKind::TextEntry;
        m_textEntry.getter = getter;
        m_textEntry.setter = setter;
    }
    UiWidget(std::string id, std::string text, ToggleButtonGetter getter, ToggleButtonSetter setter) {
        m_id = std::move(id);
        m_kind = UiWidgetKind::ToggleButton;
        m_toggleButton.text = std::move(text);
        m_toggleButton.getter = getter;
        m_toggleButton.setter = setter;
    }

    std::string id() const { return m_id; }
    UiWidgetKind kind() const { return m_kind; }
    lava::sill::GameEntity* entity() const { return m_entity; }
    void entity(lava::sill::GameEntity* entity) { m_entity = entity; }

    const TextEntryInfo& textEntry() const { return m_textEntry; }
    const ToggleButtonInfo& toggleButton() const { return m_toggleButton; }

private:
    std::string m_id;
    UiWidgetKind m_kind = UiWidgetKind::Unknown;
    lava::sill::GameEntity* m_entity = nullptr;

    // @note Can't union those easily, none trivial destructors.
    TextEntryInfo m_textEntry;
    ToggleButtonInfo m_toggleButton;
};
