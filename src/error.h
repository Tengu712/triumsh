#pragma once

#define          ERROR(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__); exit(1);
#define INTERNAL_ERROR(fmt, ...) fprintf(stderr, fmt, __VA_ARGS__); exit(2);

#define    CHECK(b, fmt, ...) if (!(b)) {          ERROR(fmt, __VA_ARGS__) }
#define VALIDATE(b, fmt, ...) if (!(b)) { INTERNAL_ERROR(fmt, __VA_ARGS__) }
