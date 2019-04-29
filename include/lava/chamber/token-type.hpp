#pragma once

#include <lava/core/macros/enum.hpp>

$enum_class(lava::chamber, TokenType,
            // Unknown
            Unknown,
            // Main
            Identifier,
            // Raw
            Number, String,
            // Single character
            Semicolon, Comma, Dot, LeftParenthesis, RightParenthesis, LeftBrace, RightBrace, LeftBracket, RightBracket, Plus,
            Minus, Multiply, Divide, Equal, Not, Less, Greater, BitwiseAnd, BitwiseOr, Sharp,
            // Double characters
            EqualEqual, NotEqual, LessOrEqual, GreaterOrEqual, PlusEqual, MinusEqual, MultiplyEqual, DivideEqual, AndAnd, OrOr, );
