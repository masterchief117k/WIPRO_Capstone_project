// src/permissions.cpp
#include "permissions.hpp"
#include <filesystem>
#include <iostream>
#include <sys/stat.h>

namespace fs = std::filesystem;

static std::string rwx(mode_t m, mode_t r, mode_t w, mode_t x) {
    std::string s;
    s += (m & r) ? 'r' : '-';
    s += (m & w) ? 'w' : '-';
    s += (m & x) ? 'x' : '-';
    return s;
}

void show_perm(ExplorerContext&, const std::string& path) {
    fs::path p(path);
    if (p.is_relative()) p = fs::weakly_canonical(fs::current_path() / p);

    struct stat st{};
    if (stat(p.c_str(), &st) != 0) { std::perror("stat"); return; }

    auto u = rwx(st.st_mode, S_IRUSR, S_IWUSR, S_IXUSR);
    auto g = rwx(st.st_mode, S_IRGRP, S_IWGRP, S_IXGRP);
    auto o = rwx(st.st_mode, S_IROTH, S_IWOTH, S_IXOTH);

    mode_t oct = ( (st.st_mode & S_IRUSR ? 4 : 0) + (st.st_mode & S_IWUSR ? 2 : 0) + (st.st_mode & S_IXUSR ? 1 : 0) ) * 100
               + ( (st.st_mode & S_IRGRP ? 4 : 0) + (st.st_mode & S_IWGRP ? 2 : 0) + (st.st_mode & S_IXGRP ? 1 : 0) ) * 10
               + ( (st.st_mode & S_IROTH ? 4 : 0) + (st.st_mode & S_IWOTH ? 2 : 0) + (st.st_mode & S_IXOTH ? 1 : 0) );

    std::cout << p << "\n"
              << "  user: " << u << "\n"
              << "  group: " << g << "\n"
              << "  other: " << o << "\n"
              << "  octal: " << oct << "\n";
}

static mode_t parse_octal(const std::string& s) {
    if (s.size()<3 || s.size()>4) throw std::runtime_error("octal must be 3 or 4 digits");
    mode_t val = 0;
    for (char c : s) {
        if (c<'0' || c>'7') throw std::runtime_error("invalid octal digit");
        val = (val<<3) + (c - '0');
    }
    return val;
}

void set_perm(ExplorerContext&, const std::string& path, const std::string& mode) {
    fs::path p(path);
    if (p.is_relative()) p = fs::weakly_canonical(fs::current_path() / p);

    struct stat st{};
    if (stat(p.c_str(), &st) != 0) { std::perror("stat"); return; }

    mode_t new_mode = st.st_mode;

    if (std::all_of(mode.begin(), mode.end(), [](char c){ return std::isdigit(c); })) {
        // Octal (e.g., 755 or 0755)
        mode_t oct = parse_octal(mode);
        // Apply only permission bits (ignore file type/high bits)
        new_mode &= ~0777;
        new_mode |= (oct & 0777);
    } else {
        // Symbolic (e.g., u+r,g-w,o=rx)
        // Parse comma-separated clauses
        size_t start = 0;
        while (start < mode.size()) {
            size_t end = mode.find(',', start);
            if (end == std::string::npos) end = mode.size();
            std::string clause = mode.substr(start, end - start);

            // targets
            int targets = 0; // 1=u,2=g,4=o; 7=ugo
            size_t i = 0;
            while (i < clause.size() && (clause[i]=='u'||clause[i]=='g'||clause[i]=='o'||clause[i]=='a')) {
                if (clause[i]=='u') targets |= 1;
                else if (clause[i]=='g') targets |= 2;
                else if (clause[i]=='o') targets |= 4;
                else if (clause[i]=='a') targets |= 7;
                ++i;
            }
            if (targets==0) targets = 7; // default: a

            if (i>=clause.size() || (clause[i] != '+' && clause[i] != '-' && clause[i] != '=')) throw std::runtime_error("invalid symbolic mode");
            char op = clause[i++];

            int rwx = 0; // 1=x,2=w,4=r
            for (; i < clause.size(); ++i) {
                char c = clause[i];
                if (c=='r') rwx |= 4;
                else if (c=='w') rwx |= 2;
                else if (c=='x') rwx |= 1;
                else throw std::runtime_error("invalid permission char");
            }

            auto apply = [&](int targetBits, mode_t r, mode_t w, mode_t x){
                mode_t mask = 0;
                if (rwx & 4) mask |= r;
                if (rwx & 2) mask |= w;
                if (rwx & 1) mask |= x;
                if (op=='=') {
                    // clear then set
                    new_mode &= ~(r|w|x);
                    new_mode |= mask;
                } else if (op=='+') {
                    new_mode |= mask;
                } else if (op=='-') {
                    new_mode &= ~mask;
                }
            };

            if (targets & 1) apply(1, S_IRUSR, S_IWUSR, S_IXUSR);
            if (targets & 2) apply(2, S_IRGRP, S_IWGRP, S_IXGRP);
            if (targets & 4) apply(4, S_IROTH, S_IWOTH, S_IXOTH);

            start = end + (end < mode.size() ? 1 : 0);
        }
    }

    if (chmod(p.c_str(), new_mode) != 0) { std::perror("chmod"); }
}

