#include <ctpy/parser.h>

int
main() {
    static constexpr auto python_code = ctpy::Content{R"(def func():
    return 123)"};
    static constexpr auto lexed = ctpy::lex<python_code>();
    static constexpr auto func = ctpy::parse<lexed>();
    static constexpr auto result = func();
    return std::get<int>(result);
}