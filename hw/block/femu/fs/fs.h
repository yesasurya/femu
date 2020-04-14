#ifndef FEMU_FS_H
#define FEMU_FS_H

#include "qemu/osdep.h"

struct fs_inode {
    uint32_t inode_number;
    uint64_t address;
};

struct fs_inode_table {
    struct fs_inode* fs_inodes;
};

#endif
