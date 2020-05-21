#include "fs.h"

#include "hw/block/block.h"
#include "hw/pci/msix.h"
#include "hw/pci/msi.h"
#include "../nvme.h"

struct fs_inode *fs_get_inode_of_fd(struct fs_inode_table *inode_table, int fd) {
    for (int i = 1; i <= inode_table->num_entries; i++) {
        struct fs_inode *inode = &inode_table->inodes[i];
        if (inode->number == fd) {
            return inode;
        }
    }
}

uint64_t fs_get_fd_of_file(struct fs_inode_table *inode_table, char *filename) {
    for (int i = 1; i <= inode_table->max_entries; i++) {
        struct fs_inode *inode = &inode_table->inodes[i];
        if (inode->is_used && strcmp(filename, inode->filename) == 0) {
            return inode->number;
        }
    }
    return FS_FILENAME_NOT_FOUND;
}

uint64_t fs_get_unused_inode_index(struct fs_inode_table *inode_table) {
    for (int i = 1; i <= inode_table->max_entries; i++) {
        if (!inode_table->inodes[i].is_used) {
            return i;
        }
    }
    return FS_NO_INODE_AVAILABLE;
}

void fs_init_inode(struct fs_inode_table *inode_table, uint64_t number) {
    inode_table->inodes[number].type = FS_INODE_FILE;
    inode_table->inodes[number].filename = "";
    inode_table->inodes[number].number = number;
    inode_table->inodes[number].address = (number - 1) * inode_table->max_file_size;
    inode_table->inodes[number].length = 0;
    inode_table->inodes[number].is_used = false;
}

void fs_init_inode_table(FemuCtrl *n) {
    n->inode_table = malloc(sizeof(struct fs_inode_table));
    n->inode_table->max_file_size = 131072; //128MB
    n->inode_table->max_entries = n->mbe.size / n->inode_table->max_file_size;
    n->inode_table->num_entries = 0;
    n->inode_table->inodes = malloc(n->inode_table->max_entries * sizeof(struct fs_inode));
    if (n->inode_table->inodes == NULL) {
        printf("YESA LOG: Inode table allocation failed\n");
        abort();
    } else {
        printf("YESA LOG: Inode table allocation success. Max entries = %" PRIu64 "\n", n->inode_table->max_entries);
    }

    for (int i = 1; i <= n->inode_table->max_entries; i++) {
        fs_init_inode(n->inode_table, i);
    }
}

uint64_t fs_open_file(struct fs_inode_table *inode_table, char *filename) {
    uint64_t fd = fs_get_fd_of_file(inode_table, filename);
    if (fd != FS_FILENAME_NOT_FOUND)
        return fd;

    printf("YESA LOG: Creating new file...\n");
    uint64_t unused_inode_index = fs_get_unused_inode_index(inode_table);
    if (unused_inode_index == FS_NO_INODE_AVAILABLE) {
        printf("YESA LOG: No inode available\n");
        abort();
    }
    struct fs_inode *inode = &inode_table->inodes[unused_inode_index];
    inode->filename = filename;
    inode->is_used = true;
    inode_table->num_entries++;
    return inode->number;
}

void fs_close_file(struct fs_inode_table *inode_table, uint64_t fd) {
    struct fs_inode *inode = fs_get_inode_of_fd(inode_table, fd);
    if (!inode->is_used) {
        printf("YESA LOG: fd has not been used\n");
        abort();
    }
    inode->is_used = false;
}

void fs_init(FemuCtrl *n) {
    fs_init_inode_table(n);
}

uint64_t nvme_fs_open(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    return NVME_SUCCESS;
}

uint64_t nvme_fs_close(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    return NVME_SUCCESS;
}

uint64_t nvme_fs_read(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    return NVME_SUCCESS;
}

uint64_t nvme_fs_write(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    return NVME_SUCCESS;
}
