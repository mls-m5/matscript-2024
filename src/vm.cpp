#include "vm.h"
#include <fstream>
#include <iostream>
#include <memory>
#include <ranges>
#include <stdexcept>
#include <string>
#include <variant>

namespace vm {

namespace {

struct File : public OtherValueContent {
    std::ifstream file;
};

Value FileNext(Context &context) {
    auto &self = context.closure->at<Map>(t("this"));
    auto &file = self.at<File>(t("file"));

    std::string line;
    if (std::getline(file.file, line)) {
        return String{line};
    }
    return Bool{};
}

void addFileStuff(Map &std) {
    auto fileType = std::make_shared<Map>();

    (*fileType)[t("lines")] = std::make_shared<Function>(
        std::vector{t("path")},
        [](Context &context) -> Value { return Value{}; });

    std[t("File")] = std::move(fileType);

    std[t("open")] = std::make_shared<Function>(
        std::vector{t("path")}, [](Context &context) -> Value {
            auto &path = context.closure->at<String>(t("value"));

            std::cout << "opening file " << path.value << std::endl;

            auto file = std::make_shared<File>();
            file->file.open(path.value);

            auto map = std::make_shared<Map>();

            (*map)[t("file")] = file;

            return map;
        });
}

std::shared_ptr<Map> createStd() {
    auto std = std::make_shared<Map>();

    (*std)[t("abs")] = std::make_shared<Function>(
        std::vector{t("value")}, [](Context &context) -> Value {
            auto &value = context.closure->at(t("value"));
            if (std::holds_alternative<Float>(value.value)) {
                return Float{std::abs(value.as<Float>().value)};
            }
            else if (std::holds_alternative<Int>(value.value)) {
                return Int{std::abs(value.as<Int>().value)};
            }
            throw std::runtime_error{"could not run abs on this"};
        });

    (*std)[t("println")] = std::make_shared<Function>(
        std::vector{t("value")}, [](Context &context) {
            auto &value = context.closure->at(t("value"));
            if (value.is<Float>()) {
                std::cout << value.as<Float>().value << std::endl;
                return Value{};
            }
            else if (value.is<Int>()) {
                std::cout << value.as<Int>().value << std::endl;
                return Value{};
            }
            else if (value.is<String>()) {
                std::cout << value.as<String>().value << std::endl;
                return Value{};
            }

            throw std::runtime_error{"could not run print on this"};
        });

    (*std)[t("help")] = std::make_shared<Function>(
        std::vector{t("value")}, [](Context &context) -> Value {
            auto &value = context.closure->at(t("value"));
            if (value.is<Float>()) {
                std::cout << "[Float]" << std::endl;
                return {};
            }
            else if (value.is<Int>()) {
                std::cout << "[Int]" << std::endl;
                return {};
            }
            else if (value.is<String>()) {
                std::cout << "[String]" << std::endl;
                return {};
            }
            else if (value.is<Map>()) {
                auto &v = value.as<Map>();

                std::cout << "[Map]{\n";
                for (auto &v : v.values) {
                    std::cout << "  " << v.name << "\n";
                }
                std::cout << "}\n";
                return {};
            }
            else if (value.is<Function>()) {
                auto &v = value.as<Function>();
                std::cout << "[function]\n";
                return {};
            }

            throw std::runtime_error{"no help for this expression"};
        });

    addFileStuff(*std);

    return std;
}
} // namespace

const std::shared_ptr<Map> &getStd() {
    static auto module = createStd();

    return module;
}

Value call(const Function &f,
           std::vector<Value> values,
           Context &context,
           Value self) {
    auto closure = Map{};

    closure[t("this")] = self;

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

Value &Context::at(const Token &name) {
    if (auto f = closure->find(name)) {
        return *f;
    }

    if (parent) {
        return parent->at(name);
    }

    throw std::runtime_error{"can not find " + name.text};
}
} // namespace vm
