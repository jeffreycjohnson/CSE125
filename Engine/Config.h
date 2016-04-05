#pragma once
#include "ForwardDecs.h";
#include <vector>
#include <map>

// INI file constants
#define TOK_L_BRAC "["
#define TOK_R_BRAC "]"
#define TOK_EQUALS "="

/*
 Defines a helper class for easily reading from a series of configuration files
 by key. We will be using .INI files, for most things. If we decide to have a
 particular format for texture/material files, we can also add to this class as
 needed.

 *** Please note that Config is a singleton! ***

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

 Config::getInt("GameConfig","foo");    // Returns 123

 */
class Config
{
public:
	Config(std::string& configPath);
	~Config();

	// Retrieve a key from one of the many config files we've loaded
	static int getInt(const std::string& section, const std::string& key);
	static float getFloat(const std::string& section, const std::string& key);
	static std::string& getString(const std::string& section, const std::string& key);

private:
	static Config* instance;

	// Member variables
	std::string configFilePath;

	// Member functions
	void loadConfigFiles();

	// A section of an .ini file
	class ConfigSection {
	public:
		ConfigSection();
		~ConfigSection();

		std::string& get(const std::string& key);
		bool hasKey(const std::string& key);
	
	private:
		std::map<std::string,std::string> keyValuePairs;
	};

	// An individual .ini file
	class ConfigFile {
	public:
		ConfigFile(std::string& filename);
		~ConfigFile();
		std::string& get(std::string& section, std::string& key);

	private:
		bool hasSection(std::string& section);

	private:
		std::map<std::string, ConfigSection> sections;
		std::vector<std::string> tokenize(std::string&);
	};

	// List of all the config files we've loaded for the game
	std::vector<ConfigFile> files;

};