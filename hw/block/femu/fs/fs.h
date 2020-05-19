#ifndef FEMU_FS_H
#define FEMU_FS_H

#include "qemu/osdep.h"

enum fs_inode_type {
    FS_INODE_FILE = 0
};

struct fs_inode {
    enum fs_inode_type type;
    char *filename;
    uint64_t number;
    uint64_t address;
    size_t length;
    struct fs_inode* inodes;
};

struct fs_inode_table {
    uint64_t max_file_size;
    uint64_t max_entries;
    uint64_t num_entries;
    struct fs_inode* inodes;
};

#endif
