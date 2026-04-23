/*
*Author: Tennyson Hung
*Description: This file is the header file for the class FileSystem. And contains all function and variable declarations.
*Date: 2026-02-03
 * */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

#include <string>
#include <vector>
#include <filesystem>
#include <chrono>

namespace fs = std::filesystem;

//helper struct
struct FileInfo {
    std::string name;
    fs::path filePath;
    std::string type;
    std::string dateModified;
    uintmax_t size;
};

class FileSystem {
public:
    FileSystem();
    bool SetCurrentDirectory(std::string directory);
    fs::path GetCurrentDirectory() { return currDirectory; }
    std::vector<FileInfo> GetCurrentDirectoryFiles();
    std::string getFileTime(fs::path file);
    bool DeleteFile(fs::path file);
    bool RenameFile(fs::path file, std::string name); 
    bool CreateNewDirectory(std::string folderName);
    void SetClipboard(fs::path source, bool isCut); 
    bool Paste(fs::path targetFolder, bool overwrite);
    fs::path GetClipboardPath() { return clipboardPath; }
    bool HasClipboardData() { return hasClipboardData; }
    void ClearClipboard();
    bool GetIsCutOp() { return isCutOp; };
    ~FileSystem(){};

private:
    fs::path currDirectory;
    fs::path clipboardPath;
    bool isCutOp;
    bool hasClipboardData;
    
    //code for getFileTime and to_time_t functions we taken from: https://stackoverflow.com/questions/56788745/how-to-convert-stdfilesystemfile-time-type-to-a-string-using-gcc-9/58237530#58237530
    //hepler method to get the most recent date a file was modified
    //the function is implement in the header file because of the use of the template, and it is also a helper function for the getFileTime()
    template <typename TP>
    std::time_t to_time_t(TP tp) {
        using namespace std::chrono;
        auto sctp = time_point_cast<system_clock::duration>(tp - TP::clock::now()
                  + system_clock::now());
        return system_clock::to_time_t(sctp);
    }

};
#endif



