#pragma once

namespace lava::chamber::macros {
/// Force evaluation of an expression
#define $eval(...) $eval_1($eval_1($eval_1(__VA_ARGS__)))
#define $eval_1(...) $eval_2($eval_2($eval_2(__VA_ARGS__)))
#define $eval_2(...) $eval_3($eval_3($eval_3(__VA_ARGS__)))
#define $eval_3(...) $eval_4($eval_4($eval_4(__VA_ARGS__)))
#define $eval_4(...) $eval_5($eval_5($eval_5(__VA_ARGS__)))
#define $eval_5(...) $eval_6($eval_6($eval_6(__VA_ARGS__)))
#define $eval_6(...) __VA_ARGS__

/// Concatenate two words.
#define $cat(a, ...) $cat_(a, __VA_ARGS__)
#define $cat_(a, ...) a##__VA_ARGS__

#define $args_count(...) $args_count_(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define $args_count_(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, n, ...) n

#define $check_(x, n, ...) n
#define $check(...) $check_(__VA_ARGS__, 0)
#define $probe(x) x, 1,

#define $void()
#define $defer(id) id $void()
#define $obstruct(...) __VA_ARGS__ $defer($void)()

/// Boolean operations.
#define $not(x) $check($cat($not_, x))
#define $not_0 $probe(~)

#define $not_bool(n) $check($cat($not_bool_, n))
#define $not_bool_0 $probe(~)
#define $bool(n) $not($not_bool(n))

/// Strings operations.
#define $is_string_empty(s) $check($is_string_empty_##s)
#define $is_string_empty_ $probe(~)

#define $are_arguments_empty(...) $cat($are_arguments_empty_, $bool($args_count(__VA_ARGS__)))(__VA_ARGS__)
#define $are_arguments_empty_0(...) 0
#define $are_arguments_empty_1(value, ...) $not($is_string_empty(value))

// $parameter_drop_types(A, a, B&, b) -> a, b
#define $parameter_drop_types(...) $parameter_drop_types_($args_count(__VA_ARGS__), __VA_ARGS__)
#define $parameter_drop_types_(n, ...) $cat($parameter_drop_types_, n)(__VA_ARGS__)
#define $parameter_drop_types_1()
#define $parameter_drop_types_2(type, name, ...) name
#define $parameter_drop_types_4(type, name, ...) name, $parameter_drop_types_2(__VA_ARGS__)
#define $parameter_drop_types_6(type, name, ...) name, $parameter_drop_types_4(__VA_ARGS__)
#define $parameter_drop_types_8(type, name, ...) name, $parameter_drop_types_6(__VA_ARGS__)
#define $parameter_drop_types_10(type, name, ...) name, $parameter_drop_types_8(__VA_ARGS__)

// $parameter_merge(A, a, B&, b) -> A a, B& b
#define $parameter_merge(...) $parameter_merge_($args_count(__VA_ARGS__), __VA_ARGS__)
#define $parameter_merge_(n, ...) $cat($parameter_merge_, n)(__VA_ARGS__)
#define $parameter_merge_1()
#define $parameter_merge_2(type, name, ...) type name
#define $parameter_merge_4(type, name, ...) type name, $parameter_merge_2(__VA_ARGS__)
#define $parameter_merge_6(type, name, ...) type name, $parameter_merge_4(__VA_ARGS__)
#define $parameter_merge_8(type, name, ...) type name, $parameter_merge_6(__VA_ARGS__)
#define $parameter_merge_10(type, name, ...) type name, $parameter_merge_8(__VA_ARGS__)

// $type_void(whatever) -> 0    $type_void(void) -> 1
#define $type_void(x) $check($cat($type_void_, x))
#define $type_void_void $probe(~)
}
