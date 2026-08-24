#include "mizuna/yaz0.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <iostream>
#include <string>

#include "main.h"
#include "mizuna/util.h"
#include "util.h"

hk::Result handle_yaz0(s32 argc, char* argv[]) {
	std::string mainErrorStr;
	mainErrorStr += std::format("usage: {} yaz0 r <compressed file> <decompressed file>\n", programName);
	mainErrorStr += std::format("       {} yaz0 w <decompressed file> <compressed file> [alignment]\n", programName);
	mainErrorStr += std::format("       {:{}}         (default alignment: 0x80)\n", " ", programName.length());
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
			fs::path outFile;
			u32 alignment = 0x80;
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
				std::string errorStr =
					std::format("usage: {} yaz0 r <compressed file> <decompressed file>\n", programName);
				HK_TRY(check_args_len(argIdx, argc, 2, errorStr));

				args.read.inFile = HK_TRY(parse_path_file(argv[++argIdx], "compressed file", errorStr));
				args.read.outFile = argv[++argIdx];
			} else if (util::isEqual(arg, "write") || util::isEqual(arg, "w")) {
				subcommand = WRITE;
				std::string errorStr =
					std::format("usage: {} yaz0 w <decompressed file> <compressed file> [alignment]\n", programName);
				HK_TRY(check_args_len(argIdx, argc, 2, errorStr));

				args.write.inFile = HK_TRY(parse_path_file(argv[++argIdx], "decompressed file", errorStr));
				args.write.outFile = argv[++argIdx];
				if (get_args_num(argIdx, argc) > 1)
					args.write.alignment = HK_TRY(parse_u32(argv[++argIdx], "alignment", errorStr));
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

		std::vector<u8> outputBuffer;
		u32 alignment = 0;
		HK_TRY(yaz0::decompress(outputBuffer, fileContents, &alignment));

		if (!isQuiet) printf("decompressing Yaz0 with alignment = %#x...\n", alignment);

		std::ofstream outfile(args.read.outFile, std::ios::out | std::ios::binary);
		outfile.write(reinterpret_cast<const char*>(outputBuffer.data()), outputBuffer.size());
	} else if (subcommand == WRITE) {
		if (fs::exists(args.write.outFile) && !isForce) {
			fprintf(stderr, "error: can't overwrite output path (try running with --force/-f)\n");
			return hk::ResultInvalidArgument();
		}

		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, args.write.inFile));

		std::vector<u8> outputBuffer;
		yaz0::compress(outputBuffer, fileContents, args.write.alignment);

		util::writeFile(args.write.outFile, outputBuffer);
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
