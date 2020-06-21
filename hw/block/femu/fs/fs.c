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
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
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
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    n->utils.buffer_prp1 = malloc(sizeof(char *) * (n->num_poller + 1));
    for (int i = 0; i <= n->num_poller; i++) {
        n->utils.buffer_prp1[i] = malloc(n->page_size);
    }

    n->utils.buffer_tokens = malloc(sizeof(char *) * (n->metadata.max_directory_total + 1));
}

void fs_init_inode_file(FemuCtrl *n, uint64_t number) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    struct fs_inode *inode = &n->inode_table.inodes[number];
    inode->is_used = false;
    inode->type = FS_INODE_FILE;
    inode->filename = malloc(n->page_size);
    inode->number = number;
    inode->address = (number - 1) * n->metadata.max_file_size;
    inode->length = 0;
    inode->parent_inode = NULL;
    inode->lowest_index_avail_child_inode = 0;
    inode->max_num_children_inodes = 0;
    inode->num_children_inodes = 0;
    inode->children_inodes = NULL;
}

void fs_init_inode_directory(FemuCtrl *n, uint64_t number) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    struct fs_inode *inode = &n->inode_table.inodes[number];
    inode->is_used = false;
    inode->type = FS_INODE_DIRECTORY;
    inode->filename = malloc(n->page_size);
    inode->number = number;
    inode->address = 0;
    inode->length = 0;
    inode->parent_inode = NULL;
    inode->lowest_index_avail_child_inode = 1;
    inode->max_num_children_inodes = n->metadata.max_directory_total;
    inode->num_children_inodes = 0;
    inode->children_inodes = malloc(sizeof(struct fs_inode*) * (inode->max_num_children_inodes + 1));
    for (int i = 0; i <= inode->max_num_children_inodes; i++) {
        inode->children_inodes[i] = NULL;
    }
}

int64_t fs_get_unused_inode_file(FemuCtrl *n) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    for (int i = 1; i <= n->metadata.max_file_total; i++) {
        struct fs_inode *inode = &n->inode_table.inodes[i];
        if (!inode->is_used) {
            return inode->number;
        }
    }
    return FS_NO_INODE_AVAILABLE;
}

int64_t fs_get_inode_file_by_name(FemuCtrl *n, char *filename, struct fs_inode *parent_inode) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    for (int i = 1; i <= n->metadata.max_file_total; i++) {
        struct fs_inode *inode = &n->inode_table.inodes[i];
        if (strcmp(filename, inode->filename) == 0 && inode->parent_inode == parent_inode && inode->is_used) {
            return inode->number;
        }
    }

    return FS_NO_INODE_FOUND;
}

int64_t fs_get_unused_inode_directory(FemuCtrl *n) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    for (int i = n->metadata.max_file_total + 1; i <= n->metadata.max_file_total + n->metadata.max_directory_total; i++) {
        struct fs_inode *inode = &n->inode_table.inodes[i];
        if (!inode->is_used) {
            return inode->number;
        }
    }
    return FS_NO_INODE_AVAILABLE;
}

int64_t fs_get_inode_directory_by_name(FemuCtrl *n, char *filename, struct fs_inode *parent_inode) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    for (int i = n->metadata.max_file_total + 1; i <= n->metadata.max_file_total + n->metadata.max_directory_total; i++) {
        struct fs_inode *inode = &n->inode_table.inodes[i];
        if (strcmp(filename, inode->filename) == 0 && inode->parent_inode == parent_inode && inode->is_used) {
            return inode->number;
        }
    }

    return FS_NO_INODE_FOUND;
}

int fs_parse_filename(FemuCtrl *n, char *filename) {
    char delimiter[2] = "/";
    int depth = 0;
    n->utils.buffer_tokens[depth] = strtok(filename, delimiter);
    while (n->utils.buffer_tokens[depth]) {
        depth++;
        n->utils.buffer_tokens[depth] = strtok(NULL, delimiter);
    }

    return depth;
}

struct fs_inode* _fs_create_directory(FemuCtrl *n, char *filename, struct fs_inode *parent_inode) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    uint64_t inode_number = fs_get_inode_directory_by_name(n, filename, parent_inode);
    if (inode_number != FS_NO_INODE_FOUND) {
        printf("YESA LOG: Failed. Directory already exists.\n");
        return;
    }

    inode_number = fs_get_unused_inode_directory(n);
    if (inode_number == FS_NO_INODE_AVAILABLE) {
        printf("YESA LOG: Failed. Free inodes are not enough.\n");
        return;
    }

    printf("YESA LOG: Success. Creating inode with name = %s\n", filename);
    struct fs_inode *inode = &n->inode_table.inodes[inode_number];
    assert(inode->type == FS_INODE_DIRECTORY);
    memcpy(inode->filename, filename, n->page_size);
    inode->is_used = true;
    inode->parent_inode = parent_inode;
    inode->num_children_inodes = 0;
    inode->lowest_index_avail_child_inode = 1;

    if (parent_inode) {
        parent_inode->children_inodes[parent_inode->lowest_index_avail_child_inode] = inode;
        while (parent_inode->children_inodes[parent_inode->lowest_index_avail_child_inode]) {
            parent_inode->lowest_index_avail_child_inode++;
            if (parent_inode->lowest_index_avail_child_inode > parent_inode->max_num_children_inodes) {
                break;
            }
        }
        parent_inode->num_children_inodes++;
    }
    n->inode_table.num_used_inode_directory++;

    return inode;
}

struct fs_inode* fs_create_directory(FemuCtrl *n, char *filename) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    int depth = fs_parse_filename(n, filename);

    int new_directory_required = 0;
    for (int i = 0; i < depth; i++) {
        uint64_t inode_number = fs_get_inode_directory_by_name(n, n->utils.buffer_tokens[i], NULL);
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
        uint64_t inode_number = fs_get_inode_directory_by_name(n, n->utils.buffer_tokens[i], parent_inode);
        if (inode_number == FS_NO_INODE_FOUND) {
            parent_inode = _fs_create_directory(n, n->utils.buffer_tokens[i], parent_inode);
        } else {
            parent_inode = &n->inode_table.inodes[inode_number];
        }
    }

    return parent_inode;
}

struct fs_inode* fs_create_directory_from_file_creation(FemuCtrl *n, int depth) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);

    int depth_to_last_directory = depth - 1;
    int new_directory_required = 0;
    for (int i = 0; i < depth_to_last_directory; i++) {
        uint64_t inode_number = fs_get_inode_directory_by_name(n, n->utils.buffer_tokens[i], NULL);
        if (inode_number == FS_NO_INODE_FOUND) {
            new_directory_required++;
        }
    }
    if (new_directory_required > (n->metadata.max_directory_total - n->inode_table.num_used_inode_directory)) {
        printf("YESA LOG: Failed. Free inodes are not enough.\n");
        return;
    }

    struct fs_inode *parent_inode = NULL;
    printf("YESA LOG: depth_to_last_directory = %d\n", depth_to_last_directory);
    for (int i = 0; i < depth_to_last_directory; i++) {
        uint64_t inode_number = fs_get_inode_directory_by_name(n, n->utils.buffer_tokens[i], parent_inode);
        if (inode_number == FS_NO_INODE_FOUND) {
            parent_inode = _fs_create_directory(n, n->utils.buffer_tokens[i], parent_inode);
        } else {
            parent_inode = &n->inode_table.inodes[inode_number];
        }
    }

    return parent_inode;
}

void _fs_delete_directory(FemuCtrl *n, struct fs_inode *inode) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    if (inode->num_children_inodes == 0) {
        inode->is_used = false;
        n->inode_table.num_used_inode_directory--;

        struct fs_inode *parent_inode = inode->parent_inode;
        if (!parent_inode) {
            return;
        }

        parent_inode->num_children_inodes--;

        parent_inode->lowest_index_avail_child_inode = 1;
        while (parent_inode->children_inodes[parent_inode->lowest_index_avail_child_inode]) {
            parent_inode->lowest_index_avail_child_inode++;
        }

        for (int i = 1; i <= parent_inode->max_num_children_inodes; i++) {
            if (parent_inode->children_inodes[i] == inode) {
                parent_inode->children_inodes[i] = NULL;
                break;
            }
        }
        inode->parent_inode = NULL;

        return;
    }

    for (int i = 1; i <= inode->max_num_children_inodes; i++) {
        struct fs_inode *child_inode = inode->children_inodes[i];
        if (child_inode) {
            _fs_delete_directory(n, child_inode);
        }
    }
    _fs_delete_directory(n, inode);
}

void fs_delete_directory(FemuCtrl *n, char *filename) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    int depth = fs_parse_filename(n, filename);

    struct fs_inode *parent_inode = NULL;
    for (int i = 0; i < depth; i++) {
        uint64_t inode_number = fs_get_inode_directory_by_name(n, n->utils.buffer_tokens[i], parent_inode);
        if (inode_number == FS_NO_INODE_FOUND) {
            printf("YESA LOG: Failed. Directory does not exists.\n");
            return;
        }
        parent_inode = &n->inode_table.inodes[inode_number];
    }
    _fs_delete_directory(n, parent_inode);
}

struct fs_inode* _fs_create_file(FemuCtrl *n, char *filename, struct fs_inode *parent_inode) {
    uint64_t inode_number = fs_get_inode_file_by_name(n, filename, NULL);
    if (inode_number != FS_NO_INODE_FOUND) {
        printf("YESA LOG: Failed. File already exists.\n");
        return;
    }

    inode_number = fs_get_unused_inode_file(n);
    if (inode_number == FS_NO_INODE_AVAILABLE) {
        printf("YESA LOG: Failed. Free inodes are not enough.\n");
        return;
    }

    printf("YESA LOG: Success. Creating inode with name = %s\n", filename);
    struct fs_inode *inode = &n->inode_table.inodes[inode_number];
    assert(inode->type == FS_INODE_FILE);
    memcpy(inode->filename, filename, n->page_size);
    inode->is_used = true;
    inode->parent_inode = parent_inode;

    if (parent_inode) {
        parent_inode->children_inodes[parent_inode->lowest_index_avail_child_inode] = inode;
        while (parent_inode->children_inodes[parent_inode->lowest_index_avail_child_inode]) {
            parent_inode->lowest_index_avail_child_inode++;
            if (parent_inode->lowest_index_avail_child_inode > parent_inode->max_num_children_inodes) {
                break;
            }
        }
        parent_inode->num_children_inodes++;
    }
    n->inode_table.num_used_inode_file++;
}

void fs_create_file(FemuCtrl *n, char *filename) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    uint64_t inode_number = fs_get_unused_inode_file(n);
    if (inode_number == FS_NO_INODE_AVAILABLE) {
        printf("YESA LOG: Failed. All inodes have been used.\n");
        return;
    }

    int depth = fs_parse_filename(n, filename);
    struct fs_inode *parent_inode = NULL;
    if (depth > 1) {
        parent_inode = fs_create_directory_from_file_creation(n, depth);
    }

    _fs_create_file(n, n->utils.buffer_tokens[depth -1], parent_inode);
}

void fs_delete_file(FemuCtrl *n, char *filename) {
//    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
//    uint64_t inode_number = fs_get_inode_file_by_name(n, filename);
//    if (inode_number == FS_NO_INODE_FOUND) {
//        printf("YESA LOG: Failed. File does not exists.\n");
//        return;
//    }
//    printf("YESA LOG: Success. Deleting inode with name = %s\n", filename);
//    struct fs_inode *inode = &n->inode_table.inodes[inode_number];
//    inode->is_used = false;
//    n->inode_table.num_used_inode_file--;
}

void fs_init_inode_table(FemuCtrl *n) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
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

    for (int i = n->metadata.max_file_total + 1; i <= n->metadata.max_file_total + n->metadata.max_directory_total; i++) {
        fs_init_inode_directory(n, i);
    }
}

void fs_init(FemuCtrl *n) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    fs_init_metadata(n);
    fs_init_inode_table(n);
    fs_init_utils(n);
}

uint64_t nvme_fs_open(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd, int index_poller) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
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
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
//    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    return NVME_SUCCESS;
}

uint64_t nvme_fs_read(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd, int index_poller, int sq_id) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;

    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    uint64_t offset = fs_cmd->fd;

    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->mbe.mem_backend + offset, n->page_size, true);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_write(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd, int index_poller, int sq_id) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;

    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    uint64_t offset = fs_cmd->fd;

    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->mbe.mem_backend + offset, n->page_size, false);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_lseek(FemuCtrl *n, NvmeNamespace *ns, NvmeCmd *cmd) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
//    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    return NVME_SUCCESS;
}

uint64_t nvme_fs_create_file(FemuCtrl *n, NvmeCmd *cmd, uint64_t index_poller) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->utils.buffer_prp1[index_poller], n->page_size, false);
    fs_create_file(n, n->utils.buffer_prp1[index_poller]);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_delete_file(FemuCtrl *n, NvmeCmd *cmd, uint64_t index_poller) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->utils.buffer_prp1[index_poller], n->page_size, false);
    fs_delete_file(n, n->utils.buffer_prp1[index_poller]);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_create_directory(FemuCtrl *n, NvmeCmd *cmd, uint64_t index_poller) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->utils.buffer_prp1[index_poller], n->page_size, false);
    fs_create_directory(n, n->utils.buffer_prp1[index_poller]);

    return NVME_SUCCESS;
}

uint64_t nvme_fs_delete_directory(FemuCtrl *n, NvmeCmd *cmd, uint64_t index_poller) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;
    uint64_t prp1 = le64_to_cpu(fs_cmd->prp1);
    address_space_rw(&address_space_memory, prp1, MEMTXATTRS_UNSPECIFIED, n->utils.buffer_prp1[index_poller], n->page_size, false);
    fs_delete_directory(n, n->utils.buffer_prp1[index_poller]);

    return NVME_SUCCESS;
}

void print_inode(struct fs_inode *inode, int depth, bool *is_checked, char *serialized) {
    printf("YESA LOG: %s, %s\n", __FILE__, __func__);
//    for (int i = 0; i < depth; i++) {
//        printf("    ");
//    }

    char *serialized_entry = malloc(4096);
    if (inode->parent_inode) {
        sprintf(serialized_entry, "%" PRIu64 ",%s>%" PRIu64 ",%s*", inode->number, inode->filename, inode->parent_inode->number, inode->parent_inode->filename);
//        printf("(%" PRIu64 ", %s, child of (%" PRIu64 ", %s))\n", inode->number, inode->filename, inode->parent_inode->number, inode->parent_inode->filename);
    } else {
        sprintf(serialized_entry, "%" PRIu64 ",%s->0,ROOT*", inode->number, inode->filename);
//        printf("(%" PRIu64 ", %s, child of UNKNOWN)\n", inode->number, inode->filename);
    }
    strcat(serialized, serialized_entry);

    is_checked[inode->number] = true;
    if (inode->type == FS_INODE_FILE) {
        return;
    }

    for (int i = 1; i <= inode->max_num_children_inodes; i++) {
        struct fs_inode *child_inode = inode->children_inodes[i];
        if (child_inode && !is_checked[child_inode->number]) {
            print_inode(child_inode, depth + 1, is_checked, serialized);
        }
    }
}

uint64_t nvme_fs_visualize(FemuCtrl *n, NvmeCmd *cmd, uint64_t index_poller) {
    NvmeFsCmd *fs_cmd = (NvmeFsCmd *)cmd;

    printf("YESA LOG: FS Visualization\n");
    printf("YESA LOG: Num used inode files = %" PRIu64 " / %" PRIu64 "\n", n->inode_table.num_used_inode_file, n->metadata.max_file_total);
    printf("YESA LOG: Num used inode directory = %" PRIu64 " / %" PRIu64 "\n", n->inode_table.num_used_inode_directory, n->metadata.max_directory_total);
    bool is_checked[n->metadata.max_file_total + n->metadata.max_directory_total + 1];
    memset(is_checked, false, n->metadata.max_file_total + n->metadata.max_directory_total + 1);

    char *serialized = malloc(100 * n->page_size);

    uint64_t total_all_inodes = n->metadata.max_file_total + n->metadata.max_directory_total;
    for (int i = 1; i <= total_all_inodes; i++) {
        struct fs_inode *inode = &n->inode_table.inodes[i];
        if (inode->is_used && !is_checked[i]) {
            print_inode(inode, 0, is_checked, serialized);
        }
    }
    printf("YESA LOG: serialized = %s\n", serialized);
    address_space_rw(&address_space_memory, fs_cmd->prp1, MEMTXATTRS_UNSPECIFIED, serialized, 100 * n->page_size, true);

    return NVME_SUCCESS;
}
