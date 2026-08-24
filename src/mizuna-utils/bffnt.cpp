#include "mizuna/bffnt.h"

#include <cstdio>
#include <filesystem>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <string>

#include "main.h"
#include "mizuna/util.h"
#include "util.h"

hk::Result handle_bffnt(s32 argc, char* argv[]) {
	std::string mainErrorStr;
	mainErrorStr += std::format("usage: {} bffnt r <font file>\n", programName);
	mainErrorStr += std::format("\n");
	mainErrorStr += std::format("options: --quiet, -q\n");
	mainErrorStr += std::format("         --force, -f\n");
	mainErrorStr += std::format("         --help, -h\n");

	enum { HELP, READ } subcommand = HELP;

	struct {
		struct {
			fs::path inFile;
		} read;
	} args;

	bool hasSubcommand = false;
	bool isQuiet = false;
	bool isForce = false;
	for (s32 argIdx = 2; argIdx < argc; argIdx++) {
		const char* arg = argv[argIdx];
		if (util::isEqual(arg, "--help") || util::isEqual(arg, "-h")) {
			subcommand = HELP;
			break;
		} else if (!isQuiet && (util::isEqual(arg, "--quiet") || util::isEqual(arg, "-q"))) {
			isQuiet = true;
		} else if (!isForce && (util::isEqual(arg, "--force") || util::isEqual(arg, "-f"))) {
			isForce = true;
		} else if (!hasSubcommand) {
			if (util::isEqual(arg, "read") || util::isEqual(arg, "r")) {
				subcommand = READ;
				std::string errorStr = std::format("usage: {} bffnt r <font file>\n", programName);
				HK_TRY(check_args_len(argIdx, argc, 1, errorStr));

				args.read.inFile = HK_TRY(parse_path_file(argv[++argIdx], "font file", errorStr));
			} else {
				fprintf(stderr, "error: unexpected argument `%s`\n\n", arg);
				fputs(mainErrorStr.c_str(), stderr);
				return hk::ResultInvalidArgument();
			}
			hasSubcommand = true;
		} else {
			fprintf(stderr, "error: unexpected argument `%s`\n\n", arg);
			fputs(mainErrorStr.c_str(), stderr);
			return hk::ResultInvalidArgument();
		}
	}

	if (subcommand == HELP) {
		fputs(mainErrorStr.c_str(), stderr);
		return hk::ResultInvalidArgument();
	} else if (subcommand == READ) {
		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, argv[3]));

		BFFNT bffnt(fileContents);
		HK_TRY(bffnt.read());
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
