// src/main.cpp
#include <iostream>
#include <sstream>
#include <vector>
#include "context.hpp"
#include "commands.hpp"

int main() {
    ExplorerContext ctx;
    std::string line;

    std::cout << "fx> " << ctx.cwd << "\n";
    while (std::cout << "> " && std::getline(std::cin, line)) {
        std::istringstream iss(line);
        std::string cmd; iss >> cmd;
        std::vector<std::string> args;
        for (std::string a; iss >> a;) args.push_back(a);

        if (cmd.empty()) continue;
        if (cmd == "exit" || cmd == "quit") break;

        try {
            dispatch_command(ctx, cmd, args);
        } catch (const std::exception& e) {
            std::cerr << "Error: " << e.what() << "\n";
        }
    }
    return 0;
}

