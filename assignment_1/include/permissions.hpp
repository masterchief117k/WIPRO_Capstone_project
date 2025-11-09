// include/permissions.hpp
#pragma once
#include "context.hpp"
#include <string>

void show_perm(ExplorerContext& ctx, const std::string& path);
void set_perm(ExplorerContext& ctx, const std::string& path, const std::string& mode);

