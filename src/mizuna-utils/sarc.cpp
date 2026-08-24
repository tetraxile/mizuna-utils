#include <cstdio>
#include <filesystem>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <string>
#include <zstd/zstd.h>

#include "main.h"
#include "mizuna/sarc/reader.h"
#include "mizuna/sarc/writer.h"
#include "mizuna/util.h"
#include "results.h"
#include "util.h"

hk::Result handle_sarc(s32 argc, char* argv[]) {
	std::string mainErrorStr;
	mainErrorStr += std::format("usage: {} sarc r|read <archive> <output dir>\n", programName);
	mainErrorStr += std::format("       {} sarc w|write <input dir> <output archive>\n", programName);
	mainErrorStr += std::format("       {} sarc l|list <archive>\n", programName);
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
				std::string errorStr = std::format("usage: {} sarc r|read <archive> <output dir>\n", programName);
				HK_TRY(check_args_len(argIdx, argc, 2, errorStr));

				args.read.archive = HK_TRY(parse_path_file(argv[++argIdx], "archive", errorStr));
				args.read.outDir = argv[++argIdx];
			} else if (util::isEqual(arg, "write") || util::isEqual(arg, "w")) {
				subcommand = WRITE;
				std::string errorStr =
					std::format("usage: {} sarc w|write <input dir> <output archive>\n", programName);
				HK_TRY(check_args_len(argIdx, argc, 2, errorStr));

				args.write.inDir = HK_TRY(parse_path_dir(argv[++argIdx], "input dir", errorStr));
				args.write.outArchive = argv[++argIdx];
			} else if (util::isEqual(arg, "list") || util::isEqual(arg, "l")) {
				subcommand = LIST;
				std::string errorStr = std::format("usage: {} sarc l|list <archive>\n", programName);
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

		std::string archiveName = args.read.archive.string();

		std::vector<u8> fileContents;
		if (archiveName.find(".zs") != std::string::npos) {
			std::vector<u8> compressedContents;
			HK_TRY(util::readFile(compressedContents, args.read.archive));
			u64 decompSize = ZSTD_getFrameContentSize(compressedContents.data(), compressedContents.size());
			fileContents.resize(decompSize);
			if (ZSTD_isError(ZSTD_decompress(
					fileContents.data(), fileContents.size(), compressedContents.data(), compressedContents.size()
				))) {
				fprintf(stderr, "error: couldn't decompress ZSTD data\n");
				return mizuna_utils::ResultDecompressionFailed();
			}
		} else {
			HK_TRY(util::readFile(fileContents, args.read.archive));
		}
		sarc::Reader sarc(fileContents);
		HK_TRY(sarc.init());

		HK_TRY(sarc.saveAll(args.read.outDir));
	} else if (subcommand == WRITE) {
		if (fs::exists(args.write.outArchive) && !isForce) {
			fprintf(stderr, "error: can't overwrite output path (try running with --force/-f)\n");
			return hk::ResultInvalidArgument();
		}

		sarc::Writer writer;

		for (const auto& entry : fs::recursive_directory_iterator(args.write.inDir)) {
			if (entry.is_directory()) continue;

			fs::path entryPath = entry.path();
			fs::path relPath = fs::relative(entryPath, args.write.inDir);

			std::vector<u8> fileContents;
			HK_TRY(util::readFile(fileContents, entryPath));

			writer.addFile(relPath.string(), fileContents);
		}

		u32 alignment;
		writer.save(args.write.outArchive, &alignment);
		if (!isQuiet) printf("saved SARC to %s (alignment = %#x)\n", args.write.outArchive.string().c_str(), alignment);
	} else if (subcommand == LIST) {
		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, args.list.archive));

		sarc::Reader sarc(fileContents);
		HK_TRY(sarc.init());

		for (const std::string& filename : sarc.getFilenames())
			printf("%s\n", filename.c_str());
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
