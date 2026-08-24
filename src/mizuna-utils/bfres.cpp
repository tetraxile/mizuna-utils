#include <cstdio>
#include <filesystem>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <string>

#include "main.h"
#include "mizuna/bfres/reader.h"
#include "mizuna/util.h"
#include "util.h"

hk::Result handle_bfres(s32 argc, char* argv[]) {
	std::string mainErrorStr;
	mainErrorStr += std::format("usage: {} bfres r <input file> <output file>\n", programName);
	mainErrorStr += std::format("       {} bfres w <input file>\n", programName);
	mainErrorStr += std::format("\n");
	mainErrorStr += std::format("options: --quiet, -q\n");
	mainErrorStr += std::format("         --force, -f\n");
	mainErrorStr += std::format("         --help, -h\n");

	enum { HELP, READ, WRITE } subcommand = HELP;

	struct {
		struct {
			fs::path inFile;
			fs::path outFile;
		} read;

		struct {
			fs::path inFile;
		} write;
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
				std::string errorStr = std::format("usage: {} bfres r <input file> <output file>\n", programName);
				HK_TRY(check_args_len(argIdx, argc, 2, errorStr));

				args.read.inFile = HK_TRY(parse_path_file(argv[++argIdx], "input file", errorStr));
				args.read.outFile = argv[++argIdx];
			} else if (util::isEqual(arg, "write") || util::isEqual(arg, "w")) {
				subcommand = WRITE;
				std::string errorStr = std::format("usage: {} bfres w <input file>\n", programName);
				HK_TRY(check_args_len(argIdx, argc, 1, errorStr));

				args.write.inFile = HK_TRY(parse_path_file(argv[++argIdx], "input file", errorStr));
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
		if (fs::exists(args.read.outFile) && !isForce) {
			fprintf(stderr, "error: can't overwrite output path (try running with --force/-f)\n");
			return hk::ResultInvalidArgument();
		}
		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, args.read.inFile));

		bfres::Reader bfres(fileContents);
		HK_TRY(bfres.read());

		HK_TRY(bfres.exportGLTF(args.read.outFile));
	} else if (subcommand == WRITE) {
		printf("meow %s\n", args.write.inFile.string().c_str());
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
