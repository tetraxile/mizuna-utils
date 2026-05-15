#include <cstdio>
#include <hk/types.h>
#include <string>

#include "argspp/args.h"
#include "config.h"
#include "mini/ini.h"

s32 main(s32 argc, char** argv) {
	std::string gameName;
	std::string romfsPath;

	enum class Mode { help, romfs, defaultGame };
	Mode mode = Mode::help;

	std::string programName = fs::path(argv[0]).filename().string();
	std::string usageText = "usage:\n"
	                        "  " +
	                        programName +
	                        " romfs <game> <romfs path> [-h]\n"
	                        "  " +
	                        programName +
	                        " default_game <game> [-h]\n"
	                        "\n"
	                        "options:\n"
	                        "  <game>          one of \"smo\" or \"3dw\"\n"
	                        "  <romfs path>    path to game's romfs directory\n"
	                        "  -h, --help      show this screen\n";

	args::ArgParser parser(usageText);
	parser.flag("h help");

	args::ArgParser romfsParser =
		parser.command("romfs", usageText, [&mode, &gameName, &romfsPath](std::string cmd, args::ArgParser& parser) {
			if (parser.args.size() != 2) return;
			mode = Mode::romfs;
			gameName = parser.args[0];
			romfsPath = parser.args[1];
			if (!fs::is_directory(romfsPath)) {
				fprintf(stderr, "error: romfs path not found\n\n");
				mode = Mode::help;
			}
		});

	args::ArgParser defaultGameParser =
		parser.command("default_game", usageText, [&mode, &gameName](std::string cmd, args::ArgParser& parser) {
			if (parser.args.size() != 1) return;
			mode = Mode::defaultGame;
			gameName = parser.args[0];
		});

	parser.parse(argc, argv);

	if (mode == Mode::help) {
		fprintf(stderr, "%s", usageText.c_str());
		return parser.found("help") ? 0 : 1;
	}

	if (gameName != "smo" && gameName != "3dw") {
		fprintf(stderr, "error: game must be one of \"smo\" or \"3dw\"\n\n%s", usageText.c_str());
		return 1;
	}

	generateDefaultConfig();

	mINI::INIFile iniFile(getConfigPath());
	mINI::INIStructure ini;
	bool readSuccess = iniFile.read(ini);

	if (!readSuccess) {
		fprintf(stderr, "error: couldn't read mizuna config file");
		return 1;
	}

	if (mode == Mode::romfs) {
		printf("setting romfs path to %s\n", romfsPath.c_str());
		ini["romfs"][gameName] = romfsPath;
	} else if (mode == Mode::defaultGame) {
		printf("setting default game to %s\n", gameName.c_str());
		ini["default"]["game"] = gameName;
	}

	iniFile.write(ini, true);
}
