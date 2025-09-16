#include "sequential_file.h"

struct SequentialFile {
    Storage* storage;
    File* file;
};

SequentialFile* sequential_file_alloc(Storage* storage) {
    SequentialFile* file = malloc(sizeof(SequentialFile));
    file->storage = storage;
    file->file = storage_file_alloc(storage);
    return file;
}

void sequential_file_free(SequentialFile* file) {
    if(file) {
        if(file->file) {
            storage_file_free(file->file);
        }
        free(file);
    }
}

bool sequential_file_open(SequentialFile* file, const char* path) {
    if(!file || !file->file) return false;
    return storage_file_open(file->file, path, FSAM_READ, FSOM_OPEN_EXISTING);
}

void sequential_file_close(SequentialFile* file) {
    if(file && file->file) {
        storage_file_close(file->file);
    }
}

bool sequential_file_read(SequentialFile* file, uint8_t* data, size_t bytes) {
    if(!file || !file->file) return false;
    return storage_file_read(file->file, data, bytes) == bytes;
}

uint64_t sequential_file_get_size(SequentialFile* file) {
    if(!file || !file->file) return 0;
    return storage_file_size(file->file);
}