#pragma once
#include <vector>
#include <filesystem>
#include <fstream>
#include <string>
#include <algorithm>
#include <stdexcept>
#include <span>

namespace tools {
    template<class T>
    class Matrix {
    public:
        Matrix() = default;
        // basic constructor: allocate storage
        Matrix(size_t r, size_t c) : data(r* c), rows(r), cols(c) {}
        Matrix(size_t r, size_t c,T v) : data(r*c,v), rows(r), cols(c) {}

        // construct from span of elements (copied)
        Matrix(size_t r, size_t c, std::span<const T> data_) : data(data_.begin(), data_.end()), rows(r), cols(c) {
            if (data.size() != r * c)
                throw std::runtime_error("Data size does not match matrix dimensions");
        }

        // construct taking ownership of a vector
        Matrix(size_t r, size_t c, std::vector<T> data_) : data(std::move(data_)), rows(r), cols(c) {
            if (data.size() != r * c)
                throw std::runtime_error("Data size does not match matrix dimensions");
        }

        size_t num_rows() const {
            return rows;
        }
        size_t num_cols() const {
            return cols;
        }

        // return a span referencing the row (mutable)
        std::span<T> get_row(size_t r) {
            if (r >= rows) throw std::out_of_range("row index out of range");
            return std::span<T>(data.data() + r * cols, cols);
        }

        // return a span referencing the row (const)
        std::span<const T> get_row(size_t r) const {
            if (r >= rows) throw std::out_of_range("row index out of range");
            return std::span<const T>(data.data() + r * cols, cols);
        }

        // allow M[r][c] access using spans
        std::span<T> operator[](size_t r) {
            return get_row(r);
        }
        std::span<const T> operator[](size_t r) const {
            return get_row(r);
        }

        void transpose() {
            std::vector<T> transposed(data.size());
            for (size_t r = 0; r < rows; ++r) {
                for (size_t c = 0; c < cols; ++c) {
                    transposed[c * rows + r] = data[r * cols + c];
                }
			}
            data = std::move(transposed);
            std::swap(rows, cols);
		}

        std::vector<T> pop_last_row() {
            if (rows == 0) {
                throw std::runtime_error("No rows to pop");
            }
            std::vector<T> last_row(data.end() - cols, data.end());
            data.resize(data.size() - cols);
            --rows;
            return last_row;
		}

    private:
        std::vector<T> data{ 0 };
        size_t rows{};
        size_t cols{};

    };

    using CharMatrix = Matrix<char>;

    inline CharMatrix read_matrix_from_file(const std::string& filename) {
        std::ifstream is(filename);
        if (!is.is_open()) {
            throw std::runtime_error("Unable to open file: " + filename);
        }

        std::vector<std::string> lines;
        std::string line;
        while (std::getline(is, line)) {
            // remove possible CR from CRLF
            if (!line.empty() && line.back() == '\r') line.pop_back();
            lines.push_back(line);
        }

        size_t rows = lines.size();
        size_t cols = 0;
        for (const auto& l : lines) cols = std::max(cols, l.size());

        // flatten into a single vector and use the vector-taking constructor
        std::vector<char> flat;
        flat.reserve(rows * cols);
        for (size_t r = 0; r < rows; ++r) {
            const auto& l = lines[r];
            for (size_t c = 0; c < cols; ++c) {
                char ch = (c < l.size()) ? l[c] : ' ';
                flat.push_back(ch);
            }
        }

        return CharMatrix(rows, cols, std::move(flat));
    }

    void print_charmatrix(const CharMatrix& matrix) {
        for (size_t r = 0; r < matrix.num_rows(); ++r) {
            auto row = matrix.get_row(r);
            for (size_t c = 0; c < matrix.num_cols(); ++c) {
                std::cout << row[c];
            }
            std::cout << "\n";
        }
	}

    void print_charmatrix_scaled(const CharMatrix& matrix, size_t max_width = 100, char target_char = '#') {
        if (matrix.num_rows() == 0 || matrix.num_cols() == 0) {
            std::cout << "(Empty matrix)\n";
            return;
        }

        // Finde Bounding Box der '#' Zeichen
        size_t min_row = matrix.num_rows();
        size_t max_row = 0;
        size_t min_col = matrix.num_cols();
        size_t max_col = 0;
        bool found_any = false;

        for (size_t r = 0; r < matrix.num_rows(); ++r) {
            auto row = matrix.get_row(r);
            for (size_t c = 0; c < matrix.num_cols(); ++c) {
                if (row[c] == target_char) {
                    found_any = true;
                    min_row = std::min(min_row, r);
                    max_row = std::max(max_row, r);
                    min_col = std::min(min_col, c);
                    max_col = std::max(max_col, c);
                }
            }
        }

        if (!found_any) {
            std::cout << "(No '" << target_char << "' characters found)\n";
            return;
        }

        // Berechne tatsächliche Größe der relevanten Region
        size_t actual_height = max_row - min_row + 1;
        size_t actual_width = max_col - min_col + 1;

        std::cout << "Actual size: " << actual_width << "x" << actual_height 
                  << " (from [" << min_col << "," << min_row << "] to [" 
                  << max_col << "," << max_row << "])\n";

        // Berechne Skalierungsfaktor
        size_t scale_x = 1;
        size_t scale_y = 1;
        
        if (actual_width > max_width) {
            scale_x = (actual_width + max_width - 1) / max_width;
            scale_y = scale_x; // Behalte Seitenverhältnis bei
        }

        size_t output_width = (actual_width + scale_x - 1) / scale_x;
        size_t output_height = (actual_height + scale_y - 1) / scale_y;

        if (scale_x > 1) {
            std::cout << "Scaling by " << scale_x << "x" << scale_y 
                      << " to fit in " << output_width << "x" << output_height << "\n";
        }

        // Erstelle skalierte Ausgabe
        for (size_t out_r = 0; out_r < output_height; ++out_r) {
            for (size_t out_c = 0; out_c < output_width; ++out_c) {
                // Prüfe ob in diesem Ausgabe-Block ein '#' existiert
                bool has_target = false;
                
                for (size_t dy = 0; dy < scale_y && !has_target; ++dy) {
                    size_t r = min_row + out_r * scale_y + dy;
                    if (r > max_row) break;
                    
                    auto row = matrix.get_row(r);
                    for (size_t dx = 0; dx < scale_x && !has_target; ++dx) {
                        size_t c = min_col + out_c * scale_x + dx;
                        if (c > max_col) break;
                        
                        if (row[c] == target_char) {
                            has_target = true;
                        }
                    }
                }
                
                std::cout << (has_target ? target_char : '.');
            }
            std::cout << "\n";
        }
    }
}