#pragma once

#include <array>
#include <optional>
#include <string_view>
#include <type_traits>
#include <variant>

namespace ctpy {

enum class Keyword { def };

enum class Operator { plus };

struct Identifier final {
    std::string_view value;

    constexpr bool
    operator==(Identifier const&) const noexcept = default;
};

using Lexeme = std::variant<Keyword, Operator, Identifier>;

template<std::size_t N>
struct Lexemes final {
    std::array<Lexeme, N> elements;

    template<class... Ts>
    constexpr explicit Lexemes(Ts&&... ts) : elements{std::forward<Ts>(ts)...} {
    }

    constexpr explicit Lexemes(std::array<Lexeme, N> const& elements) : elements{elements} {
    }

    constexpr bool
    operator==(Lexemes const&) const noexcept = default;
};

template<class... Ts>
Lexemes(Ts...) -> Lexemes<sizeof...(Ts)>;

struct Content final {
    char const* data;
    std::size_t size;

    constexpr Content(std::string_view const string) : data{string.data()}, size{string.size()} {
    }

    [[nodiscard]] constexpr bool
    empty() const noexcept {
        return size == 0U;
    }

    constexpr operator std::string_view() const noexcept {
        return std::string_view{data, size};
    }
};

namespace detail {

    inline constexpr auto is_keyword = [](std::string_view const content) constexpr noexcept
            -> std::optional<std::pair<Keyword, std::string_view>> {
        constexpr auto def = std::string_view{"def"};
        if (content.starts_with(def)) {
            return std::optional<std::pair<Keyword, std::string_view>>{
                    std::in_place, Keyword::def, content.substr(def.size())};
        }
        return std::nullopt;
    };

    inline constexpr auto is_operator = [](std::string_view const content) constexpr noexcept
            -> std::optional<std::pair<Operator, std::string_view>> {
        if (content.starts_with('+')) {
            return std::optional<std::pair<Operator, std::string_view>>{
                    std::in_place, Operator::plus, content.substr(1)};
        }
        return std::nullopt;
    };

    inline constexpr auto is_identifier = [](std::string_view const content) constexpr noexcept
            -> std::optional<std::pair<Identifier, std::string_view>> {
        auto const end = std::min(content.find(' '), content.size());
        return std::optional<std::pair<Identifier, std::string_view>>{
                std::in_place, Identifier{content.substr(0, end)}, content.substr(end)};
    };

    template<
            auto is_keyword_func = is_keyword,
            auto is_operator_func = is_operator,
            auto is_identifier_func = is_identifier>
        requires requires {
            {
                is_keyword_func(std::string_view{})
            } -> std::same_as<std::optional<std::pair<Keyword, std::string_view>>>;
            {
                is_operator_func(std::string_view{})
            } -> std::same_as<std::optional<std::pair<Operator, std::string_view>>>;
            {
                is_identifier_func(std::string_view{})
            } -> std::same_as<std::optional<std::pair<Identifier, std::string_view>>>;
        }
    inline constexpr auto is_lexeme = [](std::string_view strings) constexpr noexcept
            -> std::optional<std::pair<Lexeme, std::string_view>> {
        strings.remove_prefix(std::min(strings.find_first_not_of(' '), strings.size()));
        auto result = std::optional<std::pair<Lexeme, std::string_view>>{is_keyword_func(strings)};
        if (not result.has_value()) {
            result = is_operator_func(strings);
        }
        if (not result.has_value()) {
            result = is_identifier_func(strings);
        }
        return result;
    };

    constexpr std::size_t
    lexeme_length(std::string_view content) noexcept {
        auto length = 0ULL;
        while (not content.empty()) {
            auto const lexeme = is_lexeme<>(content);
            if (not lexeme.has_value()) {
                break;
            }
            ++length;
            content = lexeme->second;
        }
        return length;
    }

}  // namespace detail

template<Content const& content, auto is_lexeme_func = detail::is_lexeme<>>
constexpr auto
lex() noexcept {
    constexpr auto size = detail::lexeme_length(content);
    auto content_view = std::string_view{content};
    auto elements = std::array<Lexeme, size>{};
    for (auto& element: elements) {
        auto const is_lexeme_result = is_lexeme_func(content_view);
        if (not is_lexeme_result.has_value()) {
            abort();
        }
        element = is_lexeme_result->first;
        content_view = is_lexeme_result->second;
    }
    return Lexemes<size>{elements};
}

}  // namespace ctpy