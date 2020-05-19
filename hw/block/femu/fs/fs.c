#include "qemu/osdep.h"
#include "fs.h"
#include "../nvme.h"

uint64_t fs_get_inode_total(struct fs_inode_table *inode_table, uint64_t mem_size_bytes) {
    return mem_size_bytes / inode_table->max_file_size;
}

struct fs_inode fs_get_inode_of_fd(struct fs_inode_table *inode_table, int fd) {
    for (int i = 1; i <= inode_table->num_entries; i++) {
        struct fs_inode inode = inode_table->inodes[i];
        if (inode.number == fd) {
            return inode;
        }
    }
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

    struct fs_inode *inode = &inode_table->inodes[inode_table->num_entries + 1];
    inode->type = FS_INODE_FILE;
    inode->filename = filename;
    inode->number = inode_table->num_entries + 1;
    inode->address = (inode->number - 1) * inode_table->max_file_size;
    inode->length = 0;

    inode_table->num_entries++;
    return inode->number;
}

void fs_init(FemuCtrl *n) {
    n->inode_table = malloc(sizeof(struct fs_inode_table));
    n->inode_table->max_file_size = 131072; //128MB
    uint64_t inode_total = fs_get_inode_total(n->inode_table, n->mbe.size);
    n->inode_table->max_entries = inode_total;
    n->inode_table->num_entries = 0;
    n->inode_table->inodes = malloc((inode_total + 1) * sizeof(struct fs_inode));
    if (n->inode_table->inodes == NULL) {
        printf("YESA LOG: Inode table allocation failed\n");
        abort();
    } else {
        printf("YESA LOG: Inode table allocation success. n->mbe.size = %" PRIu64 ", inode_total = %" PRIu64 "\n", n->mbe.size, inode_total);
    }
}
