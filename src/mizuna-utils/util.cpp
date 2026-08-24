#include "util.h"

#include <cctype>

#include "mizuna/results.h"

hk::ValueOrResult<u32> parse_u32(const std::string& arg, const std::string& argName, const std::string& errorStr) {
	if (arg.starts_with("0x")) {
		for (auto it = arg.begin() + 2; *it; it++) {
			if (!std::isxdigit(*it)) {
				fprintf(stderr, "error: failed to parse hexadecimal number for argument `%s`\n\n", argName.c_str());
				std::fputs(errorStr.c_str(), stderr);
				return hk::ResultInvalidArgument();
			}
		}
		return std::stoul(arg, 0, 16);
	} else {
		for (auto it = arg.begin(); *it; it++) {
			if (!std::isdigit(*it)) {
				fprintf(stderr, "error: failed to parse decimal number for argument `%s`\n\n", argName.c_str());
				std::fputs(errorStr.c_str(), stderr);
				return hk::ResultInvalidArgument();
			}
		}
		return std::stoul(arg, 0, 10);
	}
}

hk::ValueOrResult<fs::path> parse_path_dir(
	const std::string& arg, const std::string& argName, const std::string& errorStr
) {
	if (!fs::is_directory(arg)) {
		fprintf(stderr, "error: directory not found for argument `%s`\n\n", argName.c_str());
		std::fputs(errorStr.c_str(), stderr);
		return mizuna::ResultDirNotFound();
	}

	return fs::path(arg);
}

hk::ValueOrResult<fs::path> parse_path_file(
	const std::string& arg, const std::string& argName, const std::string& errorStr
) {
	if (!fs::is_regular_file(arg)) {
		fprintf(stderr, "error: file not found for argument `%s`\n\n", argName.c_str());
		std::fputs(errorStr.c_str(), stderr);
		return mizuna::ResultFileNotFound();
	}

	return fs::path(arg);
}

s32 get_args_num(s32 argIdx, s32 argc) {
	return argc - argIdx;
}

hk::Result check_args_len(s32 argIdx, s32 argc, s32 numRequired, const std::string& errorStr) {
	if (get_args_num(argIdx, argc) <= numRequired) {
		std::fputs(errorStr.c_str(), stderr);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
