// include/commands.hpp
#pragma once
#include "context.hpp"
#include <string>
#include <vector>

void dispatch_command(ExplorerContext& ctx,
                      const std::string& cmd,
                      const std::vector<std::string>& args);

