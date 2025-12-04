#include "read_input.h"
#include <vector>
#include <fstream>
#include <iostream>
#include <filesystem>

std::vector<std::string> read_input(const std::filesystem::path& path) {
    std::ifstream input_file(path);
    if (!input_file) {
        std::cerr << "Could not open the file: " << path << std::endl;
        return {};
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input_file, line)) {
        lines.push_back(std::move(line));
    }

    return lines;
}


