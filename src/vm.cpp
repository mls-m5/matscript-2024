#include "vm.h"
#include <memory>

namespace vm {

namespace {
std::shared_ptr<Map> createStd() {
    auto std = std::make_shared<Map>();

    (*std)[Token::from("abs")] = std::make_shared<Function>();

    return std;
}
} // namespace

const std::shared_ptr<Map> &getStd() {
    static auto module = createStd();

    return module;
}

void call(const Function &f, std::vector<Value> values, Context &context) {
    auto closure = Map{};

    auto newContext = Context{
        .closure = &closure,
        .parent = &context,
    };

    if (f.native) {
        f.native(newContext);
        return;
    }

    for (auto i : std::ranges::iota_view{
             0uz, std::min(values.size(), f.argumentNames.size())}) {
        closure[f.argumentNames.at(i)] = std::move(values.at(i));
    }

    call(f.body, newContext);
}

void call(const Section &section, Context &context) {
    for (auto &command : section.commands) {
        command->run(context);
    }
}

} // namespace vm
