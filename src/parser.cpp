#include "parser.h"
#include <algorithm>
#include <doctest/doctest.h>

namespace ctpy {

namespace {

    TEST_CASE("empty function") {
        static constexpr auto expected = Function<0, 0, 0>{};
        static constexpr auto lexemes =
                Lexemes{Keyword::def,
                        Identifier{"func"},
                        Operator::bracketleft,
                        Operator::bracketright,
                        Operator::semicolon,
                        Operator::linebreak};
        REQUIRE(parse<lexemes>() == expected);
    }

    TEST_CASE("function returning constant") {
        static constexpr auto expected =
                Function<1, 0, 2>{ConstantOperation{0, Variable{123}}, ReturnOperation{0}};
        static constexpr auto lexemes =
                Lexemes{Keyword::def,
                        Identifier{"func"},
                        Operator::bracketleft,
                        Operator::bracketright,
                        Operator::semicolon,
                        Operator::linebreak,
                        //
                        Keyword::return_,
                        Literal{"123"}};
        REQUIRE(parse<lexemes>() == expected);
    }

    TEST_CASE("parse_return_subexpression") {
        static constexpr auto lexemes = Lexemes{Literal{"123"}};
        static constexpr auto result = detail::parse_return_subexpression(lexemes.elements);
        REQUIRE(result.operation == Operation{ConstantOperation{0, Variable{123}}});
        REQUIRE(result.return_index == 0);
        REQUIRE(result.remaining_lexemes.empty());
    }

    TEST_CASE("calculate_parameters_count") {
        static constexpr auto result =
                detail::calculate_function_parameters(Lexemes{Keyword::def,
                                                              Identifier{"func"},
                                                              Operator::bracketleft,
                                                              Operator::bracketright,
                                                              Operator::semicolon}
                                                              .elements)
                        .parameters_count;
        REQUIRE(result == 0);
    }

    TEST_CASE("calculate_operation_count empty") {
        static constexpr auto result =
                detail::calculate_function_parameters(Lexemes{}.elements).operation_count;
        REQUIRE(result == 0);
    }

    TEST_CASE("calculate_operation_count non-empty") {
        static constexpr auto build_operations_mock =
                [](std::span<Lexeme const> const lexemes) -> detail::BuildOperationsReturn {
            return std::vector<Operation>(2);
        };
        REQUIRE(detail::calculate_function_parameters<build_operations_mock>(
                        Lexemes<2>{Identifier{"1"}, Identifier{"2"}}.elements)
                        .operation_count == 2);
    }

    TEST_CASE("build_operations") {
        static constexpr auto lexemes = std::array<Lexeme, 2>{Keyword::return_, Literal{"123"}};
        auto const expected =
                std::vector<Operation>{{ConstantOperation{0, 123}, ReturnOperation{0}}};
        auto const result = detail::build_operations(lexemes);
        REQUIRE(result == expected);
    }

    TEST_CASE("check_function_header") {
        static constexpr auto lexemes =
                Lexemes{Keyword::def,
                        Identifier{"func"},
                        Operator::bracketleft,
                        Operator::bracketright,
                        Operator::semicolon,
                        Operator::linebreak,
                        //
                        Keyword::return_,
                        Literal{"123"}};
        static auto constexpr expected = Lexemes{Keyword::return_, Literal{"123"}};
        static auto constexpr result = detail::check_function_header<lexemes>();
        REQUIRE(std::ranges::equal(result, expected.elements));
    }

}  // namespace

}  // namespace ctpy