// include/fsops.hpp
#pragma once
#include "context.hpp"
#include <string>

void fs_list(ExplorerContext& ctx, const std::string& path);
void fs_cd(ExplorerContext& ctx, const std::string& path);
void fs_back(ExplorerContext& ctx);
void fs_forward(ExplorerContext& ctx);
void fs_touch(ExplorerContext& ctx, const std::string& file);
void fs_mkdir(ExplorerContext& ctx, const std::string& dir);
void fs_rm(ExplorerContext& ctx, const std::string& path);
void fs_mv(ExplorerContext& ctx, const std::string& src, const std::string& dst);
void fs_cp(ExplorerContext& ctx, const std::string& src, const std::string& dst);

