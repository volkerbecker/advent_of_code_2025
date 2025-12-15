#include "load_data.h"

#include <algorithm>
#include <array>
#include <cctype>
#include <fstream>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string_view>

namespace {
std::string trim(const std::string& value) {
    const auto begin = value.find_first_not_of(" \t");
    if (begin == std::string::npos) {
        return {};
    }
    const auto end = value.find_last_not_of(" \t");
    return value.substr(begin, end - begin + 1);
}

bool is_digits(const std::string& value) {
    return !value.empty() && std::all_of(value.begin(), value.end(), [](unsigned char ch) {
        return std::isdigit(ch) != 0;
    });
}

bool is_region_descriptor(const std::string& line) {
    const auto colon_pos = line.find(':');
    const auto x_pos = line.find('x');
    if (colon_pos == std::string::npos || x_pos == std::string::npos || x_pos > colon_pos) {
        return false;
    }

    const auto dims = trim(line.substr(0, colon_pos));
    const auto sep = dims.find('x');
    if (sep == std::string::npos) {
        return false;
    }

    const auto width_part = trim(dims.substr(0, sep));
    const auto height_part = trim(dims.substr(sep + 1));
    return is_digits(width_part) && is_digits(height_part);
}

bool is_shape_header(const std::string& line) {
    const auto colon_pos = line.find(':');
    if (colon_pos == std::string::npos) {
        return false;
    }

    if (line.find('x') != std::string::npos && line.find('x') < colon_pos) {
        return false;
    }

    const auto id_part = trim(line.substr(0, colon_pos));
    return is_digits(id_part);
}

void append_shape(std::vector<Shape>& shapes, int id, const std::vector<std::string>& rows) {
    if (rows.empty()) {
        throw std::runtime_error("Shape " + std::to_string(id) + " has no rows");
    }

    const size_t width = rows.front().size();
    if (width == 0) {
        throw std::runtime_error("Shape " + std::to_string(id) + " has zero width");
    }

    std::vector<char> buffer;
    buffer.reserve(width * rows.size());
    for (const auto& row : rows) {
        if (row.size() != width) {
            throw std::runtime_error("Shape " + std::to_string(id) + " rows differ in width");
        }
        buffer.insert(buffer.end(), row.begin(), row.end());
    }

    shapes.push_back(Shape{ id, tools::CharMatrix(rows.size(), width, std::move(buffer)) });
}

std::string matrix_signature(const tools::CharMatrix& matrix) {
    std::string signature;
    signature.reserve(matrix.num_rows() * (matrix.num_cols() + 1) + 32);
    signature.append(std::to_string(matrix.num_rows()));
    signature.push_back('x');
    signature.append(std::to_string(matrix.num_cols()));
    signature.push_back(':');

    for (size_t r = 0; r < matrix.num_rows(); ++r) {
        const auto row = matrix.get_row(r);
        signature.append(row.begin(), row.end());
        signature.push_back('|');
    }

    return signature;
}

tools::CharMatrix rotate_clockwise(const tools::CharMatrix& matrix) {
    const size_t rows = matrix.num_rows();
    const size_t cols = matrix.num_cols();
    std::vector<char> buffer(rows * cols);

    for (size_t r = 0; r < cols; ++r) {
        for (size_t c = 0; c < rows; ++c) {
            const size_t src_row = rows - 1 - c;
            const size_t src_col = r;
            buffer[r * rows + c] = matrix.get_row(src_row)[src_col];
        }
    }

    return tools::CharMatrix(cols, rows, std::move(buffer));
}

tools::CharMatrix flip_horizontal(const tools::CharMatrix& matrix) {
    const size_t rows = matrix.num_rows();
    const size_t cols = matrix.num_cols();
    std::vector<char> buffer(rows * cols);

    for (size_t r = 0; r < rows; ++r) {
        const auto source_row = matrix.get_row(r);
        for (size_t c = 0; c < cols; ++c) {
            buffer[r * cols + c] = source_row[cols - 1 - c];
        }
    }

    return tools::CharMatrix(rows, cols, std::move(buffer));
}

tools::CharMatrix flip_vertical(const tools::CharMatrix& matrix) {
    const size_t rows = matrix.num_rows();
    const size_t cols = matrix.num_cols();
    std::vector<char> buffer(rows * cols);

    for (size_t r = 0; r < rows; ++r) {
        const auto source_row = matrix.get_row(rows - 1 - r);
        for (size_t c = 0; c < cols; ++c) {
            buffer[r * cols + c] = source_row[c];
        }
    }

    return tools::CharMatrix(rows, cols, std::move(buffer));
}

tools::CharMatrix apply_operations(const tools::CharMatrix& base, std::string_view operations) {
    tools::CharMatrix transformed = base;
    for (char op : operations) {
        switch (op) {
        case 'R':
            transformed = rotate_clockwise(transformed);
            break;
        case 'H':
            transformed = flip_horizontal(transformed);
            break;
        case 'V':
            transformed = flip_vertical(transformed);
            break;
        default:
            break;
        }
    }
    return transformed;
}
} // namespace

void load_input_file(const std::string& filepath, std::vector<Shape>& shapes, std::vector<Region>& regions) {
    std::ifstream input(filepath);
    if (!input.is_open()) {
        throw std::runtime_error("Unable to open input file: " + filepath);
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        if (!line.empty() && line.back() == '\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }

    size_t idx = 0;
    const auto skip_blank = [&]() {
        while (idx < lines.size() && lines[idx].empty()) {
            ++idx;
        }
    };

    skip_blank();

    // shapes section
    while (idx < lines.size()) {
        if (lines[idx].empty()) {
            ++idx;
            continue;
        }

        if (is_region_descriptor(lines[idx])) {
            break;
        }

        if (!is_shape_header(lines[idx])) {
            throw std::runtime_error("Unexpected line in shape section: " + lines[idx]);
        }

        const auto colon_pos = lines[idx].find(':');
        const int id = std::stoi(trim(lines[idx].substr(0, colon_pos)));
        ++idx;
        skip_blank();

        std::vector<std::string> rows;
        while (idx < lines.size()) {
            if (lines[idx].empty()) {
                ++idx;
                if (!rows.empty()) {
                    break;
                }
                continue;
            }

            if (is_shape_header(lines[idx]) || is_region_descriptor(lines[idx])) {
                break;
            }

            rows.push_back(lines[idx]);
            ++idx;
        }

        append_shape(shapes, id, rows);
    }

    // region section
    while (idx < lines.size()) {
        if (lines[idx].empty()) {
            ++idx;
            continue;
        }

        if (!is_region_descriptor(lines[idx])) {
            throw std::runtime_error("Unexpected line in region section: " + lines[idx]);
        }

        const auto colon_pos = lines[idx].find(':');
        const auto dims = trim(lines[idx].substr(0, colon_pos));
        const auto sep = dims.find('x');
        const auto width_part = trim(dims.substr(0, sep));
        const auto height_part = trim(dims.substr(sep + 1));
        const size_t width = static_cast<size_t>(std::stoul(width_part));
        const size_t height = static_cast<size_t>(std::stoul(height_part));

        std::vector<int> to_integrate;
        std::istringstream ids_stream(lines[idx].substr(colon_pos + 1));
        int id_value = 0;
        while (ids_stream >> id_value) {
            to_integrate.push_back(id_value);
        }

        regions.push_back(Region{ width, height, std::move(to_integrate) });
        ++idx;
    }
}

void Shape::build_derived_shapes() {
    derived_shapes.clear();

    std::set<std::string> seen;
    auto add_variant = [&](tools::CharMatrix candidate) {
        if (seen.insert(matrix_signature(candidate)).second) {
            derived_shapes.push_back(std::move(candidate));
        }
    };

    add_variant(shape);

    constexpr std::array<std::string_view, 4> rotations{ "", "R", "RR", "RRR" };
    constexpr std::array<std::string_view, 4> flips{ "", "H", "V", "HV" };

    for (const auto flip_ops : flips) {
        for (const auto rot_ops : rotations) {
            std::string sequence;
            sequence.reserve(flip_ops.size() + rot_ops.size());
            sequence.append(flip_ops);
            sequence.append(rot_ops);
            if (sequence.empty()) {
                continue;
            }
            add_variant(apply_operations(shape, sequence));
        }
    }
}
