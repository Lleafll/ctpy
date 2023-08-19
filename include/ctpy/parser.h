#pragma once

#include "function.h"
#include "lexer.h"
#include <charconv>
#include <span>
#include <stdexcept>
#include <vector>

namespace ctpy {

namespace detail {

    // TODO: do more than integer parsing
    constexpr Variable
    parse_literal_to_variable(Literal const& literal) noexcept {
        auto value = 0;
        std::from_chars(literal.value.data(), literal.value.data() + literal.value.size(), value);
        return Variable{value};
    }

    struct ParseReturnSubExpressionReturn final {
        Operation operation;
        std::size_t return_index;
        std::span<Lexeme const> remaining_lexemes;
    };
    inline constexpr auto parse_return_subexpression =
            [](std::span<Lexeme const> const lexemes) constexpr noexcept
            -> ParseReturnSubExpressionReturn {
        return std::visit(
                [&]<class T>(T const& first_lexeme) -> ParseReturnSubExpressionReturn {
                    if constexpr (std::is_same_v<T, Literal>) {
                        return ParseReturnSubExpressionReturn{
                                ConstantOperation{0, parse_literal_to_variable(first_lexeme)},
                                0,
                                lexemes.subspan<1>()};
                    } else {
                        throw "Could not parse return sub-expression";  // NOLINT(*-exception-baseclass)
                    }
                },
                lexemes.front());
    };

    using BuildOperationsReturn = std::vector<Operation>;
    inline constexpr auto build_operations =
            [](std::span<Lexeme const> lexemes) constexpr noexcept -> BuildOperationsReturn {
        auto operations = BuildOperationsReturn{};
        while (not lexemes.empty()) {
            std::visit(
                    [&]<class T>(T const& first_lexeme) {
                        if constexpr (std::is_same_v<T, Keyword>) {
                            if (first_lexeme == Keyword::return_) {
                                auto next_operations =
                                        parse_return_subexpression(lexemes.subspan<1>());
                                operations.emplace_back(next_operations.operation);
                                operations.emplace_back(
                                        ReturnOperation{next_operations.return_index});
                                lexemes = next_operations.remaining_lexemes;
                            } else {
                                if consteval {
                                    lexemes = {};
                                } else {
                                    abort();
                                }
                            }
                        } else {
                            if consteval {
                                lexemes = {};
                            } else {
                                abort();
                            }
                        }
                    },
                    lexemes.front());
        }
        return operations;
    };

    struct FunctionParameters final {
        std::size_t stack_size;
        std::size_t parameters_count;
        std::size_t operation_count;
    };

    constexpr std::size_t
    determine_stack_size(std::span<Operation const> const operations) noexcept {
        auto stack_size = std::size_t{0};
        for (auto const& operation: operations) {
            stack_size = std::max(
                    stack_size,
                    std::visit(
                            []<class T>(T const& operation) -> std::size_t {
                                if constexpr (std::is_same_v<T, AdditionOperation>) {
                                    return std::max(
                                                   operation.lhs,
                                                   std::max(operation.rhs, operation.target)) +
                                           1;
                                } else if constexpr (std::is_same_v<T, AssignOperation>) {
                                    return std::max(operation.from, operation.to) + 1;
                                } else if constexpr (std::is_same_v<T, ConstantOperation>) {
                                    return operation.index + 1;
                                } else if constexpr (std::is_same_v<T, ReturnOperation>) {
                                    return operation.stack_index + 1;
                                }
                            },
                            operation));
        }
        return stack_size;
    }

    template<auto build_operations_func = build_operations>
    constexpr FunctionParameters
    calculate_function_parameters(std::span<Lexeme const> const lexemes) noexcept {
        auto const operations = build_operations_func(lexemes);
        return {determine_stack_size(operations), 0U, operations.size()};
    }

    template<auto const& lexemes>
    constexpr std::span<Lexeme const>
    check_function_header() noexcept {
        constexpr auto lexemes_view = std::span<Lexeme const>{lexemes.elements};
        static_assert(lexemes_view[0] == Lexeme{Keyword::def});
        static_assert(std::holds_alternative<Identifier>(lexemes_view[1]));
        static_assert(lexemes_view[2] == Lexeme{Operator::bracketleft});
        static_assert(lexemes_view[3] == Lexeme{Operator::bracketright});
        static_assert(lexemes_view[4] == Lexeme{Operator::semicolon});
        static_assert(lexemes_view[5] == Lexeme{Operator::linebreak});
        return lexemes_view.subspan<6>();
    }

}  // namespace detail

template<auto const& lexemes>
constexpr auto
parse() noexcept {
    constexpr auto lexemes_view = detail::check_function_header<lexemes>();
    constexpr auto function_parameters = detail::calculate_function_parameters<>(lexemes_view);
    auto function = Function<
            function_parameters.stack_size,
            function_parameters.parameters_count,
            function_parameters.operation_count>{};
    auto const operations = detail::build_operations(lexemes_view);
    std::ranges::copy(operations, function.operations.begin());
    return function;
}

}  // namespace ctpy