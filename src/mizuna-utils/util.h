#pragma once

#include <filesystem>
#include <hk/ValueOrResult.h>
#include <string>

namespace fs = std::filesystem;

hk::ValueOrResult<u32> parse_u32(const std::string& arg, const std::string& argName, const std::string& errorStr);

hk::ValueOrResult<fs::path> parse_path_dir(
	const std::string& arg, const std::string& argName, const std::string& errorStr
);

hk::ValueOrResult<fs::path> parse_path_file(
	const std::string& arg, const std::string& argName, const std::string& errorStr
);

hk::Result check_args_len(s32 argIdx, s32 argc, s32 numRequired, const std::string& errorStr);

s32 get_args_num(s32 argIdx, s32 argc);
