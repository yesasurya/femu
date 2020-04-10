#include "fs.h"

uint64_t get_inode_total(uint64_t mem_size_bytes) {
    return mem_size_bytes / 4096;
}