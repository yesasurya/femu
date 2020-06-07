#ifndef FEMU_FS_H
#define FEMU_FS_H

#include "qemu/osdep.h"

#define FS_FILENAME_NOT_FOUND -1
#define FS_NO_INODE_AVAILABLE -1

enum fs_inode_type {
    FS_INODE_FILE = 0
};

struct fs_inode {
    enum fs_inode_type type;
    char *filename;
    uint64_t number;
    uint64_t address;
    size_t length;
    bool is_used;
};

struct fs_inode_table {
    //  yesa: Temporarily storing filename here. Improved later.
    char **filename_buf;

    //  yesa: Only for testing
    char **test_buffer;
    uint64_t *offset;

    uint64_t max_file_size;
    uint64_t max_entries;
    uint64_t num_entries;
    struct fs_inode* inodes;
};

#endif
