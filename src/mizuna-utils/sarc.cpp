#include <cstdio>
#include <filesystem>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <string>
#include <zstd/zstd.h>

#include "main.h"
#include "mizuna/results.h"
#include "mizuna/sarc/reader.h"
#include "mizuna/sarc/writer.h"
#include "mizuna/util.h"

hk::Result handle_sarc(s32 argc, char* argv[]) {
	if (argc < 3 || util::isEqual(argv[2], "--help")) {
		fprintf(stderr, "usage: %s sarc r|read <archive> <output dir>\n", programName.c_str());
		fprintf(stderr, "       %s sarc w|write <input dir> <output archive>\n", programName.c_str());
		fprintf(stderr, "       %s sarc l|list <archive>\n", programName.c_str());
		return hk::ResultInvalidArgument();
	}

	if (util::isEqual(argv[2], "read") || util::isEqual(argv[2], "r")) {
		if (argc < 5) {
			fprintf(stderr, "usage: %s sarc r|read <archive> <output dir>\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}

		std::string archiveName = argv[3];

		std::vector<u8> fileContents;
		if (archiveName.find(".zs") != std::string::npos) {
			std::vector<u8> compressedContents;
			HK_TRY(util::readFile(compressedContents, archiveName));
			u64 decompSize = ZSTD_getFrameContentSize(compressedContents.data(), compressedContents.size());
			fileContents.resize(decompSize);
			ZSTD_decompress(
				fileContents.data(), fileContents.size(), compressedContents.data(), compressedContents.size()
			);
		} else {
			HK_TRY(util::readFile(fileContents, archiveName));
		}
		sarc::Reader sarc(fileContents);
		HK_TRY(sarc.init());

		HK_TRY(sarc.saveAll(argv[4]));
	} else if (util::isEqual(argv[2], "write") || util::isEqual(argv[2], "w")) {
		if (argc < 5) {
			fprintf(stderr, "usage: %s sarc w|write <input dir> <output archive>\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}

		fs::path inDir = argv[3];
		if (!fs::is_directory(inDir)) {
			return mizuna::ResultDirNotFound();
		}

		sarc::Writer writer;

		for (const auto& entry : fs::recursive_directory_iterator(inDir)) {
			fs::path entryPath = entry.path();
			fs::path relPath = fs::relative(entryPath, inDir);

			if (entry.is_directory()) continue;

			std::vector<u8> fileContents;
			HK_TRY(util::readFile(fileContents, entryPath));

			writer.addFile(relPath.string(), fileContents);
		}

		u32 alignment;
		writer.save(argv[4], &alignment);
		printf("saved SARC to %s (alignment = %#x)\n", argv[4], alignment);
	} else if (util::isEqual(argv[2], "list") || util::isEqual(argv[2], "l")) {
		if (argc < 4) {
			fprintf(stderr, "usage: %s sarc l|list <archive>\n", programName.c_str());
			return hk::ResultInvalidArgument();
		}

		std::vector<u8> fileContents;
		HK_TRY(util::readFile(fileContents, argv[3]));

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
