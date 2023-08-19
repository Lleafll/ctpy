#pragma once

#include <array>
#include <tuple>
#include <variant>

namespace ctpy {

using Variable = std::variant<int, double>;

template<std::size_t variable_count>
struct Stack final {
    Variable return_value = {};
    std::array<Variable, variable_count> variables = {};

    Stack() = default;

    template<class... Ts>
    explicit constexpr Stack(auto return_value, Ts&&... ts) noexcept
        : return_value{return_value},
          variables{std::forward<Ts>(ts)...} {
    }

    constexpr bool
    operator==(Stack const&) const noexcept = default;
};

template<class... Ts>
Stack(auto, Ts&&...) -> Stack<sizeof...(Ts)>;

struct ReturnOperation final {
    std::size_t stack_index;

    constexpr void
    operator()(auto& stack) const noexcept {
        stack.return_value = stack.variables[stack_index];
    }

    constexpr bool
    operator==(ReturnOperation const&) const noexcept = default;
};

struct AssignOperation final {
    std::size_t from;
    std::size_t to;

    constexpr void
    operator()(auto& stack) const noexcept {
        stack.variables[to] = stack.variables[from];
    }

    constexpr bool
    operator==(AssignOperation const&) const noexcept = default;
};

struct AdditionOperation final {
    std::size_t lhs;
    std::size_t rhs;
    std::size_t target;

    constexpr void
    operator()(auto& stack) const noexcept {
        stack.variables[target] = std::visit(
                [](auto&& lhs_, auto&& rhs_) { return Variable{lhs_ + rhs_}; },
                stack.variables[lhs],
                stack.variables[rhs]);
    }

    constexpr bool
    operator==(AdditionOperation const&) const noexcept = default;
};

struct ConstantOperation final {
    std::size_t index;
    Variable value;

    constexpr void
    operator()(auto& stack) const noexcept {
        stack.variables[index] = value;
    }

    constexpr bool
    operator==(ConstantOperation const&) const noexcept = default;
};

using Operation =
        std::variant<AdditionOperation, AssignOperation, ConstantOperation, ReturnOperation>;

template<std::size_t stack_size, std::size_t parameters_count, std::size_t operation_count>
struct Function final {
    std::array<Operation, operation_count> operations;

    template<class... Operations>
    explicit constexpr Function(Operations const&... operations) noexcept
        : operations{Operation{operations}...} {
    }

    template<class... Parameters>
    constexpr auto
    operator()(Parameters&&... parameters) const noexcept {
        static_assert(
                sizeof...(parameters) == parameters_count, "Wrong number of parameters passed");
        auto stack = Stack<stack_size>{};
        [&stack]<std::size_t... I>(auto&& parameters, std::index_sequence<I...> const indexes) {
            ((std::get<I>(stack.variables) = std::get<I>(parameters)), ...);
        }(std::tuple{std::forward<Parameters>(parameters)...},
          std::make_index_sequence<parameters_count>{});
        for (auto const& operation: operations) {
            std::visit([&](auto&& operation_) { std::invoke(operation_, stack); }, operation);
        }
        return std::move(stack.return_value);
    }

    constexpr bool
    operator==(Function const&) const noexcept = default;
};

}  // namespace ctpy
