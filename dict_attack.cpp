#include "dict_attack.hpp"
#include <iostream>

int main(int argc, char* argv[]) {
    if (argc == 1) {
        run_interactive_menu();
        return 0;
    }

    Config cfg = parse_args(argc, argv);

    if (!cfg.hash_word.empty()) {
        std::cout << hash_single_word(cfg.hash_word) << "\n";
        return 0;
    }

    if (cfg.wordlist_path.empty() || cfg.target_hex.empty() ||
        (cfg.mode != "seq" && cfg.mode != "omp")) {
        print_usage(argv[0]);
        return 1;
    }

    return run_benchmark(cfg) ? 0 : 1;
}