#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

// Returns file size in bytes, or LONG_MAX on failure.
long get_file_size(FILE *fp) {
	if (fseek(fp, 0, SEEK_END)) return LONG_MAX;
	const long file_size = ftell(fp);
	if (fseek(fp, 0, SEEK_SET)) return LONG_MAX;
	return file_size;
}

// Allocates and reads file contents with null termination.
// Returns NULL on failure.
uint8_t *read_file(FILE *fp, long file_size) {
	uint8_t *const data = (uint8_t *)malloc(file_size + 1);
	if (!data) return NULL;
	size_t readed_size = fread((void *)data, 1, file_size, fp);
	if (readed_size < file_size) return NULL;
	data[readed_size] = '\0';
	return data;
}

int main(int argc, const char *const argv[]) {
	if (argc < 2) {
		printf("Usage: trish <file-path>\n");
		return 0;
	}

	FILE *const fp = fopen(argv[1], "rb");
	if (!fp) {
		fprintf(stderr, "No such file: %s\n", argv[1]);
		return 1;
	}

	const long file_size = get_file_size(fp);
	if (file_size == LONG_MAX) {
		fprintf(stderr, "Failed to seek in '%s'\n", argv[1]);
		fclose(fp);
		return 1;
	}

	uint8_t *const data = read_file(fp, file_size);
	if (!data) {
		fprintf(stderr, "Failed to read '%s'\n", argv[1]);
		fclose(fp);
		return 1;
	}

	fclose(fp);

	printf("%s", (char *)data);

	free((void *)data);
	return 0;
}
