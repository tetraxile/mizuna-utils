#include <cstdio>
#include <filesystem>
#include <format>
#include <hk/container/FixedString.h>
#include <hk/container/FixedVec.h>
#include <hk/diag/diag.h>
#include <set>
#include <string>
#include <vector>

#include "config.h"
#include "mini/ini.h"
#include "mizuna/byml/reader.h"
#include "mizuna/byml/writer.h"
#include "mizuna/results.h"
#include "mizuna/sarc/reader.h"
#include "mizuna/util.h"
#include "mizuna/yaz0.h"
#include "results.h"

namespace fs = std::filesystem;

bool endsWith(const std::string& fullString, const std::string& ending) {
	if (fullString.length() >= ending.length())
		return 0 == fullString.compare(fullString.length() - ending.length(), ending.length(), ending);
	else
		return false;
}

struct Tester {
	const std::string getCurPath() const {
		std::string out;
		for (size i = 0; i < curPath.size(); i++) {
			out += curPath[i];
			if (i < curPath.size() - 1) out += '/';
		}
		return out;
	}

	hk::Result testSpecificFile(const fs::path& path);
	hk::Result testWriteAllRomfs(const std::string& gameName, const fs::path& romfsPath);
	hk::Result testWriteDir(const fs::path& dataDir);
	hk::Result testWriteFile(const std::vector<u8>& contents, const std::string& extension);
	hk::Result testWriteSzs(const std::vector<u8>& contents);
	hk::Result testWriteSarc(const std::vector<u8>& archiveBytes);
	hk::Result testWriteByaml(const std::vector<u8>& inputBytes);
	hk::Result testWriteByamlHash(byml::Writer& writer, const byml::Reader& reader);
	hk::Result testWriteByamlArray(byml::Writer& writer, const byml::Reader& reader);

	std::vector<std::string> failed;
	std::vector<std::string> curPath;
};

hk::Result Tester::testSpecificFile(const fs::path& path) {
	std::vector<u8> contents;
	HK_TRY(util::readFile(contents, path));
	curPath.push_back(path.filename());
	HK_TRY(testWriteFile(contents, path.extension().string()));
	curPath.pop_back();

	printf("\nfailed: %zu\n", failed.size());
	for (const std::string& path : failed) {
		printf("\t\x1b[1;31m%s\x1b[1;0m\n", path.c_str());
	}

	return hk::ResultSuccess();
}

hk::Result Tester::testWriteByamlArray(byml::Writer& writer, const byml::Reader& reader) {
	for (u32 i = 0; i < reader.getSize(); i++) {
		byml::NodeType type = HK_TRY(reader.getTypeByIdx(i));
		switch (type) {
		case byml::NodeType::String: {
			std::string str;
			HK_TRY(reader.getStringByIdx(&str, i));
			HK_TRY(writer.addString(str));
			break;
		}
		case byml::NodeType::Binary:
		case byml::NodeType::BinaryAlignment: {
			std::vector<u8> data;
			u32 alignment;
			HK_TRY(reader.getBinaryByIdx(&data, &alignment, i));
			HK_TRY(writer.addBinary(data, alignment));
			break;
		}
		case byml::NodeType::Array: {
			byml::Reader array = HK_TRY(reader.getContainerByIdx(i));
			HK_TRY(writer.pushArray());
			HK_TRY(testWriteByamlArray(writer, array));
			HK_TRY(writer.pop());
			break;
		}
		case byml::NodeType::Hash: {
			byml::Reader hash = HK_TRY(reader.getContainerByIdx(i));
			HK_TRY(writer.pushHash());
			HK_TRY(testWriteByamlHash(writer, hash));
			HK_TRY(writer.pop());
			break;
		}
		case byml::NodeType::Bool: {
			HK_TRY(writer.addBool(HK_TRY(reader.getBoolByIdx(i))));
			break;
		}
		case byml::NodeType::S32: {
			HK_TRY(writer.addS32(HK_TRY(reader.getS32ByIdx(i))));
			break;
		}
		case byml::NodeType::F32: {
			HK_TRY(writer.addF32(HK_TRY(reader.getF32ByIdx(i))));
			break;
		}
		case byml::NodeType::U32: {
			HK_TRY(writer.addU32(HK_TRY(reader.getU32ByIdx(i))));
			break;
		}
		case byml::NodeType::S64: {
			HK_TRY(writer.addS64(HK_TRY(reader.getS64ByIdx(i))));
			break;
		}
		case byml::NodeType::U64: {
			HK_TRY(writer.addU64(HK_TRY(reader.getU64ByIdx(i))));
			break;
		}
		case byml::NodeType::F64: {
			HK_TRY(writer.addF64(HK_TRY(reader.getF64ByIdx(i))));
			break;
		}
		case byml::NodeType::Null: {
			HK_TRY(writer.addNull());
			break;
		}
		case byml::NodeType::StringTable: {
			return byml::ResultInvalidNodeType();
		}
		}
	}

	return hk::ResultSuccess();
}

hk::Result Tester::testWriteByamlHash(byml::Writer& writer, const byml::Reader& reader) {
	for (u32 i = 0; i < reader.getSize(); i++) {
		byml::NodeType type = HK_TRY(reader.getTypeByIdx(i));
		std::string key;
		HK_TRY(reader.getKeyByIdx(&key, i));
		switch (type) {
		case byml::NodeType::String: {
			std::string str;
			HK_TRY(reader.getStringByIdx(&str, i));
			HK_TRY(writer.addString(key, str));
			break;
		}
		case byml::NodeType::Binary:
		case byml::NodeType::BinaryAlignment: {
			std::vector<u8> data;
			u32 alignment;
			HK_TRY(reader.getBinaryByIdx(&data, &alignment, i));
			HK_TRY(writer.addBinary(key, data, alignment));
			break;
		}
		case byml::NodeType::Array: {
			byml::Reader array = HK_TRY(reader.getContainerByIdx(i));
			HK_TRY(writer.pushArray(key));
			HK_TRY(testWriteByamlArray(writer, array));
			HK_TRY(writer.pop());
			break;
		}
		case byml::NodeType::Hash: {
			byml::Reader hash = HK_TRY(reader.getContainerByIdx(i));
			HK_TRY(writer.pushHash(key));
			HK_TRY(testWriteByamlHash(writer, hash));
			HK_TRY(writer.pop());
			break;
		}
		case byml::NodeType::Bool: {
			HK_TRY(writer.addBool(key, HK_TRY(reader.getBoolByIdx(i))));
			break;
		}
		case byml::NodeType::S32: {
			HK_TRY(writer.addS32(key, HK_TRY(reader.getS32ByIdx(i))));
			break;
		}
		case byml::NodeType::F32: {
			HK_TRY(writer.addF32(key, HK_TRY(reader.getF32ByIdx(i))));
			break;
		}
		case byml::NodeType::U32: {
			HK_TRY(writer.addU32(key, HK_TRY(reader.getU32ByIdx(i))));
			break;
		}
		case byml::NodeType::S64: {
			HK_TRY(writer.addS64(key, HK_TRY(reader.getS64ByIdx(i))));
			break;
		}
		case byml::NodeType::U64: {
			HK_TRY(writer.addU64(key, HK_TRY(reader.getU64ByIdx(i))));
			break;
		}
		case byml::NodeType::F64: {
			HK_TRY(writer.addF64(key, HK_TRY(reader.getF64ByIdx(i))));
			break;
		}
		case byml::NodeType::Null: {
			HK_TRY(writer.addNull(key));
			break;
		}
		case byml::NodeType::StringTable: {
			return byml::ResultInvalidNodeType();
		}
		}
	}

	return hk::ResultSuccess();
}

hk::Result Tester::testWriteByaml(const std::vector<u8>& inputBytes) {
	byml::Reader reader;
	HK_TRY(reader.init(inputBytes.data(), inputBytes.size()));

	byml::Writer writer(reader.getVersion());

	if (HK_TRY(reader.getType()) == byml::NodeType::Array) {
		HK_TRY(writer.pushArray());
		HK_TRY(testWriteByamlArray(writer, reader));
		HK_TRY(writer.pop());
	} else if (HK_TRY(reader.getType()) == byml::NodeType::Hash) {
		HK_TRY(writer.pushHash());
		HK_TRY(testWriteByamlHash(writer, reader));
		HK_TRY(writer.pop());
	}

	std::vector<u8> outBytes;
	writer.saveToVec(outBytes, reader.getByteOrder());
	// util::writeFile("/home/tetra/dev/mizuna-utils/test/in.bin", inputBytes);
	// util::writeFile("/home/tetra/dev/mizuna-utils/test/out.bin", outBytes);

	if (inputBytes.size() != outBytes.size()) {
		failed.push_back(
			std::format(
				"{} (size mismatch, {:#x} -> {:#x})\n", getCurPath().c_str(), inputBytes.size(), outBytes.size()
			)
		);

		return mizuna_utils::ResultWriteMismatch();
	}

	bool isMismatch = false;
	size_t i;
	for (i = 0; i < inputBytes.size(); i++) {
		if (inputBytes[i] != outBytes[i]) {
			isMismatch = true;
			break;
		}
	}
	if (isMismatch) {
		failed.push_back(
			std::format(
				"{} (byte mismatch, offset: {:#x}, {:02x} -> {:02x})\n", getCurPath().c_str(), i, inputBytes[i],
				outBytes[i]
			)
		);
		return mizuna_utils::ResultWriteMismatch();
	}

	return hk::ResultSuccess();
}

hk::Result Tester::testWriteSarc(const std::vector<u8>& archiveBytes) {
	sarc::Reader archive(archiveBytes);
	HK_TRY(archive.init());

	for (const std::string& entryName : archive.getFilenames()) {
		curPath.push_back(entryName);
		std::vector<u8> entryBytes;
		HK_TRY(archive.getFileData(entryBytes, entryName));

		const std::string& extension = fs::path(entryName).extension().string();
		HK_TRY(testWriteFile(entryBytes, extension));

		curPath.pop_back();
	}

	return hk::ResultSuccess();
}

hk::Result Tester::testWriteSzs(const std::vector<u8>& contents) {
	std::vector<u8> archiveBytes;
	HK_TRY(yaz0::decompress(archiveBytes, contents));

	HK_TRY(testWriteSarc(archiveBytes));

	return hk::ResultSuccess();
}

hk::Result Tester::testWriteFile(const std::vector<u8>& contents, const std::string& extension) {
	hk::Result r;

	if (extension == ".szs") {
		HK_TRY(testWriteSzs(contents));
	} else if (extension == ".bin") {
	} else if (extension == ".byml") {
		r = testWriteByaml(contents);
	} else if (extension == ".bnvib") {
	} else if (extension == ".gsh") {
	} else if (extension == ".sarc") {
	} else if (extension == ".baglmf" or extension == ".baglcc" or extension == ".baglpreset" or
	           extension == ".baglshpp" or extension == ".baglssao" or extension == ".baglprojparam") {
		HK_TRY(reader::checkSignature(contents.data(), "AAMP", 4));
	} else {
		fprintf(stderr, "error: unexpected file extension '%s'\n", extension.c_str());
		return hk::ResultFailed();
	}

	if (r == mizuna_utils::ResultWriteMismatch()) {
		printf("\x1b[1;31m%s\x1b[1;0m\n", getCurPath().c_str());
	} else if (r.failed()) {
		return r;
	} else {
		printf("\x1b[1;32m%s\x1b[1;0m\n", getCurPath().c_str());
	}

	return hk::ResultSuccess();
}

hk::Result Tester::testWriteDir(const fs::path& dataDir) {
	const std::string dirName = dataDir.stem();
	curPath.push_back(dirName);

	// printf("%s\n", dirName.c_str());

	if (dirName == "LayoutData") {
		// for (const auto& entry : fs::directory_iterator(dataDir)) {
		// 	printf("\t%s\n", entry.path().filename().c_str());
		// }
	} else if (dirName == "LocalizedData") {
		// for (const auto& entry : fs::directory_iterator(dataDir)) {
		// 	printf("\t%s\n", entry.path().filename().c_str());
		// }
	} else if (dirName == "EffectData") {
		// for (const auto& entry : fs::directory_iterator(dataDir)) {
		// 	printf("\t%s\n", entry.path().filename().c_str());
		// }
	} else if (dirName == "StageData") {
		// for (const auto& entry : fs::directory_iterator(dataDir)) {
		// 	printf("\t%s\n", entry.path().filename().c_str());
		// }
	} else if (dirName == "ShaderData") {
		// for (const auto& entry : fs::directory_iterator(dataDir)) {
		// 	printf("\t%s\n", entry.path().filename().c_str());
		// }
	} else if (dirName == "SystemData") {
		for (const auto& entry : fs::directory_iterator(dataDir)) {
			std::vector<u8> entryBytes;
			HK_TRY(util::readFile(entryBytes, entry.path()));
			curPath.push_back(entry.path().filename());
			HK_TRY(testWriteFile(entryBytes, entry.path().extension().string()));
			curPath.pop_back();
		}
	} else if (dirName == "EventData") {
		// for (const auto& entry : fs::directory_iterator(dataDir)) {
		// 	printf("\t%s\n", entry.path().filename().c_str());
		// }
	} else if (dirName == "ObjectData") {
		// for (const auto& entry : fs::directory_iterator(dataDir)) {
		// 	printf("\t%s\n", entry.path().filename().c_str());
		// }
	} else if (dirName == "SoundData") {
		// for (const auto& entry : fs::directory_iterator(dataDir)) {
		// 	printf("\t%s\n", entry.path().filename().c_str());
		// }
	} else if (dirName == "MovieData") {
	} else {
		fprintf(stderr, "error: unrecognised data directory '%s'\n", dirName.c_str());
		return hk::ResultFailed();
	}

	curPath.pop_back();

	return hk::ResultSuccess();
}

hk::Result Tester::testWriteAllRomfs(const std::string& gameName, const fs::path& romfsPath) {
	if (gameName != "smo") return hk::ResultUnknown();
	if (!fs::is_directory(romfsPath)) return hk::ResultFailed();

	for (const auto& dataDir : fs::directory_iterator(romfsPath))
		HK_TRY(testWriteDir(dataDir.path()));

	printf("\nfailed: %zu\n", failed.size());
	for (const std::string& path : failed) {
		printf("\t\x1b[1;31m%s\x1b[1;0m\n", path.c_str());
	}

	return hk::ResultSuccess();
}

s32 main(s32 argc, char** argv) {
	if (argc > 2) {
		fprintf(stderr, "error: unrecognised command-line argument\n");
		return 1;
	}

	mINI::INIFile configFile(getConfigPath());
	mINI::INIStructure ini;
	bool readSuccess = configFile.read(ini);
	if (!readSuccess) generateDefaultConfig();

	std::string gameName = "smo";
	std::string romfsPath;
	if (romfsPath.empty()) romfsPath = ini["romfs"][gameName];
	if (romfsPath.empty()) {
		fprintf(stderr, "error: romfs path for game '%s' not set in config\n", gameName.c_str());
		return 1;
	}

	fs::path specificFile;
	if (argc == 2) {
		specificFile = std::format("{}/{}", romfsPath, argv[1]);
		if (!fs::is_regular_file(specificFile)) {
			fprintf(stderr, "error: input file doesn't exist (%s)\n", specificFile.string().c_str());
			return 1;
		}
	}

	Tester tester;
	hk::Result r;
	if (specificFile.empty())
		r = tester.testWriteAllRomfs(gameName, romfsPath);
	else {
		r = tester.testSpecificFile(specificFile);
	}

	if (r.failed()) fprintf(stderr, "error: %s\n", hk::diag::getResultName(r));

#ifdef _WIN32
	if (!isatty_win()) system("pause");
#endif

	if (r.failed()) return 1;
}
