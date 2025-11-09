// src/commands.cpp
#include "commands.hpp"
#include "fsops.hpp"
#include "permissions.hpp"
#include "search.hpp"
#include "util.hpp"

#include <iostream>
#include <filesystem>
#include <cstdlib>
#include <stdexcept>

namespace fs = std::filesystem;

static void print_help() {
    std::cout <<
    "Commands:\n"
    "  ls [path]\n"
    "  cd <path>        (cd with no args -> home; cd - -> previous dir)\n"
    "  pwd\n"
    "  back | forward\n"
    "  bookmark add <name> [path]\n"
    "  bookmark go <name>\n"
    "  bookmark rm <name>\n"
    "  bookmark list\n"
    "  touch <file>\n"
    "  mkdir <dir>\n"
    "  rm <path>\n"
    "  mv <src> <dst>\n"
    "  cp <src> <dst>\n"
    "  find <pattern> [--recursive] [--glob] [--regex]\n"
    "  perm <path>\n"
    "  chmod <path> <mode> (e.g., 755 or u+r,g-w)\n"
    "  showhidden on|off\n"
    "  sort name|size|time\n"
    "  help\n"
    "  exit\n";
}

// Expand ~ to HOME for a path string; returns unchanged if no leading ~
static std::string expand_home(const std::string& in) {
    if (in.empty()) return in;
    if (in[0] != '~') return in;
    const char* home = std::getenv("HOME");
    std::string homeStr = home ? home : "/";
    if (in.size() == 1) return homeStr;
    if (in[1] == '/') return homeStr + in.substr(1);
    // do not support ~user expansion here
    return in;
}

void dispatch_command(ExplorerContext& ctx,
                      const std::string& cmd,
                      const std::vector<std::string>& args) {
    try {
        if (cmd == "help") { print_help(); return; }

        else if (cmd == "pwd") { std::cout << ctx.cwd << "\n"; return; }

        else if (cmd == "ls") {
            std::string path = args.size() ? expand_home(args[0]) : ".";
            fs_list(ctx, path);
            return;
        }

        else if (cmd == "cd") {
            // cd with no args -> home
            if (args.empty()) {
                const char* home = std::getenv("HOME");
                if (!home) throw std::runtime_error("HOME not set");
                fs_cd(ctx, std::string(home));
                std::cout << ctx.cwd << "\n";
                return;
            }

            std::string raw = args[0];
            if (raw == "-") {
                // previous directory shortcut
                fs_back(ctx);
                std::cout << ctx.cwd << "\n";
                return;
            }

            // expand leading ~
            std::string path = expand_home(raw);
            fs_cd(ctx, path);
            std::cout << ctx.cwd << "\n";
            return;
        }

        else if (cmd == "back") { fs_back(ctx); std::cout << ctx.cwd << "\n"; return; }
        else if (cmd == "forward") { fs_forward(ctx); std::cout << ctx.cwd << "\n"; return; }

        else if (cmd == "bookmark") {
            if (args.empty()) throw std::runtime_error("bookmark requires subcommand add|go|rm|list");
            const std::string& op = args[0];

            if (op == "add") {
                if (args.size() < 2) throw std::runtime_error("bookmark add <name> [path]");
                std::string name = args[1];
                std::string p = (args.size() >= 3) ? args[2] : ".";
                p = expand_home(p);
                fs::path candidate = fs::path(p);
                if (candidate.is_relative()) candidate = ctx.cwd / candidate;
                std::error_code ec;
                fs::path canonical = fs::weakly_canonical(candidate, ec);
                if (ec) canonical = fs::absolute(candidate);
                ctx.bookmarks[name] = canonical;
                std::cout << "Added bookmark '" << name << "' -> " << canonical << "\n";
                return;
            }
            else if (op == "go") {
                if (args.size() != 2) throw std::runtime_error("bookmark go <name>");
                std::string name = args[1];
                auto it = ctx.bookmarks.find(name);
                if (it == ctx.bookmarks.end()) { std::cerr << "bookmark not found: " << name << "\n"; return; }
                fs_cd(ctx, it->second.string());
                std::cout << ctx.cwd << "\n";
                return;
            }
            else if (op == "rm") {
                if (args.size() != 2) throw std::runtime_error("bookmark rm <name>");
                std::string name = args[1];
                size_t erased = ctx.bookmarks.erase(name);
                if (erased) std::cout << "Removed bookmark: " << name << "\n";
                else std::cerr << "bookmark not found: " << name << "\n";
                return;
            }
            else if (op == "list") {
                if (ctx.bookmarks.empty()) { std::cout << "No bookmarks\n"; return; }
                for (const auto& kv : ctx.bookmarks) {
                    std::cout << kv.first << " -> " << kv.second << "\n";
                }
                return;
            }
            else {
                throw std::runtime_error("bookmark subcommand must be add|go|rm|list");
            }
        }

        else if (cmd == "touch") { if (args.size()!=1) throw std::runtime_error("touch <file>"); fs_touch(ctx, expand_home(args[0])); return; }
        else if (cmd == "mkdir") { if (args.size()!=1) throw std::runtime_error("mkdir <dir>"); fs_mkdir(ctx, expand_home(args[0])); return; }
        else if (cmd == "rm") { if (args.size()!=1) throw std::runtime_error("rm <path>"); fs_rm(ctx, expand_home(args[0])); return; }
        else if (cmd == "mv") { if (args.size()!=2) throw std::runtime_error("mv <src> <dst>"); fs_mv(ctx, expand_home(args[0]), expand_home(args[1])); return; }
        else if (cmd == "cp") { if (args.size()!=2) throw std::runtime_error("cp <src> <dst>"); fs_cp(ctx, expand_home(args[0]), expand_home(args[1])); return; }

        else if (cmd == "find") { search_run(ctx, args); return; }

        else if (cmd == "perm") { if (args.size()!=1) throw std::runtime_error("perm <path>"); show_perm(ctx, expand_home(args[0])); return; }
        else if (cmd == "chmod") { if (args.size()!=2) throw std::runtime_error("chmod <path> <mode>"); set_perm(ctx, expand_home(args[0]), args[1]); return; }

        else if (cmd == "showhidden") { if (args.size()!=1) throw std::runtime_error("showhidden on|off"); ctx.show_hidden = (args[0]=="on"); return; }

        else if (cmd == "sort") {
            if (args.size()!=1) throw std::runtime_error("sort name|size|time");
            if (args[0]=="name") ctx.sort = ExplorerContext::SortMode::name;
            else if (args[0]=="size") ctx.sort = ExplorerContext::SortMode::size;
            else if (args[0]=="time") ctx.sort = ExplorerContext::SortMode::time;
            else throw std::runtime_error("invalid sort key");
            return;
        }

        else {
            std::cerr << "Unknown command: " << cmd << "\n";
        }
    } catch (const std::exception& e) {
        // rethrow to be handled by main loop or print here depending on your preference
        throw;
    }
}

