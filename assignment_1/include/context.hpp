// include/context.hpp
#pragma once
#include <filesystem>
#include <unordered_map>
#include <vector>
#include <string>

struct ExplorerContext {
    std::filesystem::path cwd = std::filesystem::current_path();
    std::vector<std::filesystem::path> back_stack;
    std::vector<std::filesystem::path> forward_stack;
    std::unordered_map<std::string, std::filesystem::path> bookmarks;
    bool show_hidden = false;
    enum class SortMode { name, size, time } sort = SortMode::name;
};

