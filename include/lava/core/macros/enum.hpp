#pragma once

#include <lava/core/macros/common.hpp>

#include <ostream>

namespace lava::chamber::macros {
/**
 * Generates an enum class with stringify functions.
 *
 * ```c++
 * $enum_class(lava, Power,
 *      Low,
 *      High,
 * );
 * ```
 *
 * will generate:
 *
 * ```c++
 * namespace lava {
 *     enum class Power {
 *         Low,
 *         High,
 *     };
 * };
 *
 * constexpr const char* stringify(lava::Power value)
 * {
 *     switch (value) {
 *     case lava::Power::Low: {
 *         return "lava::Power::Low";
 *     };
 *     case lava::Power::High: {
 *         return "lava::Power::High";
 *     };
 *     default: break;
 *     }
 *     return "lava::Power::<Unknown>";
 * };
 *
 * inline std::ostream& operator<<(std::ostream& os, lava::Power value)
 * {
 *     os << stringify(value);
 *     return os;
 * };
 * ```
 *
 */
#define $enum_class(Namespace, Enum, ...)                                                                                        \
    $enum_class_(Namespace, Enum, __VA_ARGS__);                                                                                  \
    $enum_stringify(Namespace, Enum, __VA_ARGS__);                                                                               \
    $enum_ostream(Namespace, Enum);
#define $enum_class_(Namespace, Enum, ...)                                                                                       \
    namespace Namespace {                                                                                                        \
        enum class Enum { __VA_ARGS__ };                                                                                         \
    }

#define $enum_stringify(Namespace, Enum, ...)                                                                                    \
    constexpr const char* stringify(Namespace::Enum value)                                                                       \
    {                                                                                                                            \
        switch (value) {                                                                                                         \
            $enum_stringify_cases(Namespace::Enum, __VA_ARGS__);                                                                 \
        default: break;                                                                                                          \
        }                                                                                                                        \
        return #Namespace "::" #Enum "::<Unknown>";                                                                              \
    }

#define $enum_stringify_cases(Enum, ...) $eval($enum_stringify_cases_(Enum, __VA_ARGS__))
#define $enum_stringify_cases_(Enum, ...) $enum_stringify_case_($are_arguments_empty(__VA_ARGS__), Enum, __VA_ARGS__)
#define $enum_stringify_cases_indirect() $enum_stringify_cases_

#define $enum_stringify_case_(n, Enum, ...) $cat($enum_stringify_case_, n)(Enum, __VA_ARGS__)
#define $enum_stringify_case_0(Enum, ...) /* Empty end case */
#define $enum_stringify_case_1(Enum, Value, ...)                                                                                 \
    $enum_stringify_case(Enum, Value);                                                                                           \
    $obstruct($enum_stringify_cases_indirect)()(Enum, __VA_ARGS__)

#define $enum_stringify_case(Enum, Value) $cat($enum_stringify_case_check_, $is_string_empty(Value))(Enum, Value)
#define $enum_stringify_case_check_1(...) /* Empty end case */
#define $enum_stringify_case_check_0(Enum, Value)                                                                                \
    case Enum::Value: {                                                                                                          \
        return #Enum "::" #Value;                                                                                                \
    }

#define $enum_ostream(Namespace, Enum)                                                                                           \
    inline std::ostream& operator<<(std::ostream& os, Namespace::Enum value)                                                     \
    {                                                                                                                            \
        os << stringify(value);                                                                                                  \
        return os;                                                                                                               \
    }
}
