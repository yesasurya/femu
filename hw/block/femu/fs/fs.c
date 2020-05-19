#include "fs.h"

#include "hw/block/block.h"
#include "hw/pci/msix.h"
#include "hw/pci/msi.h"
#include "../nvme.h"

struct fs_inode fs_get_inode_of_fd(struct fs_inode_table *inode_table, int fd) {
    for (int i = 1; i <= inode_table->num_entries; i++) {
        struct fs_inode inode = inode_table->inodes[i];
        if (inode.number == fd) {
            return inode;
        }
    }
}

uint64_t fs_get_fd_of_file(struct fs_inode_table *inode_table, char *filename) {
    printf("YESA LOG: inode_table->max_entries = %" PRIu64 "\n", inode_table->max_entries);
    for (int i = 1; i <= inode_table->max_entries; i++) {
        printf("Iteration %d\n", i);
        struct fs_inode inode = inode_table->inodes[i];
        if (inode.is_used) {
            printf("YESA LOG: inode is used\n");
        } else {
            printf("YESA LOG: inode is not used\n");
        }
        printf("filename = %s\n", filename);
        printf("inode.filename = %s\n", inode.filename);
        if (inode.is_used && strcmp(filename, inode.filename) == 0) {
            return inode.number;
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
    struct fs_inode inode = inode_table->inodes[unused_inode_index];
    inode.filename = filename;
    inode.is_used = true;
    inode_table->num_entries++;
    return inode.number;
}

void fs_close_file(struct fs_inode_table *inode_table, uint64_t fd) {
    struct fs_inode inode = fs_get_inode_of_fd(inode_table, fd);
    if (!inode.is_used) {
        printf("YESA LOG: fd has not been used\n");
        abort();
    }
    inode.is_used = false;
}

void fs_init(FemuCtrl *n) {
    fs_init_inode_table(n);
}

uint64_t nvme_fs_open(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
    printf("YESA LOG: nvme_fs_open\n");

    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    uint32_t nlb  = le16_to_cpu(fs_cmd->nlb) + 1;
    uint64_t slba = le64_to_cpu(fs_cmd->slba);
    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    const uint8_t lba_index = NVME_ID_NS_FLBAS_INDEX(ns->id_ns.flbas);
    const uint8_t data_shift = ns->id_ns.lbaf[lba_index].ds;
    uint64_t data_size = (uint64_t)nlb << data_shift;
    uint64_t data_offset = slba << data_shift;

    hwaddr len = n->page_size;
    /* Processing prp1 */
    char *filename = malloc(len);
    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, filename, len, false);
    printf("YESA LOG: opening filename = %s\n", filename);

    uint64_t fd = fs_open_file(n->inode_table, filename);
    printf("YESA LOG: file fd = %" PRIu64 "\n", fd);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_close(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
    printf("YESA LOG: nvme_fs_close\n");

    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    printf("YESA LOG: closing file fd = %" PRIu32 "\n", fs_cmd->fd);

    fs_close_file(n->inode_table, fs_cmd->fd);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_read(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
    printf("YESA LOG: nvme_fs_read\n");

    return NVME_SUCCESS;
}

uint64_t nvme_fs_write(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
    printf("YESA LOG: nvme_fs_write\n");

    return NVME_SUCCESS;
}
