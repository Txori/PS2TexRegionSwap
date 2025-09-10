/*
    PS2TexRegionSwap, by Etienne Bégué - www.txori.com
    Convert Shadow of the Colossus' remastered texture by Sad Origami
    from PAL (SCES-53326) to NTSC (SCUS-97472) version

    You will need json library
    Download it from https://github.com/nlohmann/json
    and unzip it in include/nlohmann/json

    Compile using w64devkit (https://github.com/skeeto/w64devkit):
    g++ -O3 -std=c++17 -Iinclude -o PS2TexRegionSwap.exe PS2TexRegionSwap.cpp
*/

#include <iostream>
#include <filesystem>
#include <string>
#include <unordered_set>
#include <conio.h>  // For _getch()
#include "include/nlohmann/json.hpp" // For parsing json
#include <fstream>
#include <unordered_map>

using json = nlohmann::json;

struct TextureMapping {
    std::unordered_map<std::string, std::string> langs; // ES, EN, FR, DE, IT
    bool skip = false;
    std::string description;
};

// Map: NTSC filename → TextureMapping
std::unordered_map<std::string, TextureMapping> mapping;

// Folder paths
const std::string SOURCE_DIR = "SCES-53326/replacements";
const std::string DESTINATION_DIR = "SCUS-97472/replacements";

namespace fs = std::filesystem;

// Function to display an exit message and wait for user input
void exitMessage() {
    std::cout << std::endl;
    std::cout << "Press any key to exit" << std::endl;
#ifdef _WIN32
    system("pause >nul");
#else
    system("read -n 1 -s -r -p \"\"");
#endif
}

// Function to convert dds file nomenclature from PAL to NTSC
std::string convertFileName(const std::string& filename) {
    std::string prefix = filename.substr(0, filename.size() - 8); // Remove the last 8 characters
    int middleNum = filename[filename.size() - 8] - '0' - 4; // Convert middle character to integer and subtract 4
    std::string newMiddleChar = std::to_string(middleNum); // Convert the result back to string
    std::string suffix = filename.substr(filename.size() - 7); // Get the last 7 characters
    return prefix + newMiddleChar + suffix; // Assemble the new filename
}


void displayMenu(int currentOption) {
    const char* languages[] = {"English", "Spanish", "French", "Deutsch", "Italian"};
    
    std::cout << "\nSelect your language:\n";
    for (int i = 0; i < 5; ++i) {
        if (i == currentOption) {
            std::cout << "> ";  // Highlight current option with ">"
        } else {
            std::cout << "  ";  // No highlight
        }
        std::cout << languages[i] << std::endl;
    }
}


int selectLanguage() {
    int currentOption = 0;
    int key;

    // Initial display
    displayMenu(currentOption);

    while (true) {
        key = _getch();  // Get a single keypress (no need to press Enter)
        
        if (key == 224) {  // Arrow keys return a two-part code: first 224
            key = _getch();  // Get the second part of the arrow key code
            if (key == 72) {  // Up arrow
                if (currentOption > 0) {
                    currentOption--;
                }
            } else if (key == 80) {  // Down arrow
                if (currentOption < 4) {
                    currentOption++;
                }
            }
        } else if (key == 13) {  // Enter key
            return currentOption + 1;  // Return the selected language (1-based index)
        }

        // Redraw the menu with the current option highlighted
        system("cls");  // Clear the console (Windows-specific)
        displayMenu(currentOption);
    }
}


void copyAndRenameFiles(const std::string& sourceDir, const std::string& destinationDir, const std::string& langCode) {
    if (!fs::exists(sourceDir)) {
        std::cerr << "Error: Source directory '" << sourceDir << "' does not exist." << std::endl;
        exitMessage();
        return;
    }

    for (const auto& entry : fs::recursive_directory_iterator(sourceDir)) {
        if (!entry.is_regular_file()) continue;

        std::string sourceFilePath = entry.path().string();
        std::string originalFilename = entry.path().filename().string();
        std::string extension = entry.path().extension().string();
        std::string baseFilename = entry.path().stem().string(); // filename without extension
        std::string relativeDir = entry.path().parent_path().lexically_relative(fs::path(sourceDir)).string();

        std::string newFilename = "";
        bool foundInJson = false;
        bool shouldSkip = false;

        std::cout << "\n--- Processing file: " << originalFilename << " ---\n";

        // Iterate through all NTSC entries in the JSON mapping
        for (auto& [ntscName, data] : mapping) {
            for (auto& [fileLang, palName] : data.langs) {
                if (palName == baseFilename) {
                    foundInJson = true;
                    std::cout << "Matched JSON entry -> NTSC key: " << ntscName
                              << " | Lang: " << fileLang
                              << " | skip=" << (data.skip ? "true" : "false")
                              << " | desc=" << data.description << "\n";

                    // If this entry is flagged "skip", always skip and stop searching
                    if (data.skip) {
                        shouldSkip = true;
                        std::cout << ">>> SKIPPING due to skip=true\n";
                        goto foundMatch;
                    }

                    // Otherwise, apply language filtering
                    if (fileLang == langCode) {
                        newFilename = ntscName + extension;
                        std::cout << ">>> Language match! Will rename to " << newFilename << "\n";
                    } else {
                        shouldSkip = true;
                        std::cout << ">>> SKIPPING because wrong language (expected " << langCode << ")\n";
                    }
                    goto foundMatch; // break out of both loops
                }
            }
        }

    foundMatch:;

        // Fallback: if not in JSON and not skipped
        if (!foundInJson && !shouldSkip) {
            newFilename = convertFileName(originalFilename);
            std::cout << "No JSON match, fallback convertFileName -> " << newFilename << "\n";
        }

        // Copy only if allowed and we have a destination name
        if (!shouldSkip && !newFilename.empty()) {
            std::string destinationFilePath = (fs::path(destinationDir) / fs::path(relativeDir) / newFilename).string();
            try {
                fs::create_directories(fs::path(destinationFilePath).parent_path());
                fs::copy_file(sourceFilePath, destinationFilePath, fs::copy_options::overwrite_existing);
                std::cout << ">>> COPIED: " << originalFilename << " -> " << newFilename << "\n";
            } catch (const std::exception& e) {
                std::cerr << "Error copying file: " << e.what() << std::endl;
            }
        } else {
            std::cout << ">>> NOT COPIED (skip=" << (shouldSkip ? "true" : "false")
                      << ", newFilename=" << (newFilename.empty() ? "empty" : newFilename) << ")\n";
        }
    }
}


void loadJsonMapping(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        std::cerr << "Cannot open JSON mapping file.\n";
        return;
    }

    json j;
    file >> j;

    for (auto it = j.begin(); it != j.end(); ++it) {
        std::string ntsc = it.key();
        TextureMapping tm;

        // read description if exists
        if (it.value().contains("label")) {
            tm.description = it.value()["label"];
        }

        // read skip flag if exists
        if (it.value().contains("skip")) {
            tm.skip = it.value()["skip"].get<bool>();
        }

        // read language mappings
        for (auto& lang : {"EN", "ES", "FR", "DE", "IT"}) {
            if (it.value().contains(lang)) {
                tm.langs[lang] = it.value()[lang].get<std::string>();
            }
        }

        mapping[ntsc] = tm;
    }
}


int main() {
    int languageIndex = selectLanguage() - 1;
    const std::vector<std::string> langCodes = {"EN", "ES", "FR", "DE", "IT"};
    std::string langCode = langCodes[languageIndex];

    loadJsonMapping("maps/SCES-53326.json");

    copyAndRenameFiles(SOURCE_DIR, DESTINATION_DIR, langCode);

    exitMessage();
    return 0;
}
