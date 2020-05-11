#include "qemu/osdep.h"
#include "fs.h"

uint64_t fs_get_inode_total(struct fs_inode_table *inode_table, uint64_t mem_size_bytes) {
    return mem_size_bytes / inode_table->max_file_size;
}
