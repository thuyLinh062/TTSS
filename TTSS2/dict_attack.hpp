#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

struct Config {
    std::string mode = "omp";       // "seq" hoặc "omp"
    int threads = 4;                // Số luồng OpenMP
    std::string wordlist_path = ""; // Đường dẫn tệp từ điển
    std::string target_hex = "";    // Chuỗi hash SHA-256 mục tiêu (64 ký tự hex)
    std::string hash_word = "";     // Tiện ích sinh mã hash từ mật khẩu mẫu
    bool csv_output = false;
};

// Xử lý tham số & giao diện (dict_menu.cpp)
void print_usage(const char* prog);
Config parse_args(int argc, char* argv[]);
void run_interactive_menu();

// Tính toán & xử lý dữ liệu (dict_compute.cpp)
std::string hash_single_word(const std::string& word);
int64_t attack_sequential(const std::vector<std::string_view>& words,
    const std::array<uint8_t, 32>& target_digest);
int64_t attack_openmp(const std::vector<std::string_view>& words,
    const std::array<uint8_t, 32>& target_digest);
bool run_benchmark(Config cfg);