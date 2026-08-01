#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "rkimgman.h"
#include <getopt.h>

void print_usage(const char *prog) {
    printf("Usage: %s <subcommand> [options]\n", prog);
    printf("Subcommands:\n");
    printf("  unpack <input> <output_dir>\n");
    printf("  pack-rkfw <input_dir> <output_file> [options]\n");
    printf("  pack-rkaf <input_dir> <output_file> -m <model> -M <manufacturer>\n");
}

int main(int argc, char **argv) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    const char *subcommand = argv[1];

    if (strcmp(subcommand, "unpack") == 0) {
        if (argc != 4) {
            printf("Usage: %s unpack <input_file> <output_dir>\n", argv[0]);
            return 1;
        }
        return unpack_file(argv[2], argv[3]);
    } else if (strcmp(subcommand, "pack-rkfw") == 0) {
        if (argc < 4) {
            printf("Usage: %s pack-rkfw <input_dir> <output_file> [-c chip] [-v version]\n", argv[0]);
            return 1;
        }
        const char *input = argv[2];
        const char *output = argv[3];
        const char *chip = NULL;
        const char *version = NULL;
        // basic argparse
        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "-c") == 0 && i + 1 < argc) chip = argv[++i];
            else if (strcmp(argv[i], "-v") == 0 && i + 1 < argc) version = argv[++i];
        }
        return pack_rkfw(input, output, chip, version, 0, NULL);
    } else if (strcmp(subcommand, "pack-rkaf") == 0) {
        if (argc < 6) {
            printf("Usage: %s pack-rkaf <input_dir> <output_file> -m <model> -M <manufacturer>\n", argv[0]);
            return 1;
        }
        const char *input = argv[2];
        const char *output = argv[3];
        const char *model = "Unknown";
        const char *manufacturer = "Unknown";
        for (int i = 4; i < argc; i++) {
            if (strcmp(argv[i], "-m") == 0 && i + 1 < argc) model = argv[++i];
            else if (strcmp(argv[i], "-M") == 0 && i + 1 < argc) manufacturer = argv[++i];
        }
        return pack_rkaf(input, output, model, manufacturer);
    } else {
        printf("Unknown subcommand: %s\n", subcommand);
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}
