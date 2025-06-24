#ifndef UTIL_H
#define UTIL_H

#include <string>

class Util {
public:
	static std::string decompressFile(std::string location);
	static std::string sha1Hash(std::string content);
	static std::string compressFile(std::string& input);
	static void parseTreeContent(std::string content, bool isNameOnly);
	static std::string toHex(std::string bin_sha);
};

#endif