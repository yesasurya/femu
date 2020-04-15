#ifndef FEMU_FS_H
#define FEMU_FS_H

#include "qemu/osdep.h"

enum fs_inode_type {
    FS_INODE_FILE = 0,
    FS_INODE_DIR = 1,
};

struct fs_inode {
    enum fs_inode_type type;
    uint64_t number;
    uint64_t address;
    size_t length;
    struct fs_inode* inodes;
};

//  YESA: At the moment, just make fixed allocation, maximum size of file is 128MB
struct fs_inode_table {
    struct fs_inode* inodes;
};

#endif
