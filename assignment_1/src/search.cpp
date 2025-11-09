// src/search.cpp
#include "search.hpp"
#include <filesystem>
#include <iostream>
#include <regex>
namespace fs = std::filesystem;

static bool match_glob(const std::string& name, const std::string& pat) {
    // Very light glob: '*' and '?'
    size_t n=0, p=0; size_t star=std::string::npos, match=0;
    while (n < name.size()) {
        if (p < pat.size() && (pat[p] == '?' || pat[p] == name[n])) { ++n; ++p; }
        else if (p < pat.size() && pat[p] == '*') { star = p++; match = n; }
        else if (star != std::string::npos) { p = star + 1; n = ++match; }
        else return false;
    }
    while (p < pat.size() && pat[p] == '*') ++p;
    return p == pat.size();
}

void search_run(ExplorerContext& ctx, const std::vector<std::string>& args) {
    if (args.empty()) { std::cerr << "find <pattern> [--recursive] [--regex] [--glob]\n"; return; }
    std::string pattern = args[0];
    bool recursive=false, use_regex=false, use_glob=false;
    for (size_t i=1;i<args.size();++i) {
        if (args[i]=="--recursive") recursive=true;
        else if (args[i]=="--regex") use_regex=true;
        else if (args[i]=="--glob") use_glob=true;
    }

    if (use_regex && use_glob) { std::cerr << "Choose either --regex or --glob\n"; return; }

    if (!recursive) {
        for (const auto& e : fs::directory_iterator(ctx.cwd)) {
            auto name = e.path().filename().string();
            if (!ctx.show_hidden && !name.empty() && name[0]=='.') continue;
            bool ok = false;
            if (use_regex) ok = std::regex_search(name, std::regex(pattern));
            else if (use_glob) ok = match_glob(name, pattern);
            else ok = (name.find(pattern) != std::string::npos);
            if (ok) std::cout << e.path() << "\n";
        }
    } else {
        for (const auto& e : fs::recursive_directory_iterator(ctx.cwd)) {
            auto name = e.path().filename().string();
            if (!ctx.show_hidden && !name.empty() && name[0]=='.') continue;
            bool ok = false;
            if (use_regex) ok = std::regex_search(name, std::regex(pattern));
            else if (use_glob) ok = match_glob(name, pattern);
            else ok = (name.find(pattern) != std::string::npos);
            if (ok) std::cout << e.path() << "\n";
        }
    }
}

