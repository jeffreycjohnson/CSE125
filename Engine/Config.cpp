#include "Config.h"
#include <iostream>
#include <fstream>
#include <sstream>

ConfigFile::ConfigFile(std::string& configFilePath)
{
	this->configFilePath = configFilePath;
	load();
}


ConfigFile::~ConfigFile()
{
}

int ConfigFile::getInt(const std::string & section, const std::string & key)
{
	return std::stoi(sections[section].get(key));
}

float ConfigFile::getFloat(const std::string & section, const std::string & key)
{
	return std::stof(sections[section].get(key));
}

std::string & ConfigFile::getString(const std::string & section, const std::string & key)
{
	return sections[section].get(key);
}


void ConfigFile::load() {
	// Open the config file
	std::ifstream file(this->configFilePath);
	std::string line;
	std::string currentSection;
	long lineNumber = 0;

	while (file.good()) {
		getline(file, line);
		lineNumber++;
		auto type = TokenizedStringType::UNKNOWN;

		// Tokenize will figure out what kind of line this
		// is, (comment, section header, or key-value pair)
		auto tokens = tokenize(line, type);

		switch (type) {
			case TokenizedStringType::COMMENT: {
				continue;
			} 
			break;
			case TokenizedStringType::SECTION: {
				// tokens list is '[', 'Name', ']'
				currentSection = tokens[1];
				if ( !hasSection(currentSection) ) {
					ConfigSection c;
					sections[currentSection] = c; // just copy. avoid ptrs.
				}
			}
			break;
			case TokenizedStringType::KEY_VALUE_PAIR: {
				// tokens list is '{{key}}', '{{value}}'
				sections[currentSection].set(tokens[0], tokens[1]);
			}
			break;
			case TokenizedStringType::UNKNOWN: {
				std::cerr << "Error parsing: {" << configFilePath << "} at line (" << lineNumber << ")" << std::endl;
				break;
			}
			break;
		}
	}
	if (file.bad() && !file.eof()) {
		std::cerr << "Error while reading {" << configFilePath << "}. File may be corrupted, or something like that." << std::endl;
	}

	file.close();

}

bool ConfigFile::hasSection(std::string& section) {
	// If the section name doesn't exist in our map, return false
	auto iter = sections.find(section);
	return ( iter != sections.end() );
}

/**
 * This function returns a vector of strings, where every string is a valid
 * token in the INI file. Since INI files have different kinds of lines, this
 * function returns different tokens depending on the line we read from the file.
 * This is what the "type" var is for. It let's us know what to expect from the
 * return value:
 *   a COMMENT, a SECTION header, or a KEY_VALUE_PAIR
 *
 * NOTE:  If we were unable to determine
 */
std::vector<std::string> ConfigFile::tokenize(const std::string& line, ConfigFile::TokenizedStringType type) {

	std::vector<std::string> tokens;

	if ( line.length() == 0 ) {
		type = TokenizedStringType::COMMENT;
		return tokens;
	}

	// Detect what kind of line this is, by peeking at the first char
	if ( line.at(0) == TOK_L_BRAC ) {
		std::string sectionName = line.substr(1, line.length() - 2);
		tokens.push_back("[");
		tokens.push_back(sectionName);
		tokens.push_back("]");
		type = TokenizedStringType::SECTION;
		return tokens;
	}
	else if ( line.at(0) == TOK_COMMENT ) {
		return tokens;
	}
	else {
		auto found = line.find(TOK_EQUALS);

		// We found an equals character. Hooray!
		if ( found != std::string::npos ) {
			std::string key   = line.substr(0, found);
			std::string value = line.substr(found + 1, line.length() - found - 1);
			tokens.push_back(key);
			tokens.push_back(value);
			type = TokenizedStringType::KEY_VALUE_PAIR;
		}
		else {
			type = TokenizedStringType::UNKNOWN;
		}
	}

	return tokens;

}

std::string & ConfigFile::ConfigSection::get(const std::string & key)
{
	if (hasKey(key)) {
		return keyValuePairs[key];
	}
	else {
		return std::string("");
	}
}

bool ConfigFile::ConfigSection::hasKey(const std::string & key)
{
	auto iter = keyValuePairs.find(key);
	return (iter != keyValuePairs.end());
}

void ConfigFile::ConfigSection::set(const std::string & key, const std::string & value)
{
	keyValuePairs[key] = value;
}
