#ifndef FEMU_FS_H
#define FEMU_FS_H

#include "qemu/osdep.h"

#define SLBA_FILENAME 0x1
#define SLBA_FD 0x2
#define SLBA_DATA_LEN 0x3
#define SLBA_DATA 0x4
#define SLBA_FS_START 0xf

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

// yesa:    max_entries = size of SSD / max_file_size
struct fs_inode_table {
    uint64_t max_file_size;
    uint64_t max_entries;
    uint64_t num_entries;
    struct fs_inode* inodes;
};

#endif
