#include "../inc/Util.hpp"
#include <zlib.h>
#include <vector>
#include <iostream>
#include <openssl/sha.h>
#include <iomanip>

std::string Util::decompressFile(std::string compressedContent) {
	uLongf decompSize = 10000;
	std::vector<char> buffer(decompSize);
	int res = uncompress(
		reinterpret_cast<Bytef*>(buffer.data()),
		&decompSize,
		reinterpret_cast<Bytef*>(compressedContent.data()),
		compressedContent.size());
	if (res != Z_OK) {
		std::cerr << "Decompression failed.\n";
	}
	// return everything, not only before the first \0
	return std::string(buffer.data(), decompSize);
}

std::string Util::sha1Hash(std::string content) {
	unsigned char obuf[SHA_DIGEST_LENGTH]; // 20B = 40 hex chars
	SHA1(reinterpret_cast<const unsigned char*>(content.c_str()), content.size(), obuf);

	// bytes to hex chars:
	std::ostringstream result;
	for (int i = 0; i < SHA_DIGEST_LENGTH; ++i)
		result << std::hex << std::setw(2) << std::setfill('0') << (int)obuf[i];
	return result.str();
}

std::string Util::compressFile(std::string& input) {
    z_stream zs;
    memset(&zs, 0, sizeof(zs));

    if (deflateInit(&zs, Z_BEST_SPEED) != Z_OK) {
        throw std::runtime_error("deflateInit failed");
    }

    zs.next_in = (Bytef*)input.data();
    zs.avail_in = input.size();

    std::vector<Bytef> buffer(input.size());  
    std::string output;

    int ret;
    do {
        zs.next_out = buffer.data();
        zs.avail_out = buffer.size();

        ret = deflate(&zs, Z_FINISH);

        if (output.size() < zs.total_out) {
            output.append((char*)buffer.data(), zs.total_out - output.size());
        }
    } while (ret == Z_OK);

    deflateEnd(&zs);

    if (ret != Z_STREAM_END) {
        throw std::runtime_error("compression failed");
    }

    return output;
}

std::string Util::toHex(std::string bin_sha) {
    // svaki char pojedinacno i hex (2 chara)
    std::stringstream ss;
    for (unsigned char c : bin_sha) {
        ss << std::hex << std::setfill('0') << std::setw(2) << (int)c;
    }
    return ss.str();
}

void Util::parseTreeContent(std::string content, bool isNameOnly) {
    /*
    *   The format of a tree object file looks like this (after Zlib decompression):

          tree <size>\0
          <mode> <name>\0<20_byte_sha>
          <mode> <name>\0<20_byte_sha>

        (The above code block is formatted with newlines for readability, but the actual file doesn't contain newlines)

        The file starts with tree <size>\0. This is the "object header", similar to what we saw with blob objects.

        After the header, there are multiple entries. Each entry is of the form <mode> <name>\0<sha>.
        <mode> is the mode of the file/directory (check the previous section for valid values)
        <name> is the name of the file/directory
        \0 is a null byte
        <20_byte_sha> is the 20-byte SHA-1 hash of the blob/tree (this is not in hexadecimal format)


        Output:
          $ git ls-tree <tree_sha>
          040000 tree <tree_sha_1>    dir1
          040000 tree <tree_sha_2>    dir2
          100644 blob <blob_sha_1>    file1
    *   
    */

    size_t pos = content.find('\0');
    if (pos == std::string::npos) {
        std::cerr << "Invalid tree object." << std::endl;
        return;
    }
    size_t i = pos + 1;
    while (i < content.size()) {
        std::string name;
        std::string mode, bin_sha, hex_sha, type;
        while (content[i] != ' ') {
            mode += content[i++];
        }
        i++;
        while (content[i] != '\0') {
            name += content[i++];
        }
        i++;
        bin_sha = content.substr(i, 20);
        hex_sha = toHex(bin_sha);
        i += 20;
        type = (mode == "40000" || mode == "040000") ? "tree" : "blob";

        if (isNameOnly) {
            std::cout << name << std::endl;
        }
        else {
            std::cout << mode << " " << type << " " << hex_sha << "\t" << name << std::endl;
        }
    }
}