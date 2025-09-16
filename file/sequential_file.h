#pragma once

#include <storage/storage.h>

typedef struct SequentialFile SequentialFile;

SequentialFile* sequential_file_alloc(Storage* storage);
void sequential_file_free(SequentialFile* file);
bool sequential_file_open(SequentialFile* file, const char* path);
void sequential_file_close(SequentialFile* file);
bool sequential_file_read(SequentialFile* file, uint8_t* data, size_t bytes);
uint64_t sequential_file_get_size(SequentialFile* file);