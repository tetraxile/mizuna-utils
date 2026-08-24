#include <cstdio>
#include <filesystem>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <string>
#include <zstd/zstd.h>

#include "hk/Result.h"
#include "main.h"
#include "mizuna/sarc/reader.h"
#include "mizuna/sarc/writer.h"
#include "mizuna/util.h"
#include "mizuna/yaz0.h"
#include "util.h"

hk::Result handle_szs(s32 argc, char* argv[]) {
	std::string mainErrorStr;
	mainErrorStr += std::format("usage: {} szs r|read <archive> <output dir>\n", programName);
	mainErrorStr += std::format("       {} szs w|write <input dir> <output archive> [alignment]\n", programName);
	mainErrorStr += std::format("       {:{}}         (default alignment: 0x80)\n", " ", programName.length());
	mainErrorStr += std::format("       {} szs l|list <archive>\n", programName);
	mainErrorStr += std::format("\n");
	mainErrorStr += std::format("options: --quiet, -q\n");
	mainErrorStr += std::format("         --force, -f\n");
	mainErrorStr += std::format("         --help, -h\n");

	enum { HELP, READ, WRITE, LIST } subcommand = HELP;

	struct {
		struct {
			fs::path archive;
			fs::path outDir;
		} read;

		struct {
			fs::path inDir;
			fs::path outArchive;
			u32 alignment = 0x80;
		} write;

		struct {
			fs::path archive;
		} list;
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
				std::string errorStr = std::format("usage: {} szs r|read <archive> <output dir>\n", programName);
				HK_TRY(check_args_len(argIdx, argc, 2, errorStr));

				args.read.archive = HK_TRY(parse_path_file(argv[++argIdx], "archive", errorStr));
				args.read.outDir = argv[++argIdx];
			} else if (util::isEqual(arg, "write") || util::isEqual(arg, "w")) {
				subcommand = WRITE;
				std::string errorStr;
				errorStr +=
					std::format("usage: {} szs w|write <input dir> <output archive> [alignment]\n", programName);
				errorStr += std::format("       {:{}s}         (default alignment: 0x80)\n", " ", programName.length());
				HK_TRY(check_args_len(argIdx, argc, 2, errorStr));

				args.write.inDir = HK_TRY(parse_path_dir(argv[++argIdx], "input dir", errorStr));
				args.write.outArchive = argv[++argIdx];
				if (argc <= argIdx + 3) args.write.alignment = HK_TRY(parse_u32(argv[++argIdx], "alignment", errorStr));
			} else if (util::isEqual(arg, "list") || util::isEqual(arg, "l")) {
				subcommand = LIST;
				std::string errorStr = std::format("usage: {} szs l|list <archive>\n", programName);
				HK_TRY(check_args_len(argIdx, argc, 1, errorStr));

				args.list.archive = HK_TRY(parse_path_file(argv[++argIdx], "archive", errorStr));
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
		if (fs::exists(args.read.outDir) && !isForce) {
			fprintf(stderr, "error: can't overwrite output path (try running with --force/-f)\n");
			return hk::ResultInvalidArgument();
		}

		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, args.read.archive));

		std::vector<u8> decompressed;
		u32 alignment = 0;
		HK_TRY(yaz0::decompress(decompressed, fileContents, &alignment));

		if (!isQuiet) printf("decompressing Yaz0 with alignment = %#x...\n", alignment);

		sarc::Reader sarc(decompressed);
		HK_TRY(sarc.init());

		HK_TRY(sarc.saveAll(args.read.outDir));
	} else if (subcommand == WRITE) {
		if (fs::exists(args.write.outArchive) && !isForce) {
			fprintf(stderr, "error: can't overwrite output path (try running with --force/-f)\n");
			return hk::ResultInvalidArgument();
		}

		sarc::Writer writer;
		for (const auto& entry : fs::recursive_directory_iterator(args.write.inDir)) {
			fs::path entryPath = entry.path();
			fs::path relPath = fs::relative(entryPath, args.write.inDir);

			std::vector<u8> fileContents;
			HK_TRY(util::readFile(fileContents, entryPath));

			writer.addFile(relPath.string(), fileContents);
		}

		std::vector<u8> sarcContents;
		u32 alignment;
		writer.saveToVec(sarcContents, &alignment);

		std::vector<u8> szsContents;
		yaz0::compress(szsContents, sarcContents, alignment);

		util::writeFile(args.write.outArchive, szsContents);
	} else if (subcommand == LIST) {
		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, args.list.archive));

		std::vector<u8> decompressed;
		HK_TRY(yaz0::decompress(decompressed, fileContents));

		sarc::Reader sarc(decompressed);
		HK_TRY(sarc.init());

		for (const std::string& filename : sarc.getFilenames())
			printf("%s\n", filename.c_str());
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
