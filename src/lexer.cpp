#include "lexer.h"
#include <doctest/doctest.h>
#include <ostream>

using namespace std::string_view_literals;

namespace ctpy {

namespace {

    TEST_CASE("keyword def") {
        static constexpr auto content = Content{"def"};
        REQUIRE(lex<content>() == Lexemes{Keyword::def});
    }

    TEST_CASE("identifier") {
        static constexpr auto content = Content{"def abc"};
        static constexpr auto result = lex<content>();
        REQUIRE(result == Lexemes{Keyword::def, Identifier{"abc"}});
    }

    TEST_CASE("is_keyword def") {
        static constexpr auto result = detail::is_keyword("def abc");
        REQUIRE(result.has_value());
        REQUIRE(result->first == Keyword::def);
        REQUIRE(result->second == " abc"sv);
    }

    TEST_CASE("is_operator plus") {
        static constexpr auto result = detail::is_operator("+ abc");
        REQUIRE(result.has_value());
        REQUIRE(result->first == Operator::plus);
        REQUIRE(result->second == " abc"sv);
    }

    TEST_CASE("is_identifier abc") {
        static constexpr auto result = detail::is_identifier("abc abc");
        REQUIRE(result.has_value());
        REQUIRE(result->first == Identifier{"abc"});
        REQUIRE(result->second == " abc"sv);
    }

    TEST_CASE("is_lexeme uses is_identifier") {
        static constexpr auto is_keyword_mock = [](std::string_view const) constexpr noexcept {
            return std::optional<std::pair<Keyword, std::string_view>>{};
        };
        static constexpr auto is_operator_mock = [](std::string_view const) constexpr noexcept {
            return std::optional<std::pair<Operator, std::string_view>>{};
        };
        static constexpr auto is_identifier_mock = [](std::string_view const content) constexpr
                -> std::optional<std::pair<Identifier, std::string_view>> {
            if (content != content) {
                throw "not 'content' passed as content";  // NOLINT(*-exception-baseclass)
            }
            return std::optional<std::pair<Identifier, std::string_view>>{
                    std::in_place, Identifier{"abc"}, "def"};
        };
        static constexpr auto result =
                detail::is_lexeme<is_keyword_mock, is_operator_mock, is_identifier_mock>("content");
        REQUIRE(result.first == Lexeme{Identifier{"abc"}});
        REQUIRE(result.second == "def");
    }

    TEST_CASE("is_lexeme uses is_operator") {
        static constexpr auto is_keyword_mock = [](std::string_view const) constexpr {
            return std::optional<std::pair<Keyword, std::string_view>>{};
        };
        static constexpr auto is_operator_mock = [](std::string_view const content) constexpr
                -> std::optional<std::pair<Operator, std::string_view>> {
            if (content != content) {
                throw "not 'content' passed as content";  // NOLINT(*-exception-baseclass)
            }
            return std::optional<std::pair<Operator, std::string_view>>{
                    std::in_place, Operator::plus, "def"};
        };
        static constexpr auto result =
                detail::is_lexeme<is_keyword_mock, is_operator_mock>("content");
        REQUIRE(result.first == Lexeme{Operator::plus});
        REQUIRE(result.second == "def");
    }

    TEST_CASE("is_lexeme uses is_keyword") {
        static constexpr auto is_keyword_mock = [](std::string_view const content) constexpr {
            if (content != content) {
                throw "not 'content' passed as content";  // NOLINT(*-exception-baseclass)
            }
            return std::optional<std::pair<Keyword, std::string_view>>{
                    std::in_place, Keyword::def, "def"};
        };
        static constexpr auto result = detail::is_lexeme<is_keyword_mock>("content");
        REQUIRE(result.first == Lexeme{Keyword::def});
        REQUIRE(result.second == "def");
    }

    TEST_CASE("lexemes_length 0") {
        REQUIRE(detail::lexeme_length("") == 0);
    }

    TEST_CASE("lexemes_length 1") {
        REQUIRE(detail::lexeme_length("def") == 1);
    }

    TEST_CASE("lexemes_length 3") {
        static constexpr auto result = detail::lexeme_length("def def abc");
        REQUIRE(result == 3);
    }

    TEST_CASE("lex function header") {
        static constexpr auto content = Content{R"(def func():
)"};
        static constexpr auto result = lex<content>();
        static constexpr auto expected =
                Lexemes{Keyword::def,
                        Identifier{"func"},
                        Operator::bracketleft,
                        Operator::bracketright,
                        Operator::semicolon,
                        Operator::linebreak};
        REQUIRE(result == expected);
    }

    TEST_CASE("is_identifier only letters") {
        static constexpr auto content = R"(func():)";
        static constexpr auto identifier = detail::is_identifier(content);
        REQUIRE(identifier->first.value == "func");
    }

    TEST_CASE("lex function") {
        static constexpr auto content = Content{R"(def func():
    return 123)"};
        static constexpr auto result = lex<content>();
        static constexpr auto expected =
                Lexemes{Keyword::def,
                        Identifier{"func"},
                        Operator::bracketleft,
                        Operator::bracketright,
                        Operator::semicolon,
                        Operator::linebreak,
                        Keyword::return_,
                        Literal{"123"}};
        REQUIRE(result == expected);
    }

}  // namespace

}  // namespace ctpy