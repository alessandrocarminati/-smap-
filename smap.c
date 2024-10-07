#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <inttypes.h>

#define MAX_SYMBOLS 1000000

typedef struct {
	uint64_t start_address;
	uint64_t end_address;
	char symbol_name[256];
} Symbol;

Symbol symbols[MAX_SYMBOLS];
int symbol_count = 0;

int compare_symbols(const void *a, const void *b) {
	uint64_t addr_a = ((Symbol *)a)->start_address;
	uint64_t addr_b = ((Symbol *)b)->start_address;
	return (addr_a > addr_b) - (addr_a < addr_b);
}

void parse_symbol_file(const char *file_name) {
	uint64_t prev_address = 0;
	char symbol_name[256];
	uint64_t address;
	char line[256];
	FILE *file;
	char type;


	file = fopen(file_name, "r");
	if (!file) {
		perror("Failed to open System.map");
		exit(EXIT_FAILURE);
	}

	while (fgets(line, sizeof(line), file)) {

		if (sscanf(line, "%" SCNx64 " %c %s", &address, &type, symbol_name) == 3) {
			if ((type=='T') || (type=='t')) {
				if (symbol_count >= MAX_SYMBOLS) {
					fprintf(stderr, "Too many symbols, increase MAX_SYMBOLS.\n");
					exit(EXIT_FAILURE);
				}

				symbols[symbol_count].start_address = address;
				strcpy(symbols[symbol_count].symbol_name, symbol_name);
				symbols[symbol_count].end_address = 0;
				symbol_count++;
			}
		}
	}

	fclose(file);

	qsort(symbols, symbol_count, sizeof(Symbol), compare_symbols);

	for (int i = 0; i < symbol_count - 1; i++) {
		symbols[i].end_address = symbols[i + 1].start_address;
	}
	symbols[symbol_count - 1].end_address = UINT64_MAX;  // The last symbol's end address
}
const char *find_symbol(uint64_t address) {
	int i;
	for (i = 0; i < symbol_count; i++) {
		if (address >= symbols[i].start_address && address < symbols[i].end_address) {
			return symbols[i].symbol_name;
		}
	}
	return NULL;
}

int main(int argc, char *argv[]) {
	const char *symbol;
	char *system_map_file = NULL;
	uint64_t search_address = 0;
	int opt;

	while ((opt = getopt(argc, argv, "f:a:")) != -1) {
		switch (opt) {
			case 'f':
				system_map_file = optarg;
				break;
			case 'a':
				search_address = strtoull(optarg, NULL, 16);
				break;
			default:
				fprintf(stderr, "Usage: %s -f <System.map file> -a <hex address>\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	if (!system_map_file || search_address == 0) {
		fprintf(stderr, "Usage: %s -f <System.map file> -a <hex address>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	parse_symbol_file(system_map_file);

	symbol = find_symbol(search_address);
	if (symbol) {
		printf("0x%" PRIx64 " -> %s\n", search_address, symbol);
	} else {
		printf("0x%" PRIx64 " not found\n", search_address);
	}

	return 0;
}
