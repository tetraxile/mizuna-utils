#include "mizuna/bfsha.h"

#include <cstdio>
#include <filesystem>
#include <hk/ValueOrResult.h>
#include <hk/diag/diag.h>
#include <string>

#include "main.h"
#include "mizuna/util.h"
#include "util.h"

hk::Result parse_bfsha(const fs::path& outPath, const BFSHA& bfsha) {
	std::string out;

	out += "filetype bfsha\n";
	out += std::format("version {}\n", bfsha.getVersion());
	out += "endian ";
	switch (bfsha.getByteOrder()) {
	case util::ByteOrder::Big: out += "big\n"; break;
	case util::ByteOrder::Little: out += "little\n"; break;
	}
	out += "\n";

	out += std::format("name: {}\n", bfsha.name);
	out += std::format("original_path: {}\n", bfsha.originalPath);

	out += "models:\n";
	for (auto [shadingModelKey, shadingModel] : bfsha.shadingModels) {
		out += std::format("  {}\n", shadingModelKey);
		out += std::format("    name: {}\n", shadingModel.name);

		if (!shadingModel.staticOptions.empty()) {
			out += std::format("    static options:\n");
			for (auto [optionKey, option] : shadingModel.staticOptions) {
				out += std::format("      {}\n", optionKey);
				out += std::format("        name: {}\n", option.name);
				out += std::format("        default index: {}\n", option.defaultIndex);
				out += std::format("        branch offset: {:#x}\n", option.branchOffset);
				out += std::format("        flags: {:#x}\n", option.flags);
				out += std::format("        key offset: {:#x}\n", option.keyOffset);
				out += std::format("        index: {}\n", option.index);
				out += std::format("        shift: {}\n", option.shift);
				out += std::format("        mask: {:#x}\n", option.mask);
				if (!option.choices.empty()) {
					out += std::format("        choices:\n");
					for (auto [choiceKey, choice] : option.choices) {
						if (option.hasArray)
							out += std::format("          {} -> {}\n", choiceKey, choice.value);
						else
							out += std::format("          {}\n", choiceKey);
					}
				}
			}
			out += "\n";
		}

		if (!shadingModel.dynamicOptions.empty()) {
			out += std::format("    dynamic options:\n");
			for (auto [optionKey, option] : shadingModel.dynamicOptions) {
				out += std::format("      {}\n", optionKey);
				out += std::format("        name: {}\n", option.name);
				out += std::format("        default index: {}\n", option.defaultIndex);
				out += std::format("        branch offset: {:#x}\n", option.branchOffset);
				out += std::format("        flags: {:#x}\n", option.flags);
				out += std::format("        key offset: {:#x}\n", option.keyOffset);
				out += std::format("        index: {}\n", option.index);
				out += std::format("        shift: {}\n", option.shift);
				out += std::format("        mask: {:#x}\n", option.mask);
				if (!option.choices.empty()) {
					out += std::format("        choices:\n");
					for (auto [choiceKey, choice] : option.choices) {
						if (option.hasArray)
							out += std::format("          {} -> {}\n", choiceKey, choice.value);
						else
							out += std::format("          {}\n", choiceKey);
					}
				}
			}
			out += "\n";
		}

		if (!shadingModel.attributes.empty()) {
			out += std::format("    attributes:\n");
			for (auto [attrKey, attr] : shadingModel.attributes) {
				out += std::format("      {}\n", attrKey);
				out += std::format("        index: {}\n", attr.index);
				out += std::format("        location: {}\n", attr.location);
			}
			out += "\n";
		}

		if (!shadingModel.samplers.empty()) {
			out += std::format("    samplers:\n");
			for (auto [samplerKey, sampler] : shadingModel.samplers) {
				out += std::format("      {}\n", samplerKey);
				out += std::format("        index: {}\n", sampler.index);
				if (!sampler.altName.empty()) out += std::format("        alt name:\n", sampler.altName);
			}
			out += "\n";
		}

		if (!shadingModel.uniformBlocks.empty()) {
			out += std::format("    uniform blocks:\n");
			for (auto [uniformBlockKey, uniformBlock] : shadingModel.uniformBlocks) {
				out += std::format("      {}\n", uniformBlockKey);
				out += std::format("        type: {}\n", UniformBlock::getTypeName(uniformBlock.type));
				out += std::format("        block size: {:#x}\n", uniformBlock.blockSize);
				out += std::format("        shading model index: {}\n", uniformBlock.shadingModelIndex);
				if (!uniformBlock.uniforms.empty()) {
					out += std::format("        uniforms:\n");
					for (auto [uniformKey, uniform] : uniformBlock.uniforms) {
						out += std::format("          {}\n", uniformKey);
						out += std::format("            index: {}\n", uniform.index);
						out += std::format("            data offset: {}\n", uniform.dataOffset);
						if (!uniform.conversionFunctionName.empty())
							out += std::format("            conversion function: {}\n", uniform.conversionFunctionName);
					}
				}
			}
			out += "\n";
		}

		if (!shadingModel.programs.empty()) {
			out += std::format("    programs:\n");
			for (size_t i = 0; i < shadingModel.programs.size(); i++) {
				Program program = shadingModel.programs[i];
				out += std::format("      {}\n", i);
				out += std::format("        flags: {:#x}\n", program.flags);
				out += std::format("        attribute active flag: {:#x}\n", program.attributeActiveFlag);
				out += std::format("        variation offset: {:#x}\n", program.variationOffset);
				if (!program.samplerIndexTable.empty()) {
					out += std::format("        sampler indices:\n");
					for (size_t j = 0; j < program.samplerIndexTable.size(); j++) {
						ShaderLocation location = program.samplerIndexTable[j];
						out += std::format(
							"          {} -> ({}, {}, {}, {})\n", j, location.vertex, location.geometry,
							location.fragment, location.compute
						);
					}
				}
				if (!program.uniformBlockIndexTable.empty()) {
					out += std::format("        uniform block indices:\n");
					for (size_t j = 0; j < program.uniformBlockIndexTable.size(); j++) {
						ShaderLocation location = program.uniformBlockIndexTable[j];
						out += std::format(
							"          {} -> ({}, {}, {}, {})\n", j, location.vertex, location.geometry,
							location.fragment, location.compute
						);
					}
				}
				if (!program.storageBufferIndexTable.empty()) {
					out += std::format("        storage buffer indices:\n");
					for (size_t j = 0; j < program.storageBufferIndexTable.size(); j++) {
						ShaderLocation location = program.storageBufferIndexTable[j];
						out += std::format(
							"          {} -> ({}, {}, {}, {})\n", j, location.vertex, location.geometry,
							location.fragment, location.compute
						);
					}
				}
			}
			out += "\n";
		}

		if (!shadingModel.keyTable.empty()) {
			out += std::format("    key table:\n");
			for (u32 value : shadingModel.keyTable)
				out += std::format("      {:#x}\n", value);
			out += "\n";
		}

		SymbolTable symbolTable = shadingModel.symbolTable;
		if (!symbolTable.samplers.empty() || !symbolTable.uniformBlocks.empty()) {
			out += std::format("    symbol table:\n");
			if (!symbolTable.samplers.empty()) {
				out += std::format("      samplers:\n");
				for (Symbol symbol : symbolTable.samplers) {
					out += std::format(
						"        (\"{}\" -> \"{}\"), (\"{}\" -> \"{}\")\n", symbol.name1, symbol.value1, symbol.name2,
						symbol.value2
					);
				}
			}
			if (!symbolTable.uniformBlocks.empty()) {
				out += std::format("      uniform blocks:\n");
				for (Symbol symbol : symbolTable.uniformBlocks) {
					out += std::format(
						"        (\"{}\" -> \"{}\"), (\"{}\" -> \"{}\")\n", symbol.name1, symbol.value1, symbol.name2,
						symbol.value2
					);
				}
			}
		}
		out += "\n";
	}

	util::writeFile(outPath, out);

	return hk::ResultSuccess();
}

hk::Result handle_bfsha(s32 argc, char* argv[]) {
	std::string mainErrorStr;
	mainErrorStr += std::format("usage: {} bfsha r <shader archive> <output metadata file>\n", programName);
	mainErrorStr += std::format("\n");
	mainErrorStr += std::format("options: --quiet, -q\n");
	mainErrorStr += std::format("         --force, -f\n");
	mainErrorStr += std::format("         --help, -h\n");

	enum { HELP, READ } subcommand = HELP;

	struct {
		struct {
			fs::path inFile;
			fs::path outFile;
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
				std::string errorStr =
					std::format("usage: {} bfsha r <shader archive> <output metadata file>\n", programName);
				HK_TRY(check_args_len(argIdx, argc, 2, errorStr));

				args.read.inFile = HK_TRY(parse_path_file(argv[++argIdx], "shader archive", errorStr));
				args.read.outFile = args.read.outFile = argv[++argIdx];
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
		HK_TRY(util::readFile(fileContents, args.read.inFile));

		BFSHA bfsha(fileContents);
		HK_TRY(bfsha.read());
		HK_TRY(parse_bfsha(args.read.outFile, bfsha));

		for (auto [key, model] : bfsha.shadingModels) {
			fs::path shaderPath = (args.read.outFile.parent_path() / (model.name + ".bnsh"));
			util::writeFile(shaderPath, model.shaderContents);
		}
	} else {
		fprintf(stderr, "error: unrecognized option '%s'\n", argv[2]);
		return hk::ResultInvalidArgument();
	}

	return hk::ResultSuccess();
}
