#pragma once
#include "ForwardDecs.h"
#include <vector>
#include <map>

/*
 Defines a helper class for easily reading from a series of configuration files
 by key. We will be using .INI files, for most things. If we decide to have a
 particular format for texture/material files, we can also add to this class as
 needed.

 ABOUT INI FILES
 ---------------

 The basic structure of an INI file is a collection of sections, where each section
 has a list of key-value pairs. Example:

 [GameConfig]             <---- Name inside the brackets defines a SECTION
 foo=123                  <---- Key 'foo' has value '123'
 bar=1.25	                    Key 'bar' has value '1.25'
 baz=John Doe                   Key 'baz' has value 'John Doe'

 For more info: https://en.wikipedia.org/wiki/INI_file

 USAGE
 -----

 Once all of the config files have been loaded, reading them should be easy.

 ConfigFile::getInt("GameConfig","foo");    // Returns 123

 */
class ConfigFile
{
public:

	ConfigFile(std::string& configFilePath);
	~ConfigFile();

	// INI file constants
	static const char TOK_L_BRAC = '[';
	static const char TOK_R_BRAC = ']';
	static const char TOK_EQUALS = '=';
	static const char TOK_COMMENT = ';';

	// Retrieve a key from one of the many config files we've loaded
	int getInt(const std::string& section, const std::string& key);
	float getFloat(const std::string& section, const std::string& key);
	std::string& getString(const std::string& section, const std::string& key);

private:

	enum TokenizedStringType {
		COMMENT = 0x0,
		SECTION = 0x1,
		KEY_VALUE_PAIR = 0x2,
		UNKNOWN = 0x3
	};

	// A section of an .ini file
	class ConfigSection {
		friend ConfigFile;
	public:
		std::string& get(const std::string& key);
		bool hasKey(const std::string& key);

	private:
		void set(const std::string& key, const std::string& value);
		std::map<std::string, std::string> keyValuePairs;
	};

	// Member variables
	std::string configFilePath;
	std::map<std::string, ConfigSection> sections;

	// Member functions
	void load();
	bool hasSection(std::string& section);
	std::vector<std::string> tokenize(const std::string& line, ConfigFile::TokenizedStringType type);

};