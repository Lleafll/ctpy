#pragma once

#include <tuple>

namespace ctpy {

template<class Return, class... Variables>
struct Stack final {
    Return return_value;
    std::tuple<Variables...> variables;
};

template<std::size_t stack_index>
struct ReturnOperation final {
    constexpr void
    operator()(auto& stack) const noexcept {
        stack.return_value = std::get<stack_index>(stack.variables);
    }
};

template<std::size_t from, std::size_t to>
struct AssignOperation final {
    constexpr void
    operator()(auto& stack) const noexcept {
        std::get<to>(stack.variables) = std::get<from>(stack.variables);
    }
};

template<std::size_t lhs, std::size_t rhs, std::size_t target>
struct AdditionOperation final {
    constexpr void
    operator()(auto& stack) const noexcept {
        std::get<target>(stack.variables) =
                std::get<lhs>(stack.variables) + std::get<rhs>(stack.variables);
    }
};

template<std::size_t index, auto value>
struct ConstantOperation final {
    constexpr void
    operator()(auto& stack) const noexcept {
        std::get<index>(stack.variables) = value;
    }
};

template<class Stack, std::size_t parameters_count, class... Operations>
struct Function final {
    template<class... Parameters>
    constexpr auto
    operator()(Parameters&&... parameters) const noexcept {
        static_assert(
                sizeof...(parameters) == parameters_count, "Wrong number of parameters passed");
        auto stack = Stack{};
        [&stack]<std::size_t... I>(auto&& parameters, std::index_sequence<I...> const indexes) {
            ((std::get<I>(stack.variables) = std::get<I>(parameters)), ...);
        }(std::tuple{std::forward<Parameters>(parameters)...},
          std::make_index_sequence<parameters_count>{});
        (std::invoke(Operations{}, stack), ...);
        return std::move(stack.return_value);
    }
};

}  // namespace ctpy
