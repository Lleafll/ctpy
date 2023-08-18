#include "function.h"
#include <doctest/doctest.h>
#include <optional>
#include <string>

namespace ctpy {

namespace {
    TEST_CASE("ReturnOperation") {
        auto stack = Stack<std::optional<int>, int>{{}, {123}};
        ReturnOperation<0>{}(stack);
        REQUIRE(stack.return_value == 123);
    }

    TEST_CASE("AssignOperation") {
        auto stack = Stack<int, int, int>{{}, {123, 456}};
        AssignOperation<1, 0>{}(stack);
        REQUIRE(std::get<0>(stack.variables) == 456);
    }

    TEST_CASE("AdditionOperation") {
        auto stack = Stack<int, int, int, int>{{}, {2, 3, 0}};
        AdditionOperation<0, 1, 2>{}(stack);
        REQUIRE(stack.variables == std::tuple{2, 3, 5});
    }

    TEST_CASE("ConstantOperation") {
        auto stack = Stack<int, std::string>{{}, {}};
        ConstantOperation<0, 'a'>{}(stack);
        REQUIRE(stack.variables == std::tuple{"a"});
    }

    TEST_CASE("Function invokes operations") {
        using TestStack = Stack<std::pair<int, double>, int, double>;
        using Operation1 = decltype([](auto& stack) { std::get<0>(stack.variables) = 1; });
        using Operation2 = decltype([](auto& stack) {
            std::get<1>(stack.variables) = std::get<0>(stack.variables) + 1.3;
        });
        using Operation3 = decltype([](auto& stack) {
            stack.return_value = {std::get<0>(stack.variables), std::get<1>(stack.variables)};
        });
        auto const func = Function<TestStack, 0, Operation1, Operation2, Operation3>{};
        auto const [variable0, variable1] = func();
        REQUIRE(variable0 == 1);
        REQUIRE(variable1 == 2.3);
    }

    TEST_CASE("Function uses passed parameters") {
        using TestStack = Stack<std::pair<int, double>, int, double>;
        using Operation1 = decltype([](auto& stack) { std::get<0>(stack.variables) = 1; });
        using Operation2 = decltype([](auto& stack) {
            std::get<1>(stack.variables) = std::get<0>(stack.variables) + 1.3;
        });
        using Operation3 = decltype([](auto& stack) {
            stack.return_value = {std::get<0>(stack.variables), std::get<1>(stack.variables)};
        });
        auto const func = Function<TestStack, 0, Operation1, Operation2, Operation3>{};
        auto const [variable0, variable1] = func();
        REQUIRE(variable0 == 1);
        REQUIRE(variable1 == 2.3);
    }

    TEST_CASE("Function with some real operations") {
        auto const func = Function<
                Stack<int, int, int, int, int>,
                2,
                ConstantOperation<2, 5>,
                AdditionOperation<0, 1, 3>,
                AdditionOperation<2, 3, 3>,
                ReturnOperation<3>>{};
        REQUIRE(func(1, 2));
    }
}  // namespace

}  // namespace ctpy