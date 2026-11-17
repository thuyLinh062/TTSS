#pragma once

#include <cstdint>
#include <string>
#include <vector>

struct Config {
    std::string mode = "omp";      // "seq" hoặc "omp"
    int threads = 4;               // Số luồng OpenMP
    std::string input_file = "";   // Đường dẫn file nhị phân đầu vào
    std::string gen_file = "";     // Đường dẫn xuất file nếu chạy chế độ sinh dữ liệu
    uint64_t gen_size = 0;         // Kích thước N cần sinh
    bool csv_output = false;       // Xuất định dạng 1 dòng CSV cho script benchmark
};

// Xử lý tham số & giao diện (cli_menu.cpp)
void print_usage(const char* prog);
Config parse_args(int argc, char* argv[]);
void run_interactive_menu();

// Tính toán & xử lý dữ liệu (dot_compute.cpp)
bool generate_binary_data(const std::string& filepath, uint64_t N);
double dot_sequential(const std::vector<double>& A, const std::vector<double>& B);
double dot_openmp(const std::vector<double>& A, const std::vector<double>& B);
bool run_benchmark(Config cfg);