#include "function.h"
#include <doctest/doctest.h>
#include <optional>
#include <string>

namespace ctpy {

namespace {
    TEST_CASE("ReturnOperation") {
        auto stack = Stack{0, 123};
        ReturnOperation{0}(stack);
        REQUIRE(stack.return_value == Variable{123});
    }

    TEST_CASE("AssignOperation") {
        auto stack = Stack{0, 123, 456};
        AssignOperation{1, 0}(stack);
        REQUIRE(stack.variables[0] == Variable{456});
    }

    TEST_CASE("AdditionOperation") {
        auto stack = Stack{0, 2, 3, 0};
        AdditionOperation{0, 1, 2}(stack);
        REQUIRE(stack.variables == std::array{Variable{2}, Variable{3}, Variable{5}});
    }

    TEST_CASE("ConstantOperation") {
        auto stack = Stack{0, 1.23};
        ConstantOperation{0, 1.23}(stack);
        REQUIRE(stack.variables == std::array{Variable{1.23}});
    }

    TEST_CASE("Function with some real operations") {
        auto const func = Function<4, 2, 4>{
                ConstantOperation{2, 5}, // [2] = 5
                AdditionOperation{0, 1, 3}, // [3] = [0] + [1] (parameters)
                AdditionOperation{2, 3, 3}, // [3] = [2] + [3]
                ReturnOperation{3}};
        REQUIRE(func(1, 2) == Variable{8});
    }
}  // namespace

}  // namespace ctpy