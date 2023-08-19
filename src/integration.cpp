#include "parser.h"
#include <doctest/doctest.h>

namespace {

TEST_CASE("simple immediate return") {
    static constexpr auto python_code = ctpy::Content{R"(def func():
    return 123)"};
    static constexpr auto lexed = ctpy::lex<python_code>();
    static constexpr auto func = ctpy::parse<lexed>();
    static constexpr auto result = func();
    REQUIRE(std::get<int>(result) == 123);
}

}  // namespace