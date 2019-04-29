// @note We cannot really have a separate file for the implementation
// because the tokens enum (like CLEX_id) is defined within the implementation,
// and not the headers.
#define STB_C_LEXER_IMPLEMENTATION

#include <lava/chamber/lexer.hpp>

#include <lava/chamber/logger.hpp>

using namespace lava::chamber;

Lexer::Lexer(std::string_view code)
{
    stb_c_lexer_init(&m_lexer, code.data(), code.data() + code.size(), m_buffer, 100);
}

std::optional<Lexer::Token> Lexer::nextToken()
{
    if (!stb_c_lexer_get_token(&m_lexer)) {
        return std::nullopt;
    }

    switch (m_lexer.token) {
    case CLEX_id: {
        m_token.type = TokenType::Identifier;
        m_token.string = m_lexer.string;
        break;
    }
    case CLEX_dqstring: {
        m_token.type = TokenType::String;
        m_token.string = m_lexer.string;
        break;
    }
    case CLEX_floatlit: {
        m_token.type = TokenType::Number;
        m_token.string = std::to_string(m_lexer.real_number);
        m_token.number = m_lexer.real_number;
        break;
    }
    case CLEX_intlit: {
        m_token.type = TokenType::Number;
        m_token.string = std::to_string(m_lexer.int_number);
        m_token.number = static_cast<double>(m_lexer.int_number);
        break;
    }
    case CLEX_eq: {
        m_token.type = TokenType::EqualEqual;
        m_token.string = "==";
        break;
    }
    case CLEX_noteq: {
        m_token.type = TokenType::NotEqual;
        m_token.string = "!=";
        break;
    }
    case CLEX_lesseq: {
        m_token.type = TokenType::LessOrEqual;
        m_token.string = "<=";
        break;
    }
    case CLEX_greatereq: {
        m_token.type = TokenType::GreaterOrEqual;
        m_token.string = ">=";
        break;
    }
    case CLEX_pluseq: {
        m_token.type = TokenType::PlusEqual;
        m_token.string = "+=";
        break;
    }
    case CLEX_minuseq: {
        m_token.type = TokenType::MinusEqual;
        m_token.string = "-=";
        break;
    }
    case CLEX_muleq: {
        m_token.type = TokenType::MultiplyEqual;
        m_token.string = "*=";
        break;
    }
    case CLEX_diveq: {
        m_token.type = TokenType::DivideEqual;
        m_token.string = "/=";
        break;
    }
    case CLEX_andand: {
        m_token.type = TokenType::AndAnd;
        m_token.string = "&&";
        break;
    }
    case CLEX_oror: {
        m_token.type = TokenType::OrOr;
        m_token.string = "||";
        break;
    }
    default: {
        if (m_lexer.token < 0) {
            m_token.string = "Error";
        }
        else if (m_lexer.token <= 255) {
            char character = static_cast<char>(m_lexer.token);
            m_token.string = character;

            switch (m_lexer.token) {
            case ';': m_token.type = TokenType::Semicolon; break;
            case ',': m_token.type = TokenType::Comma; break;
            case '.': m_token.type = TokenType::Dot; break;
            case '(': m_token.type = TokenType::LeftParenthesis; break;
            case ')': m_token.type = TokenType::RightParenthesis; break;
            case '{': m_token.type = TokenType::LeftBrace; break;
            case '}': m_token.type = TokenType::RightBrace; break;
            case '[': m_token.type = TokenType::LeftBracket; break;
            case ']': m_token.type = TokenType::RightBracket; break;
            case '+': m_token.type = TokenType::Plus; break;
            case '-': m_token.type = TokenType::Minus; break;
            case '*': m_token.type = TokenType::Multiply; break;
            case '/': m_token.type = TokenType::Divide; break;
            case '=': m_token.type = TokenType::Equal; break;
            case '!': m_token.type = TokenType::Not; break;
            case '<': m_token.type = TokenType::Less; break;
            case '>': m_token.type = TokenType::Greater; break;
            case '&': m_token.type = TokenType::BitwiseAnd; break;
            case '|': m_token.type = TokenType::BitwiseOr; break;
            case '#': m_token.type = TokenType::Sharp; break;
            default: logger.warning("chamber.lexer") << "Unhandled single character token '" << character << "'." << std::endl;
            }
        }
        else {
            logger.error("chamber.lexer") << "Unhandled token: " << m_lexer.token << std::endl;
        }
    }
    }

    return m_token;
}

Lexer::TokenContext Lexer::currentTokenContext()
{
    TokenContext tokenContext;

    stb_lex_location location;
    stb_c_lexer_get_location(&m_lexer, m_lexer.where_firstchar, &location);

    tokenContext.line = location.line_number;
    tokenContext.column = location.line_offset;

    return tokenContext;
}
