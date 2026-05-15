// -----------------------------------------------------------------------------
// Args++: an argument-parsing library in portable C++11.
//
// Author: Darren Mulholland <dmulholl@tcd.ie>
// License: Public Domain
// Version: 2.1.0
// -----------------------------------------------------------------------------

#ifndef args_h
#define args_h

#include <functional>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace args {

struct ArgStream;
struct Option;
struct Flag;

class ArgParser {
public:
	ArgParser(const std::string& helptext = "", const std::string& version = "") :
		helptext(helptext), version(version) {}

	~ArgParser();

	// Stores positional arguments.
	std::vector<std::string> args;

	// Application/command help text and version strings.
	std::string helptext;
	std::string version;

	// Callback function for command parsers.
	std::optional<std::function<void(std::string, ArgParser&)>> callback;

	// Register flags and options.
	void flag(const std::string& name);
	void option(const std::string& name, const std::string& fallback = "");

	// Parse the application's command line arguments.
	void parse(int argc, char** argv);
	void parse(std::vector<std::string> args);

	// Retrieve flag and option values.
	bool found(const std::string& name);
	int count(const std::string& name);
	std::string value(const std::string& name);
	std::vector<std::string> values(const std::string& name);

	// Register a command. Returns the command's ArgParser instance.
	ArgParser& command(
		const std::string& name, const std::string& helptext = "",
		std::optional<std::function<void(std::string, ArgParser&)>> callback = std::nullopt
	);

	// Utilities for handling commands manually.
	bool commandFound();
	std::string commandName();
	ArgParser& commandParser();

	// Print a parser instance to stdout.
	void print();

private:
	std::map<std::string, Option*> options;
	std::map<std::string, Flag*> flags;
	std::map<std::string, ArgParser*> commands;
	std::string command_name;

	void parse(ArgStream& args);
	void registerOption(const std::string& name, Option* option);
	void parseLongOption(std::string arg, ArgStream& stream);
	void parseShortOption(std::string arg, ArgStream& stream);
	void parseEqualsOption(std::string prefix, std::string name, std::string value);
	void exitHelp();
	void exitVersion();
};
} // namespace args

#endif
