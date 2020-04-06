#pragma once

#include <lava/chamber/token-type.hpp>

#include <optional>
#include <string_view>

#define LAVA_STB_C_LEXER
#include <lava/chamber/stb/c_lexer.h>

namespace lava::chamber {
    /// Simple wrapper over stb's lexer.
    class Lexer {
    public:
        struct Token {
            TokenType type;
            std::string spacing; // Spacing before token
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
