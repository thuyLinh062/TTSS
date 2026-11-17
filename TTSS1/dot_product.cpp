#include "dot_product.hpp"

int main(int argc, char* argv[]) {
    if (argc == 1) {
        run_interactive_menu();
        return 0;
    }

    Config cfg = parse_args(argc, argv);

    if (cfg.gen_size > 0) {
        if (cfg.gen_file.empty()) {
            print_usage(argv[0]);
            return 1;
        }
        return generate_binary_data(cfg.gen_file, cfg.gen_size) ? 0 : 1;
    }

    if (cfg.input_file.empty() || (cfg.mode != "seq" && cfg.mode != "omp")) {
        print_usage(argv[0]);
        return 1;
    }

    return run_benchmark(cfg) ? 0 : 1;
}