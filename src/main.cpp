#include "../inc/Util.hpp"
#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <zlib.h>

#include "../inc/GitInternals.hpp"

using namespace std;

int main(int argc, char *argv[])
{

    GitInternals MyGit;
   
    // Flush after every std::cout / std::cerr
    std::cout << std::unitbuf;
    std::cerr << std::unitbuf;

     if (argc < 2) {
         std::cerr << "No command provided.\n";
         return EXIT_FAILURE;
     }
    
    string command = argv[1];

    cout << "Command: " << command << endl;
    
     if (command == "init") {
         try {
             std::filesystem::create_directory(".git");
             std::filesystem::create_directory(".git/objects");
             std::filesystem::create_directory(".git/refs");
    
             std::ofstream headFile(".git/HEAD");
             if (headFile.is_open()) {
                 headFile << "ref: refs/heads/main\n";
                 headFile.close();
             } else {
                 std::cerr << "Failed to create .git/HEAD file.\n";
                 return EXIT_FAILURE;
             }
    
             cout << "Initialized git directory\n";
         } catch (const std::filesystem::filesystem_error& e) {
             std::cerr << e.what() << '\n';
             return EXIT_FAILURE;
         }
     } else if (command == "cat-file") {
         /*
         *  cat-file: print blob file content
            An object starts with a header that specifies its type: blob, commit, tag or tree.
            This header is followed by an ASCII space (0x20).
            Then the size of the object in bytes as an ASCII num.
            Then null (0x00) follows.
            Then the contents of the object.
            - all of the above are included in size -            
         */
        
         string hashedObj = argv[3];
         string subdir = hashedObj.substr(0, 2);
         string hashedContent = hashedObj.substr(2);
         string location = ".git/objects/" + subdir + "/" + hashedContent;

         // open file
         ifstream file;
         file.open(location);
         if (!file.is_open()) {
             cerr << "Could not open file." << endl;
             return EXIT_FAILURE;
         }

         //get compressed content - from the binary file to string
         stringstream buffer;
         buffer << file.rdbuf();
         string compressedContent = buffer.str();

         // unzip
         string content = Util::decompressFile(compressedContent);

         // print - from the start of the real content
         const string data = content.substr(content.find('\0') + 1);
         cout << data;
     }
     else if (command == "hash-object") {
         string file = argv[3];

         // read file contents
         ifstream fileStream;
         fileStream.open(file);
         if (!fileStream.is_open()) {
             cerr << "Could not open file." << endl;
             return EXIT_FAILURE;
         }

         stringstream buffer;
         buffer << fileStream.rdbuf();
         string content = buffer.str();

         // sha-1 : za ime fajla i podfoldera
         string header = "blob " + std::to_string(content.size()) + '\0';
         string fullObject = header + content;
         string sha1 = Util::sha1Hash(fullObject);

         if (argv[2] != "-w") {
             cout << sha1 << endl;
             return EXIT_SUCCESS;
         }

         cout << sha1 << endl;

         // make file
         string subdir = sha1.substr(0, 2);
         string name = sha1.substr(2);
         string path = ".git/objects/" + subdir + "/" + name;

         std::filesystem::create_directories(".git/objects/" + subdir);
         std::ofstream blobFile(path);

         // compress
         if (blobFile.is_open()) { 
             blobFile << Util::compressFile(fullObject);
             blobFile.close();
         }
         else {
             std::cerr << "Failed to create file: " << path << endl;
             return EXIT_FAILURE;
         }
     }
     else if (command == "ls-tree") {
        // in: git ls-tree --name-only <tree_sha> ili bez name-only
        string treeSha;
        bool nameOnly = false;

        if (string(argv[2]) == "--name-only") {
            treeSha = argv[3];
            nameOnly = true;
        }
        else {
            treeSha = argv[2];
        }

        string subdir = treeSha.substr(0, 2);
        string hashedContent = treeSha.substr(2);
        string location = ".git/objects/" + subdir + "/" + hashedContent;

        // open file
        ifstream file;
        file.open(location, ios::binary);
        if (!file.is_open()) {
            cerr << "Could not open file." << endl;
            return EXIT_FAILURE;
        }

        //get compressed content - from the binary file to string
        stringstream buffer;
        buffer << file.rdbuf();
        string compressedContent = buffer.str();

        // unzip
        string content = Util::decompressFile(compressedContent);

        if (nameOnly) {
            Util::parseTreeContent(content, true);
        }
        else {
            Util::parseTreeContent(content, false);
        } 
     }
     else if (command == "write-tree") {
        
     }
     
     else {
         std::cerr << "Unknown command " << command << '\n';
         return EXIT_FAILURE;
     }
    
     return EXIT_SUCCESS;
}
