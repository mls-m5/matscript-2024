#pragma once

#include <filesystem>
#include <ranges>
#include <vector>

struct Settings {
    std::filesystem::path path;

    Settings(int argc, char *argv[]) {
        auto args = std::vector<std::string>{argv + 1, argv + argc};

        for (auto i : std::ranges::iota_view{0uz, args.size()}) {
            auto arg = args.at(i);

            path = arg;
        }
    }
};
