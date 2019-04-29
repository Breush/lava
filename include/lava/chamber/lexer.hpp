#pragma once

#include <lava/chamber/token-type.hpp>

#define STB_C_LEX_C_DECIMAL_INTS Y
#define STB_C_LEX_C_HEX_INTS Y
#define STB_C_LEX_C_OCTAL_INTS Y
#define STB_C_LEX_C_DECIMAL_FLOATS Y
#define STB_C_LEX_C99_HEX_FLOATS N
#define STB_C_LEX_C_IDENTIFIERS Y
#define STB_C_LEX_C_DQ_STRINGS Y
#define STB_C_LEX_C_SQ_STRINGS N
#define STB_C_LEX_C_CHARS Y
#define STB_C_LEX_C_COMMENTS Y
#define STB_C_LEX_CPP_COMMENTS Y
#define STB_C_LEX_C_COMPARISONS Y
#define STB_C_LEX_C_LOGICAL Y
#define STB_C_LEX_C_SHIFTS Y
#define STB_C_LEX_C_INCREMENTS Y
#define STB_C_LEX_C_ARROW Y
#define STB_C_LEX_EQUAL_ARROW N
#define STB_C_LEX_C_BITWISEEQ Y
#define STB_C_LEX_C_ARITHEQ Y

#define STB_C_LEX_PARSE_SUFFIXES N
#define STB_C_LEX_DECIMAL_SUFFIXES ""
#define STB_C_LEX_HEX_SUFFIXES ""
#define STB_C_LEX_OCTAL_SUFFIXES ""
#define STB_C_LEX_FLOAT_SUFFIXES ""

#define STB_C_LEX_0_IS_EOF N
#define STB_C_LEX_INTEGERS_AS_DOUBLES N
#define STB_C_LEX_MULTILINE_DSTRINGS N
#define STB_C_LEX_MULTILINE_SSTRINGS N
#define STB_C_LEX_USE_STDLIB Y
#define STB_C_LEX_DOLLAR_IDENTIFIER Y
#define STB_C_LEX_FLOAT_NO_DECIMAL Y
#define STB_C_LEX_DEFINE_ALL_TOKEN_NAMES N
#define STB_C_LEX_DISCARD_PREPROCESSOR N // @note The only reason why we copied these settings.

#define STB_C_LEXER_DEFINITIONS

#include <stb/stb_c_lexer.h>

#include <optional>
#include <string_view>

namespace lava::chamber {
    /// Simple wrapper over stb's lexer.
    class Lexer {
    public:
        struct Token {
            TokenType type;
            std::string string;
            double number = 0.0; // @note Integers are converted to numbers anyway.
        };

        struct TokenContext {
            uint32_t line = 1u;
            uint32_t column = 0u;
        };

    public:
        Lexer(std::string_view code);

        std::optional<Token> currentToken() const { return m_token; }
        std::optional<Token> nextToken();
        TokenContext currentTokenContext();

    private:
        stb_lexer m_lexer;
        char m_buffer[100];
        Token m_token;
    };
}
