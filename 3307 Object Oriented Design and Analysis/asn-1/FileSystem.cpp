/*
*Author: Tennyson Hung
*Description: This is the cpp file for the FileSystem class, containing the implementation of the classes functions as defined in the header.
*Date: 2026-02-03
 * */

#include "FileSystem.h"
#include <filesystem>
#include <iostream>
#include <vector>
#include <string>
#include <ctime>
#include <chrono>
#include <sstream>
#include <iomanip>
#include <fstream>


namespace fs = std::filesystem;
using namespace std;

/*
 *Name: FileSystem
 *Description: Constructor for the FileSystem class, initializes variables of the class.
 *Parameters: None
 *Return: None
 * */
FileSystem::FileSystem() {
    try {
        currDirectory = fs::current_path();
	clipboardPath = fs::path();
        isCutOp = false;
        hasClipboardData = false;
    } catch (const fs::filesystem_error& e) {
        cerr << "Critical Error: Could not access current path: " << e.what() << endl;
        currDirectory = "/"; // Fallback
    }
}

/*
 *Name: SetCurrentDirectory
 *Description: This function sets the current directory to the the given one
 *Parameters: std::string directory, the name of the direcotry to be switched too
 *Return: Return true if successful, false otherwise
 * */
bool FileSystem::SetCurrentDirectory(std::string directory) {
    try {
        //Use absolute to get the absolute file path for the desired directory
	fs::path targetPath = fs::absolute(currDirectory / directory);

        if (fs::exists(targetPath) && fs::is_directory(targetPath)) {
            //clean the path, remove "." or ".." if they are in the path
            currDirectory = fs::canonical(targetPath);
            return true;
        }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Error changing directory: " << e.what() << std::endl;
    }
    return false;
}

/*
 *Name: GetCurrentDirectoryFiles
 *Description: This function gets a list of all files in the current directory
 *Parameters: None
 *Return: Returns a vector contianing all files and their information int the current directory
 * */
vector<FileInfo> FileSystem::GetCurrentDirectoryFiles() {
    vector<FileInfo> files;
    try {
        for (const auto& entry : fs::directory_iterator(currDirectory)) {
            FileInfo info;
            info.name = entry.path().filename().string();
            info.filePath = entry.path(); 
            
            // Safe check for file type
            if (fs::is_regular_file(entry)) {
                info.size = fs::file_size(entry);
                info.type = "File";
            } else {
                info.size = 0;
                info.type = fs::is_directory(entry) ? "Dir" : "Other";
            }

            info.dateModified = getFileTime(entry.path());
            files.push_back(info);
        }
    } catch (const fs::filesystem_error& e) {
        cerr << "Error reading directory: " << e.what() << endl;
    }
    return files;
}

/*
 *Name: getFileTime
 *Description: This function, gets the file's most recent modification time and date
 *Parameters: fs::path file, is the file path of the file to get the time and date for
 *Return: returns a string containing the files most recent modifation time and date
 *Note: The code for this function is cited in the header file
 * */
string FileSystem::getFileTime(fs::path file) {
    try {
        if (fs::exists(file)) {
            fs::file_time_type file_time = fs::last_write_time(file);
            std::time_t tt = to_time_t(file_time);
            std::tm *gmt = std::gmtime(&tt);
            std::stringstream buffer;
            buffer << std::put_time(gmt, "%A, %d %B %Y %H:%M:%S");
	    return buffer.str();
        }
    } catch (const fs::filesystem_error& e) {
        return "Access Denied";
    }
    return "File Not Found";
}

/*
 *Name: DeleteFile
 *Description: Deletes a specified file, in the current directory
 *Parameters: fs::path file, the path of the file to be deleted
 *Return: Returns true if successful, false otherwise.
 * */
bool FileSystem::DeleteFile(fs::path file) {
    try {
	
        fs::path fullPath = fs::absolute(GetCurrentDirectory() / file);
        if (fs::exists(fullPath) && !fs::is_directory(fullPath)) {
            return fs::remove(fullPath);
        }
    } catch (const fs::filesystem_error& e) {
        cerr << "Error deleting file: " << e.what() << endl;
    }
    return false;
}


/*
 *Name: RenameFile
 *Description: Renames a file or directry to a new name
 *Parameters: fs::path file, the path of the file to be renamed
 *            string newName, the new name for the file/directory
 *Return: Return true if successful, false otherwise
 * */
bool FileSystem::RenameFile(fs::path file, string newName) {
    try {
        

        fs::path oldPath = fs::absolute(GetCurrentDirectory() / file);
        
        // create the new name, and ensure it stays in the same folder
        fs::path newPath = oldPath.parent_path() / newName;

        if (!fs::exists(oldPath)) {
            std::cerr << "Rename Error: Source file not found." << std::endl;
            return false;
        }

        //Check for name collision and allow if it's a case change on same file
        if (fs::exists(newPath) && !fs::equivalent(oldPath, newPath)) {
            std::cerr << "Rename Error: Destination name already exists." << std::endl;
            return false;
        }

        fs::rename(oldPath, newPath);
        return true;    

    } catch (const fs::filesystem_error& e) {
        cerr << "Error renaming file: " << e.what() << endl;
    }
    return false;
}

/*
 *Name: CreateNewDirectory
 *Description: Creates a new directory
 *Parameters: std::string folderName, is the name of the new directory
 *Return: Returns true if successful, false otherwise
 * */
bool FileSystem::CreateNewDirectory(std::string folderName) {
    try {
        fs::path newPath = GetCurrentDirectory() / folderName;
	//if directoru already exsists, stop the operation
        if (fs::exists(newPath)) return false; 
        return fs::create_directory(newPath);
    } catch (const fs::filesystem_error& e) {
        return false;
    }
}

/*
 *Name: SetClipboard
 *Description: Sets the clipboard/stores the file path of the file we want to copy
 *Parameters: fs::path source, this is the path of the file we want to copy
 *            bool isCut, when it is true, we know we have preformed the cut operation, false for copy operation
 *Return: None
 * */
void FileSystem::SetClipboard(fs::path source, bool isCut) {
    clipboardPath = source;
    isCutOp = isCut;
    hasClipboardData = true;
}

/*
 *Name: Paste 
 *Description: This function pastes the file from the clipboard to the current directory
 *Parameters: fs::path targetFolder, this is the target folder for the file to be pasted in to,
 * 	      bool overwrite, when true if there is a file with the same name, we overwrite it during the paste, if false we don't overwrite
 *Return: Returns true if successful, false otherwise
 * */
bool FileSystem::Paste(fs::path targetFolder, bool overwrite) {
    //if clipboard is empty nothing to paste
    if (!hasClipboardData) return false;

    if(!fs::exists(clipboardPath)) return false;
 

    fs::path destination = targetFolder / clipboardPath.filename();

    //prevent a directory to be pasted into itself
    if (clipboardPath == destination) return true;

    try {
        if (isCutOp) {
            // when isCutOp is true, then we rename/move the copied file
            if (fs::exists(destination)) {
                if (!overwrite) return false; // Stop when we don't want to overwrite
                fs::remove_all(destination);  // Remove existing so rename succeeds and we can overwrite
            }
            fs::rename(clipboardPath, destination);
            ClearClipboard();
        } else {
            // when isCutOp is false, we copy the file
            if (fs::exists(destination)) {
		if(!overwrite) return false; // Stop when we don't want to overwrite 
	    
            fs::copy(clipboardPath, destination, fs::copy_options::overwrite_existing);
            ClearClipboard();
	}
        return true;
    }
    } catch (const fs::filesystem_error& e) {
        std::cerr << "Paste Error: " << e.what() << std::endl;
        return false;
    }
}

/*
 *Name: ClearClipboard
 *Description: This function clears the virtual clippboard
 *Parameters: None
 *Return: None  
 * */
void FileSystem::ClearClipboard(){

  hasClipboardData = false; 
  clipboardPath = fs::path();
}




