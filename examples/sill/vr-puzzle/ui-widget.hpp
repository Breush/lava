#pragma once

enum class UiWidgetKind {
    Unknown,
    TextEntry,
    ToggleButton,
    Select,
};

class UiWidget
{
public:
    using TextEntryGetter = std::function<const std::string&()>;
    using TextEntrySetter = std::function<void(const std::string&)>;
    using ToggleButtonGetter = std::function<bool()>;
    using ToggleButtonSetter = std::function<void(bool)>;
    using SelectGetter = std::function<uint8_t()>;
    using SelectSetter = std::function<void(uint8_t)>;

    struct TextEntryInfo {
        TextEntryGetter getter = nullptr;
        TextEntrySetter setter = nullptr;
    };

    struct ToggleButtonInfo {
        std::string text;
        ToggleButtonGetter getter = nullptr;
        ToggleButtonSetter setter = nullptr;
    };

    struct SelectInfo {
        std::vector<std::string_view> options;
        SelectGetter getter = nullptr;
        SelectSetter setter = nullptr;
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
    UiWidget(std::string id, std::vector<std::string_view> options, SelectGetter getter, SelectSetter setter) {
        m_id = std::move(id);
        m_kind = UiWidgetKind::Select;
        m_select.options = std::move(options);
        m_select.getter = getter;
        m_select.setter = setter;
    }

    std::string id() const { return m_id; }
    UiWidgetKind kind() const { return m_kind; }
    lava::sill::Entity* entity() const { return m_entity; }
    void entity(lava::sill::Entity* entity) { m_entity = entity; }

    const TextEntryInfo& textEntry() const { return m_textEntry; }
    const ToggleButtonInfo& toggleButton() const { return m_toggleButton; }
    const SelectInfo& select() const { return m_select; }

private:
    std::string m_id;
    UiWidgetKind m_kind = UiWidgetKind::Unknown;
    lava::sill::Entity* m_entity = nullptr;

    // @note Can't union those easily, none trivial destructors.
    TextEntryInfo m_textEntry;
    ToggleButtonInfo m_toggleButton;
    SelectInfo m_select;
};
