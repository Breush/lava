#pragma once

namespace lava {
#define $property_declaration(type, name, ...)                                                                                   \
private:                                                                                                                         \
    type m_##name __VA_ARGS__;

#define $property_getter(type, name)                                                                                             \
public:                                                                                                                          \
    type& name() { return m_##name; }                                                                                            \
    const type& name() const { return m_##name; }

#define $property_setter(type, name)                                                                                             \
public:                                                                                                                          \
    void name(const type& value) { m_##name = value; }

/**
 * $attribute is a read-only property.
 */
#define $attribute(type, name, ...)                                                                                              \
    $property_getter(type, name);                                                                                                \
    $property_declaration(type, name, __VA_ARGS__);

/**
 * $property has both getter and setter.
 */
#define $property(type, name, ...)                                                                                               \
    $property_getter(type, name);                                                                                                \
    $property_setter(type, name);                                                                                                \
    $property_declaration(type, name, __VA_ARGS__);
}
