#include "qemu/osdep.h"
#include "fs.h"

uint64_t fs_get_inode_total(struct fs_inode_table *inode_table, uint64_t mem_size_bytes) {
    return mem_size_bytes / inode_table->max_file_size;
}

uint64_t fs_get_fd_of_file(struct fs_inode_table *inode_table, char *filename) {
    for (int i = 1; i <= inode_table->num_entries; i++) {
        struct fs_inode inode = inode_table->inodes[i];
        if (strcmp(filename, inode.filename) == 0) {
            return inode.number;
        }
    }
    return -1;
}

uint64_t fs_open_file(struct fs_inode_table *inode_table, char *filename) {
    uint64_t fd = fs_get_fd_of_file(inode_table, filename);
    if (fd != -1)
        return fd;

    struct fs_inode inode = inode_table->inodes[inode_table->num_entries + 1];
    inode.type = FS_INODE_FILE;
    inode.filename = filename;
    inode.number = inode_table->num_entries + 1;
    inode.address = (SLBA_DATA * 0x200) + (inode.number - 1) * inode_table->max_file_size;
    inode.length = 0;

    inode_table->num_entries++;
    return inode.number;
}
