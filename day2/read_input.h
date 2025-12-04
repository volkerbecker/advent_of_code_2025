#pragma once
#include <string>
#include <vector>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <utility>
#include <stdexcept>

std::vector<std::string> read_input(const std::filesystem::path& path);
std::vector<std::pair<int64_t, int64_t>> read_pairs(const std::filesystem::path& path);