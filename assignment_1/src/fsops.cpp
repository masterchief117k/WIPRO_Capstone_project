// src/fsops.cpp
#include "fsops.hpp"
#include "util.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
namespace fs = std::filesystem;

static fs::path resolve(const fs::path& cwd, const std::string& in) {
    fs::path p = fs::path(in);
    if (p.is_relative()) p = fs::weakly_canonical(cwd / p);
    else p = fs::weakly_canonical(p);
    return p;
}

void fs_list(ExplorerContext& ctx, const std::string& path) {
    fs::path p = resolve(ctx.cwd, path);
    if (!fs::exists(p)) { std::cerr << "No such path: " << p << "\n"; return; }
    if (!fs::is_directory(p)) { std::cerr << "Not a directory: " << p << "\n"; return; }

    std::vector<fs::directory_entry> entries;
    for (const auto& e : fs::directory_iterator(p)) {
        if (!ctx.show_hidden) {
            auto name = e.path().filename().string();
            if (!name.empty() && name[0]=='.') continue;
        }
        entries.push_back(e);
    }

    auto cmp_name = [](auto& a, auto& b){ return a.path().filename().string() < b.path().filename().string(); };
    auto cmp_size = [](auto& a, auto& b){
        auto sa = a.is_regular_file()? fs::file_size(a.path()) : 0;
        auto sb = b.is_regular_file()? fs::file_size(b.path()) : 0;
        return sa < sb;
    };
    auto cmp_time = [](auto& a, auto& b){
        auto ta = fs::last_write_time(a.path());
        auto tb = fs::last_write_time(b.path());
        return ta < tb;
    };

    switch (ctx.sort) {
        case ExplorerContext::SortMode::name: std::sort(entries.begin(), entries.end(), cmp_name); break;
        case ExplorerContext::SortMode::size: std::sort(entries.begin(), entries.end(), cmp_size); break;
        case ExplorerContext::SortMode::time: std::sort(entries.begin(), entries.end(), cmp_time); break;
    }

    for (const auto& e : entries) {
        auto name = e.path().filename().string();
        bool is_dir = e.is_directory();
        bool is_link = fs::is_symlink(e.path());
        std::string type = is_dir ? "dir" : (e.is_regular_file() ? "file" : "other");
        std::cout << (is_link ? "l" : "-") << " " << type << "  " << name;
        if (e.is_regular_file()) std::cout << "  " << fs::file_size(e.path()) << "B";
        std::cout << "\n";
    }
}

void fs_cd(ExplorerContext& ctx, const std::string& path) {
    fs::path p = resolve(ctx.cwd, path);
    if (!fs::exists(p)) throw std::runtime_error("cd: path does not exist");
    if (!fs::is_directory(p)) throw std::runtime_error("cd: not a directory");
    ctx.back_stack.push_back(ctx.cwd);
    ctx.cwd = p;
    ctx.forward_stack.clear();
}

void fs_back(ExplorerContext& ctx) {
    if (ctx.back_stack.empty()) { std::cerr << "No back history\n"; return; }
    ctx.forward_stack.push_back(ctx.cwd);
    ctx.cwd = ctx.back_stack.back();
    ctx.back_stack.pop_back();
}

void fs_forward(ExplorerContext& ctx) {
    if (ctx.forward_stack.empty()) { std::cerr << "No forward history\n"; return; }
    ctx.back_stack.push_back(ctx.cwd);
    ctx.cwd = ctx.forward_stack.back();
    ctx.forward_stack.pop_back();
}

void fs_touch(ExplorerContext& ctx, const std::string& file) {
    fs::path p = resolve(ctx.cwd, file);
    std::ofstream ofs(p, std::ios::app);
    if (!ofs) throw std::runtime_error("touch: cannot create file");
}

void fs_mkdir(ExplorerContext& ctx, const std::string& dir) {
    fs::path p = resolve(ctx.cwd, dir);
    if (!fs::create_directories(p)) throw std::runtime_error("mkdir: cannot create");
}

void fs_rm(ExplorerContext& ctx, const std::string& path) {
    fs::path p = resolve(ctx.cwd, path);
    if (!fs::exists(p)) throw std::runtime_error("rm: path not found");
    std::error_code ec;
    if (fs::is_directory(p)) {
        fs::remove_all(p, ec);
        if (ec) throw std::runtime_error("rm: cannot remove directory");
    } else {
        fs::remove(p, ec);
        if (ec) throw std::runtime_error("rm: cannot remove file");
    }
}

void fs_mv(ExplorerContext& ctx, const std::string& src, const std::string& dst) {
    fs::path s = resolve(ctx.cwd, src);
    fs::path d = resolve(ctx.cwd, dst);
    if (!fs::exists(s)) throw std::runtime_error("mv: src not found");
    std::error_code ec;
    fs::rename(s, d, ec);
    if (ec) throw std::runtime_error("mv: rename failed (permissions or cross-device?)");
}

void fs_cp(ExplorerContext& ctx, const std::string& src, const std::string& dst) {
    fs::path s = resolve(ctx.cwd, src);
    fs::path d = resolve(ctx.cwd, dst);
    if (!fs::exists(s)) throw std::runtime_error("cp: src not found");
    std::error_code ec;
    if (fs::is_directory(s)) {
        fs::copy(s, d, fs::copy_options::recursive | fs::copy_options::directories_only | fs::copy_options::overwrite_existing, ec);
    } else {
        fs::copy_file(s, d, fs::copy_options::overwrite_existing, ec);
    }
    if (ec) throw std::runtime_error("cp: copy failed");
}

