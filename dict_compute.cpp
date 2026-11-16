#include "dict_attack.hpp"
#include "sha256.hpp"

#include <atomic>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <omp.h>

using Clock = std::chrono::steady_clock;

std::string hash_single_word(const std::string& word) {
    std::array<uint8_t, 32> digest;
    SHA256::hash_bytes(reinterpret_cast<const uint8_t*>(word.data()),
        word.size(), digest);
    return SHA256::bytes_to_hex(digest);
}

int64_t attack_sequential(const std::vector<std::string_view>& words,
    const std::array<uint8_t, 32>& target_digest) {
    std::array<uint8_t, 32> digest;
    const int64_t n = static_cast<int64_t>(words.size());
    for (int64_t i = 0; i < n; ++i) {
        SHA256::hash_bytes(reinterpret_cast<const uint8_t*>(words[i].data()),
            words[i].size(), digest);
        if (digest == target_digest) {
            return i;
        }
    }
    return -1;
}

int64_t attack_openmp(const std::vector<std::string_view>& words,
    const std::array<uint8_t, 32>& target_digest) {
    std::atomic<bool> found{ false };
    std::atomic<int64_t> found_idx{ -1 };
    const int64_t n = static_cast<int64_t>(words.size());

#pragma omp parallel
    {
        // Moi luong co bo dem digest rieng tren stack -> khong race condition, khong false sharing
        std::array<uint8_t, 32> local_digest;

#pragma omp for schedule(static)
        for (int64_t i = 0; i < n; ++i) {
            if (found.load(std::memory_order_relaxed)) {
                continue;
            }
            SHA256::hash_bytes(reinterpret_cast<const uint8_t*>(words[i].data()),
                words[i].size(), local_digest);
            if (local_digest == target_digest) {
                found.store(true, std::memory_order_relaxed);
                int64_t expected = -1;
                found_idx.compare_exchange_strong(expected, i, std::memory_order_relaxed);
            }
        }
    }
    return found_idx.load(std::memory_order_relaxed);
}

bool run_benchmark(Config cfg) {
    if (cfg.target_hex.size() != 64) {
        std::cerr << "Loi: Chuoi hash SHA-256 phai gom dung 64 ky tu hex!\n";
        return false;
    }

    // ==================== GIAI DOAN 1: T_init ====================
    auto t0_init = Clock::now();
    if (cfg.mode == "omp") {
        omp_set_num_threads(cfg.threads);
    }
    else {
        cfg.threads = 1;
    }
    std::array<uint8_t, 32> target_digest = SHA256::hex_to_bytes(cfg.target_hex);
    std::string raw_buffer;
    std::vector<std::string_view> words;
    auto t1_init = Clock::now();

    // ==================== GIAI DOAN 2: T_IO ====================
    auto t0_io = Clock::now();
    std::ifstream in(cfg.wordlist_path, std::ios::binary | std::ios::ate);
    if (!in) {
        std::cerr << "Loi: Khong the mo file wordlist: " << cfg.wordlist_path << "\n";
        return false;
    }
    std::streamsize file_size = in.tellg();
    in.seekg(0, std::ios::beg);

    raw_buffer.resize(static_cast<size_t>(file_size));
    if (file_size > 0) {
        in.read(&raw_buffer[0], file_size);
    }
    in.close();

    // Tach tung dong thanh string_view (xu ly ca dinh dang dong \n va \r\n)
    words.reserve(static_cast<size_t>(file_size / 8) + 1);
    size_t start = 0;
    for (size_t i = 0; i < raw_buffer.size(); ++i) {
        if (raw_buffer[i] == '\n') {
            size_t end = (i > start && raw_buffer[i - 1] == '\r') ? (i - 1) : i;
            if (end > start) {
                words.emplace_back(raw_buffer.data() + start, end - start);
            }
            start = i + 1;
        }
    }
    if (start < raw_buffer.size()) {
        words.emplace_back(raw_buffer.data() + start, raw_buffer.size() - start);
    }
    auto t1_io = Clock::now();

    // ==================== GIAI DOAN 3: T_compute ====================
    auto t0_comp = Clock::now();
    int64_t matched_idx = (cfg.mode == "seq")
        ? attack_sequential(words, target_digest)
        : attack_openmp(words, target_digest);
    auto t1_comp = Clock::now();

    double t_init = std::chrono::duration<double>(t1_init - t0_init).count();
    double t_io = std::chrono::duration<double>(t1_io - t0_io).count();
    double t_comp = std::chrono::duration<double>(t1_comp - t0_comp).count();
    double t_tot = t_init + t_io + t_comp;

    std::string cracked_pw = (matched_idx >= 0) ? std::string(words[matched_idx]) : "NOT_FOUND";

    if (cfg.csv_output) {
        std::cout << cfg.mode << "," << cfg.threads << "," << words.size() << ","
            << std::fixed << std::setprecision(6)
            << t_init << "," << t_io << "," << t_comp << "," << t_tot << ","
            << cracked_pw << "\n";
    }
    else {
        std::cout << "\n=== KET QUA BAI 2: TAN CONG TU DIEN (SHA-256) ===\n"
            << "Che do        : " << cfg.mode << " (Threads: " << cfg.threads << ")\n"
            << "So tu kiem tra: " << words.size() << "\n"
            << "Mat khau tim  : " << cracked_pw << "\n"
            << std::fixed << std::setprecision(6)
            << "T_init (s)    : " << t_init << "\n"
            << "T_IO (s)      : " << t_io << "\n"
            << "T_compute (s) : " << t_comp << "\n"
            << "T_total (s)   : " << t_tot << "\n";
    }
    return true;
}