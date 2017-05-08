#pragma once

namespace lava {
#define $property_declaration(type, name, ...)                                                                                   \
private:                                                                                                                         \
    type m_##name __VA_ARGS__;

#define $property_getter(type, name)                                                                                             \
public:                                                                                                                          \
    type& name() { return m_##name; }                                                                                            \
    const type& name() const { return m_##name; }

#define $property_readonly(type, name, ...)                                                                                      \
    $property_getter(type, name);                                                                                                \
    $property_declaration(type, name, __VA_ARGS__);
}
