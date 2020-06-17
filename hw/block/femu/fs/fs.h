#ifndef FEMU_FS_H
#define FEMU_FS_H

#include "qemu/osdep.h"

#define FS_NO_INODE_FOUND -1
#define FS_NO_INODE_AVAILABLE -1

enum fs_inode_type {
    FS_INODE_FILE = 0,
    FS_INODE_DIRECTORY = 1,
};

struct fs_inode {
    bool is_used;

    struct fs_inode *parent_inode;
    enum fs_inode_type type;
    char *filename;
    uint64_t number;
    uint64_t address;
    size_t length;

    uint64_t num_children_inodes;
    struct fs_inode **children_inodes;
};

struct fs_inode_table {
    uint64_t num_used_inode_directory;
    uint64_t num_used_inode_file;
    struct fs_inode* inodes;
};

struct fs_file_table_entry {
    bool        is_used;

    uint64_t    fd;
    uint64_t    inode_number;
};

struct fs_file_table {
    struct fs_file_table_entry *entries;
};

struct fs_metadata {
    uint64_t max_file_total;
    uint64_t max_file_size;
    uint64_t max_directory_total;
};

struct fs_utils {
    char **buffer_prp1;
    char **buffer_tokens;
};

#endif
