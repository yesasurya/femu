#include "fs.h"

#include "hw/block/block.h"
#include "hw/pci/msix.h"
#include "hw/pci/msi.h"
#include "../nvme.h"

struct fs_inode *fs_get_inode_of_fd(struct fs_inode_table *inode_table, int fd) {
//    for (int i = 1; i <= inode_table->num_entries; i++) {
//        struct fs_inode *inode = &inode_table->inodes[i];
//        if (inode->number == fd) {
//            return inode;
//        }
//    }
}

uint64_t fs_get_fd_of_file(struct fs_inode_table *inode_table, char *filename) {
//    for (int i = 1; i <= inode_table->max_entries; i++) {
//        struct fs_inode *inode = &inode_table->inodes[i];
//        if (inode->is_used && strcmp(filename, inode->filename) == 0) {
//            return inode->number;
//        }
//    }
//    return FS_FILENAME_NOT_FOUND;
}

uint64_t fs_get_unused_inode_index(struct fs_inode_table *inode_table) {
//    for (int i = 1; i <= inode_table->max_entries; i++) {
//        if (!inode_table->inodes[i].is_used) {
//            return i;
//        }
//    }
//    return FS_NO_INODE_AVAILABLE;
}

uint64_t fs_open_file(struct fs_inode_table *inode_table, char *filename) {
//    uint64_t fd = fs_get_fd_of_file(inode_table, filename);
//    if (fd != FS_FILENAME_NOT_FOUND)
//        return fd;
//
//    uint64_t unused_inode_index = fs_get_unused_inode_index(inode_table);
//    if (unused_inode_index == FS_NO_INODE_AVAILABLE) {
//        printf("YESA LOG: No inode available\n");
//        abort();
//    }
//    struct fs_inode *inode = &inode_table->inodes[unused_inode_index];
//    inode->filename = filename;
//    inode->is_used = true;
//    inode_table->num_entries++;
//    return inode->number;
}

void fs_close_file(struct fs_inode_table *inode_table, uint64_t fd) {
//    struct fs_inode *inode = fs_get_inode_of_fd(inode_table, fd);
//    if (!inode->is_used) {
//        printf("YESA LOG: fd has not been used\n");
//        abort();
//    }
//    inode->is_used = false;
}

void fs_init_metadata(FemuCtrl *n) {
    if (n->max_file_total == 0) {
        if (n->max_file_size == 0) {
            n->max_file_total = 4;
            n->max_file_size = n->mbe.size / n->max_file_total;
        } else {
            n->max_file_total = n->mbe.size / n->max_file_size;
        }
    } else {
        n->max_file_size = n->mbe.size / n->max_file_total;
    }
    n->metadata.max_file_total = n->max_file_total;
    n->metadata.max_file_size = n->max_file_size;
    n->metadata.max_directory_total = 100 * n->metadata.max_file_total;
}

void fs_init_utils(FemuCtrl *n) {
    n->utils.buffer_prp1 = malloc(sizeof(char *) * (n->num_poller + 1));
    for (int i = 0; i <= n->num_poller; i++) {
        n->utils.buffer_prp1[i] = malloc(n->page_size);
    }

    n->utils.buffer_tokens = malloc(sizeof(char *) * (n->metadata.max_directory_total + 1));
}

void fs_init_inode_file(FemuCtrl *n, uint64_t number) {
    struct fs_inode *inode = &n->inode_table.inodes[number];
    inode->type = FS_INODE_FILE;
    inode->filename = malloc(n->page_size);
    inode->number = number;
    inode->address = (number - 1) * n->metadata.max_file_size;
    inode->length = 0;
    inode->is_used = false;
}

void fs_init_inode_directory(FemuCtrl *n, uint64_t number) {
    struct fs_inode *inode = &n->inode_table.inodes[number];
    inode->type = FS_INODE_DIRECTORY;
    inode->filename = malloc(n->page_size);
    inode->number = number;
    inode->num_children_inodes = 0;
    inode->children_inodes = malloc(sizeof(struct fs_inode*) * (n->metadata.max_directory_total + 1));
    inode->is_used = false;
}

int64_t fs_get_unused_inode_file(FemuCtrl *n) {
    for (int i = 1; i <= n->metadata.max_file_total; i++) {
        struct fs_inode *inode = &n->inode_table.inodes[i];
        if (!inode->is_used) {
            return inode->number;
        }
    }
    return FS_NO_INODE_AVAILABLE;
}

int64_t fs_get_inode_file_by_name(FemuCtrl *n, char *filename) {
    for (int i = 1; i <= n->metadata.max_file_total; i++) {
        struct fs_inode *inode = &n->inode_table.inodes[i];
        if (strcmp(filename, inode->filename) == 0) {
            return inode->number;
        }
    }
    return FS_NO_INODE_FOUND;
}

int64_t fs_get_unused_inode_directory(FemuCtrl *n) {
    for (int i = n->metadata.max_file_total + 1; i <= n->metadata.max_file_total + n->metadata.max_directory_total; i++) {
        struct fs_inode *inode = &n->inode_table.inodes[i];
        if (!inode->is_used) {
            return inode->number;
        }
    }
    return FS_NO_INODE_AVAILABLE;
}

int64_t fs_get_inode_directory_by_name(FemuCtrl *n, char *filename, struct fs_inode *parent_inode) {
    if (parent_inode) {
        for (int i = n->metadata.max_file_total + 1; i <= n->metadata.max_file_total + n->metadata.max_directory_total; i++) {
            struct fs_inode *inode = &n->inode_table.inodes[i];
            if (strcmp(filename, inode->filename) == 0 && inode->parent_inode == parent_inode) {
                return inode->number;
            }
        }
    }

    for (int i = n->metadata.max_file_total + 1; i <= n->metadata.max_file_total + n->metadata.max_directory_total; i++) {
        struct fs_inode *inode = &n->inode_table.inodes[i];
        if (strcmp(filename, inode->filename) == 0) {
            return inode->number;
        }
    }

    return FS_NO_INODE_FOUND;
}

void fs_create_file(FemuCtrl *n, char *filename) {
    uint64_t inode_number = fs_get_inode_file_by_name(n, filename);
    if (inode_number != FS_NO_INODE_FOUND) {
        printf("YESA LOG: Failed. File already exists.\n");
        return;
    }

    inode_number = fs_get_unused_inode_file(n);
    if (inode_number == FS_NO_INODE_AVAILABLE) {
        printf("YESA LOG: Failed. All inodes have been used.\n");
        return;
    }
    printf("YESA LOG: Success. Creating inode with name = %s\n", filename);
    struct fs_inode *inode = &n->inode_table.inodes[inode_number];
    memcpy(inode->filename, filename, n->page_size);
    inode->is_used = true;
    n->inode_table.num_used_inode_file++;
}

void fs_delete_file(FemuCtrl *n, char *filename) {
    uint64_t inode_number = fs_get_inode_file_by_name(n, filename);
    if (inode_number == FS_NO_INODE_FOUND) {
        printf("YESA LOG: Failed. File does not exists.\n");
        return;
    }
    printf("YESA LOG: Success. Deleting inode with name = %s\n", filename);
    struct fs_inode *inode = &n->inode_table.inodes[inode_number];
    inode->is_used = false;
    n->inode_table.num_used_inode_file--;
}

struct fs_inode* _fs_create_directory(FemuCtrl *n, char *filename, struct fs_inode *parent_inode) {
    uint64_t inode_number = fs_get_inode_directory_by_name(n, filename, parent_inode);
    if (inode_number != FS_NO_INODE_FOUND) {
        printf("YESA LOG: Failed. Directory already exists.\n");
        return;
    }

    inode_number = fs_get_unused_inode_directory(n);
    if (inode_number == FS_NO_INODE_AVAILABLE) {
        printf("YESA LOG: Failed. All inodes have been used.\n");
        return;
    }
    printf("YESA LOG: Success. Creating inode with name = %s\n", filename);
    struct fs_inode *inode = &n->inode_table.inodes[inode_number];
    memcpy(inode->filename, filename, n->page_size);
    inode->parent_inode = parent_inode;
    inode->is_used = true;

    if (parent_inode) {
        parent_inode->num_children_inodes++;
        parent_inode->children_inodes[parent_inode->num_children_inodes] = inode;
    }

    n->inode_table.num_used_inode_directory++;

    return inode;
}

void fs_create_directory(FemuCtrl *n, char *filename) {
    char delimiter[2] = "/";
    int depth = 0;
    n->utils.buffer_tokens[depth] = strtok(filename, delimiter);
    while (n->utils.buffer_tokens[depth]) {
        depth++;
        n->utils.buffer_tokens[depth] = strtok(NULL, delimiter);
    }

    int new_directory_required = 0;
    for (int i = 0; i < depth; i++) {
        uint64_t inode_number = fs_get_inode_directory_by_name(n, n->utils.buffer_tokens[depth], NULL);
        if (inode_number == FS_NO_INODE_FOUND) {
            new_directory_required++;
        }
    }
    if (new_directory_required > (n->metadata.max_directory_total - n->inode_table.num_used_inode_directory)) {
        printf("YESA LOG: Failed. Free inodes are not enough.\n");
        return;
    }

    struct fs_inode *parent_inode = NULL;
    for (int i = 0; i < depth; i++) {
        uint64_t inode_number = fs_get_inode_directory_by_name(n->utils.buffer_tokens[depth], parent_inode);
        if (inode_number == FS_NO_INODE_FOUND) {
            parent_inode = _fs_create_directory(n, n->utils.buffer_tokens[depth], parent_inode);
        }
    }
}

void fs_delete_directory(FemuCtrl *n, char *filename) {
    uint64_t inode_number = fs_get_inode_directory_by_name(n, filename, NULL);
    if (inode_number == FS_NO_INODE_FOUND) {
        printf("YESA LOG: Failed. Directory does not exists.\n");
        return;
    }
    printf("YESA LOG: Success. Deleting inode with name = %s\n", filename);
    struct fs_inode *inode = &n->inode_table.inodes[inode_number];
    inode->is_used = false;
    n->inode_table.num_used_inode_directory--;
}

void fs_init_inode_table(FemuCtrl *n) {
    n->inode_table.num_used_inode_file = 0;
    n->inode_table.num_used_inode_directory = 0;
    n->inode_table.inodes = malloc(sizeof(struct fs_inode) * (n->metadata.max_file_total + n->metadata.max_directory_total + 1));
    if (n->inode_table.inodes == NULL) {
        printf("YESA LOG: Inode table allocation failed\n");
        abort();
    } else {
        printf("YESA LOG: Inode table allocation success.\n");
        printf("YESA LOG: Max file total in FS = %" PRIu64 "\n", n->metadata.max_file_total);
        printf("YESA LOG: Max file size in FS = %" PRIu64 "\n", n->metadata.max_file_size);
        printf("YESA LOG: Max directory total in FS = %" PRIu64 "\n", n->metadata.max_directory_total);
    }

    for (int i = 1; i <= n->metadata.max_file_total; i++) {
        fs_init_inode_file(n, i);
    }

    for (int i = n->metadata.max_file_total; i <= n->metadata.max_file_total + n->metadata.max_directory_total; i++) {
        fs_init_inode_directory(n, i);
    }
}

void fs_read_metadata(FemuCtrl *n) {
    return NVME_SUCCESS;
}

void fs_init(FemuCtrl *n) {
    fs_init_metadata(n);
    fs_init_inode_table(n);
    fs_init_utils(n);
}

uint64_t nvme_fs_open(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd, int index_poller) {
//    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
//
//    uint32_t nlb  = le16_to_cpu(fs_cmd->nlb) + 1;
//    uint64_t slba = le64_to_cpu(fs_cmd->slba);
//    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
//    const uint8_t lba_index = NVME_ID_NS_FLBAS_INDEX(ns->id_ns.flbas);
//    const uint8_t data_shift = ns->id_ns.lbaf[lba_index].ds;
//    uint64_t data_size = (uint64_t)nlb << data_shift;
//    uint64_t data_offset = slba << data_shift;
//
//    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->inode_table->filename_buf[index_poller], n->page_size, false);
//    fs_open_file(n->inode_table, n->inode_table->filename_buf);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_close(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
//    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    return NVME_SUCCESS;
}

uint64_t nvme_fs_read(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd, int index_poller, int sq_id) {
//    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
//
//    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
//
//    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->inode_table->test_buffer[index_poller], n->page_size, true);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_write(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd, int index_poller, int sq_id) {
//    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
//
//    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
//
//    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->inode_table->test_buffer[index_poller], n->page_size, false);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_lseek(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
//    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    return NVME_SUCCESS;
}

uint64_t nvme_fs_create_file(FemuCtrl *n, NvmeCmd *cmd, uint64_t index_poller) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->utils.buffer_prp1[index_poller], n->page_size, false);
    fs_create_file(n, n->utils.buffer_prp1[index_poller]);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_delete_file(FemuCtrl *n, NvmeCmd *cmd, uint64_t index_poller) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->utils.buffer_prp1[index_poller], n->page_size, false);
    fs_delete_file(n, n->utils.buffer_prp1[index_poller]);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_create_directory(FemuCtrl *n, NvmeCmd *cmd, uint64_t index_poller) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->utils.buffer_prp1[index_poller], n->page_size, false);
    fs_create_directory(n, n->utils.buffer_prp1[index_poller]);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_delete_directory(FemuCtrl *n, NvmeCmd *cmd, uint64_t index_poller) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->utils.buffer_prp1[index_poller], n->page_size, false);
    fs_delete_directory(n, n->utils.buffer_prp1[index_poller]);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_visualize(FemuCtrl *n, NvmeCmd *cmd, uint64_t index_poller) {
    printf("YESA LOG: FS Visualization\n");
    printf("YESA LOG: INODE TABLE\n");
    printf("YESA LOG: Num used inode files = %" PRIu64 " / %" PRIu64 "\n", n->inode_table.num_used_inode_file, n->metadata.max_file_total);
    printf("YESA LOG: Num used inode directory = %" PRIu64 " / %" PRIu64 "\n", n->inode_table.num_used_inode_directory, n->metadata.max_directory_total);
    boolean is_checked[n->metadata.max_file_total + n->metadata.max_directory_total + 1];
    memset(is_checked, false, n->metadata.max_file_total + n->metadata.max_directory_total + 1);

    uint64_t total_all_inodes = n->metadata.max_file_total + n->metadata.max_directory_total;
    for (int i = 1; i <= total_all_inodes; i++) {
        struct fs_inode *inode = &n->inode_table.inodes[i];
        if (inode->is_used && !is_checked[i]) {
            print_inode(inode, 0, is_checked);
        }
    }

    return NVME_SUCCESS;
}

uint64_t print_inode(struct fs_inode *inode, int depth, boolean *is_checked) {
    for (int i = 0; i < depth; i++) {
        printf("    ");
    }
    printf("(%" PRIu64 ", %s)\n", inode->number, inode->filename);
    is_checked[inode->number] = true;
    for (int i = 1; i <= inode->num_children_inodes; i++) {
        struct fs_inode *child_inode = inode->children_inodes[i];
        if (!is_checked[child_inode->number]) {
            print_inode(child_inode, depth + 1, is_checked);
        }
    }
}
