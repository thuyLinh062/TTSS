#include "dict_attack.hpp"

#include <cstdlib>
#include <iostream>
#include <limits>

void print_usage(const char* prog) {
    std::cerr << "Cach dung:\n"
        << "  1. Tao ma hash mau:  " << prog << " --hash-word <mat_khau>\n"
        << "  2. Chay thu nghiem:  " << prog << " --mode <seq|omp> --threads <P> --wordlist <file.txt> --hash <sha256_hex> [--csv]\n"
        << "  3. Che do tuong tac: Chay truc tiep khong kem tham so\n";
}

Config parse_args(int argc, char* argv[]) {
    Config cfg;
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--mode" && i + 1 < argc) cfg.mode = argv[++i];
        else if (arg == "--threads" && i + 1 < argc) cfg.threads = std::stoi(argv[++i]);
        else if (arg == "--wordlist" && i + 1 < argc) cfg.wordlist_path = argv[++i];
        else if (arg == "--hash" && i + 1 < argc) cfg.target_hex = argv[++i];
        else if (arg == "--hash-word" && i + 1 < argc) cfg.hash_word = argv[++i];
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
        std::cout << "\n=============================================\n"
            << "   MENU CHUONG TRINH TRA CUU SHA-256 (TTSS2)\n"
            << "=============================================\n"
            << "  1. Tao ma hash SHA-256 mau tu chuoi\n"
            << "  2. Chay thu nghiem (Sequential / OpenMP)\n"
            << "  0. Thoat\n"
            << "---------------------------------------------\n"
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
            std::string word;
            std::cout << "Nhap chuoi can bam SHA-256 (vd: 123456): ";
            std::getline(std::cin >> std::ws, word);

            std::cout << "Ma SHA-256: " << hash_single_word(word) << "\n";
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

            std::cout << "Nhap ten file tu dien dau vao (vd: wordlist.txt): ";
            std::getline(std::cin >> std::ws, cfg.wordlist_path);

            std::cout << "Nhap chuoi hash SHA-256 muc tieu (64 ky tu hex): ";
            std::getline(std::cin >> std::ws, cfg.target_hex);

            run_benchmark(cfg);
        }
        else {
            std::cout << "Lua chon khong hop le, vui long chon lai!\n";
        }
    }
}