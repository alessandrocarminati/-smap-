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

void parse_system_map(const char *file_name) {
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
				if (symbol_count > 0) {
					symbols[symbol_count - 1].end_address = address;
				}

				symbols[symbol_count].start_address = address;
				strcpy(symbols[symbol_count].symbol_name, symbol_name);
				symbols[symbol_count].end_address = 0;
				symbol_count++;
			}
		}

		if (symbol_count >= MAX_SYMBOLS) {
			fprintf(stderr, "Too many symbols, increase MAX_SYMBOLS.\n");
			exit(EXIT_FAILURE);
		}
	}

	symbols[symbol_count - 1].end_address = UINT64_MAX;

	fclose(file);
}

const char *find_symbol(uint64_t address) {
	int i;
/*
0xffffffff85200000<-> 0xffffffffc14c6094 -> __init_scratch_end
                      0xffffffffc14b8008 -> __init_scratch_end
*/
	for (i = 0; i < symbol_count; i++) {
//		printf("0x%" PRIx64 " <-> 0x%" PRIx64 " -> %s\n",symbols[i].start_address, symbols[i].end_address, symbols[i].symbol_name);
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

	parse_system_map(system_map_file);

	symbol = find_symbol(search_address);
	if (symbol) {
		printf("0x%" PRIx64 " -> %s\n", search_address, symbol);
	} else {
		printf("0x%" PRIx64 " not found\n", search_address);
	}

	return 0;
}
