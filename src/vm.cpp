#include "vm.h"
#include <iostream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <variant>

namespace vm {

namespace {
std::shared_ptr<Map> createStd() {
    auto std = std::make_shared<Map>();

    (*std)[Token::from("abs")] = std::make_shared<Function>(
        std::vector{Token::from("value")}, [](Context &context) {
            auto &value = context.closure->at(Token::from("value"));
            if (std::holds_alternative<Float>(value.value)) {
                return Value{
                    .value = Float{std::abs(value.as<Float>().value)},
                };
            }
            else if (std::holds_alternative<Int>(value.value)) {
                return Value{
                    .value = Int{std::abs(value.as<Int>().value)},
                };
            }
            throw std::runtime_error{"could not run abs on this"};
        });

    (*std)[Token::from("println")] = std::make_shared<Function>(
        std::vector{Token::from("value")}, [](Context &context) {
            auto &value = context.closure->at(Token::from("value"));
            if (std::holds_alternative<Float>(value.value)) {
                std::cout << value.as<Float>().value << std::endl;
                return Value{};
            }
            else if (std::holds_alternative<Int>(value.value)) {
                std::cout << value.as<Int>().value << std::endl;
                return Value{};
            }
            else if (std::holds_alternative<String>(value.value)) {
                std::cout << value.as<String>().value << std::endl;
                return Value{};
            }

            throw std::runtime_error{"could not run print on this"};
        });

    return std;
}
} // namespace

const std::shared_ptr<Map> &getStd() {
    static auto module = createStd();

    return module;
}

Value call(const Function &f, std::vector<Value> values, Context &context) {
    auto closure = Map{};

    auto newContext = Context{
        .closure = &closure,
        .parent = &context,
    };

    for (auto i : std::ranges::iota_view{
             0uz, std::min(values.size(), f.argumentNames.size())}) {
        closure[f.argumentNames.at(i)] = std::move(values.at(i));
    }

    if (f.native) {
        return f.native(newContext);
    }

    return call(f.body, newContext);
}

Value call(const Section &section, Context &context) {
    Value ret;
    for (auto &command : section.commands) {
        ret = command->run(context);
    }
    return ret;
}

} // namespace vm
