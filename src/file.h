#pragma once

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

typedef struct FileInfo_t {
	const char *const    name;
	const uint8_t *const data;
	size_t const         size;
} FileInfo;

// Returns file size in bytes, or LONG_MAX on failure.
static inline size_t get_file_size(FILE *fp) {
	if (fseek(fp, 0, SEEK_END)) return LONG_MAX;
	const long file_size = ftell(fp);
	if (fseek(fp, 0, SEEK_SET)) return LONG_MAX;
	return (size_t)file_size;
}

// Allocates and reads file contents with null termination.
// Returns NULL on failure.
static inline uint8_t *read_file(FILE *fp, size_t file_size) {
	uint8_t *const data = (uint8_t *)malloc(file_size + 1);
	if (!data) return NULL;
	const size_t readed_size = fread((void *)data, 1, file_size, fp);
	if (readed_size < file_size) return NULL;
	data[readed_size] = '\0';
	return data;
}
