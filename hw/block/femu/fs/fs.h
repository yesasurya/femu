#ifndef FEMU_FS_H
#define FEMU_FS_H
#endif

struct fs_inode {
    uint32_t inode_number;
    uint64_t address;
};

struct fs_inode_table {
    struct fs_inode* fs_inodes;
};

uint64_t get_inode_total(uint64_t mem_size_bytes);