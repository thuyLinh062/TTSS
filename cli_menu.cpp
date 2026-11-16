#include "dot_product.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>

void print_usage(const char* prog) {
    std::cerr << "Cach dung:\n"
        << "  1. Sinh du lieu mau: " << prog << " --gen <N> --output <file.bin>\n"
        << "  2. Chay thu nghiem:  " << prog << " --mode <seq|omp> --threads <P> --input <file.bin> [--csv]\n"
        << "  3. Che do tuong tac: Chay truc tiep khong kem tham so\n";
}

Config parse_args(int argc, char* argv[]) {
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--mode" && i + 1 < argc) cfg.mode = argv[++i];
        else if (arg == "--threads" && i + 1 < argc) cfg.threads = std::stoi(argv[++i]);
        else if (arg == "--input" && i + 1 < argc) cfg.input_file = argv[++i];
        else if (arg == "--output" && i + 1 < argc) cfg.gen_file = argv[++i];
        else if (arg == "--gen" && i + 1 < argc) cfg.gen_size = std::stoull(argv[++i]);
        else if (arg == "--csv") cfg.csv_output = true;
        else {
            print_usage(argv[0]);
            std::exit(1);
        }
    }
    return cfg;
}

void run_interactive_menu() {
    while (true) {
        std::cout << "\n==========================================\n"
            << "   MENU CHUONG TRINH TICH VO HUONG (TTSS1)\n"
            << "==========================================\n"
            << "  1. Sinh du lieu mau (.bin)\n"
            << "  2. Chay thu nghiem (Sequential / OpenMP)\n"
            << "  0. Thoat\n"
            << "------------------------------------------\n"
            << "Nhap lua chon (0-2): ";

        int choice = -1;
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::cin.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
            std::cout << "Vui long chi nhap so (0, 1 hoac 2)!\n";
            continue;
        }

        if (choice == 0) {
            std::cout << "Da thoat chuong trinh.\n";
            break;
        }
        else if (choice == 1) {
            uint64_t N = 0;
            std::string filepath;
            std::cout << "Nhap so luong phan tu N (vd: 10000000): ";
            std::cin >> N;
            std::cout << "Nhap ten file xuat ra (vd: data.bin): ";
            std::getline(std::cin >> std::ws, filepath);

            generate_binary_data(filepath, N);
        }
        else if (choice == 2) {
            Config cfg;
            int mode_choice = 2;
            std::cout << "Chon che do chay (1: seq - Tuan tu | 2: omp - Da luong): ";
            std::cin >> mode_choice;

            if (mode_choice == 1) {
                cfg.mode = "seq";
                cfg.threads = 1;
            }
            else {
                cfg.mode = "omp";
                std::cout << "Nhap so luong threads OpenMP (vd: 4): ";
                std::cin >> cfg.threads;
            }

            std::cout << "Nhap ten file du lieu dau vao (vd: data.bin): ";
            std::getline(std::cin >> std::ws, cfg.input_file);

            run_benchmark(cfg);
        }
        else {
            std::cout << "Lua chon khong hop le, vui long chon lai!\n";
        }
    }
}