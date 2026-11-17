#include "dot_product.hpp"

#include <algorithm>
#include <chrono>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <omp.h>
#include <random>

using Clock = std::chrono::steady_clock;

bool generate_binary_data(const std::string& filepath, uint64_t N) {
    std::ofstream out(filepath, std::ios::binary);
    if (!out) {
        std::cerr << "Loi: Khong the mo file de ghi: " << filepath << "\n";
        return false;
    }
    out.write(reinterpret_cast<const char*>(&N), sizeof(uint64_t));

    std::mt19937_64 rng(42); // Cố định seed để tái lập kết quả
    std::uniform_real_distribution<double> dist(-1.0, 1.0);

    const size_t CHUNK = 1 << 20; // Ghi theo khối 1M phần tử để tiết kiệm RAM
    std::vector<double> buf(CHUNK);

    for (int vec_id = 0; vec_id < 2; ++vec_id) {
        uint64_t remaining = N;
        while (remaining > 0) {
            size_t current = static_cast<size_t>(std::min<uint64_t>(CHUNK, remaining));
            for (size_t i = 0; i < current; ++i) buf[i] = dist(rng);
            out.write(reinterpret_cast<const char*>(buf.data()), current * sizeof(double));
            remaining -= current;
        }
    }
    std::cout << "Da sinh xong file du lieu N=" << N << " tai: " << filepath << "\n";
    return true;
}

double dot_sequential(const std::vector<double>& A, const std::vector<double>& B) {
    double sum = 0.0;
    const size_t N = A.size();
    for (size_t i = 0; i < N; ++i) {
        sum += A[i] * B[i];
    }
    return sum;
}

double dot_openmp(const std::vector<double>& A, const std::vector<double>& B) {
    double sum = 0.0;
    const int64_t N = static_cast<int64_t>(A.size());
#pragma omp parallel for reduction(+:sum) schedule(static)
    for (int64_t i = 0; i < N; ++i) {
        sum += A[i] * B[i];
    }
    return sum;
}

bool run_benchmark(Config cfg) {
    // ==================== GIAI DOAN 1: T_init ====================
    auto t0_init = Clock::now();
    if (cfg.mode == "omp") {
        omp_set_num_threads(cfg.threads);
    }
    else {
        cfg.threads = 1;
    }

    std::ifstream in(cfg.input_file, std::ios::binary);
    if (!in) {
        std::cerr << "Loi: Khong the mo file input: " << cfg.input_file << "\n";
        return false;
    }
    uint64_t N = 0;
    in.read(reinterpret_cast<char*>(&N), sizeof(uint64_t));

    std::vector<double> A(N);
    std::vector<double> B(N);
    auto t1_init = Clock::now();

    // ==================== GIAI DOAN 2: T_IO ====================
    auto t0_io = Clock::now();
    in.read(reinterpret_cast<char*>(A.data()), N * sizeof(double));
    in.read(reinterpret_cast<char*>(B.data()), N * sizeof(double));
    in.close();
    auto t1_io = Clock::now();

    // ==================== GIAI DOAN 3: T_compute ====================
    auto t0_comp = Clock::now();
    double result = (cfg.mode == "seq") ? dot_sequential(A, B) : dot_openmp(A, B);
    auto t1_comp = Clock::now();

    double t_init = std::chrono::duration<double>(t1_init - t0_init).count();
    double t_io = std::chrono::duration<double>(t1_io - t0_io).count();
    double t_comp = std::chrono::duration<double>(t1_comp - t0_comp).count();
    double t_tot = t_init + t_io + t_comp;

    if (cfg.csv_output) {
        std::cout << cfg.mode << "," << cfg.threads << "," << N << ","
            << std::fixed << std::setprecision(6)
            << t_init << "," << t_io << "," << t_comp << "," << t_tot << ","
            << std::setprecision(10) << result << "\n";
    }
    else {
        std::cout << "\n=== KET QUA BAI 1: TICH VO HUONG ===\n"
            << "Che do       : " << cfg.mode << " (Threads: " << cfg.threads << ")\n"
            << "Kich thuoc N : " << N << "\n"
            << "Ket qua      : " << std::fixed << std::setprecision(10) << result << "\n"
            << std::setprecision(6)
            << "T_init (s)   : " << t_init << "\n"
            << "T_IO (s)     : " << t_io << "\n"
            << "T_compute (s): " << t_comp << "\n"
            << "T_total (s)  : " << t_tot << "\n";
    }
    return true;
}