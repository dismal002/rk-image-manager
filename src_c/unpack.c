#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "rkimgman.h"

static int mkdir_p(const char *path) {
    char tmp[256];
    char *p = NULL;
    size_t len;
    snprintf(tmp, sizeof(tmp), "%s", path);
    len = strlen(tmp);
    if (tmp[len - 1] == '/') tmp[len - 1] = 0;
    for (p = tmp + 1; *p; p++) {
        if (*p == '/') {
            *p = 0;
            if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST) return -1;
            *p = '/';
        }
    }
    if (mkdir(tmp, S_IRWXU) != 0 && errno != EEXIST) return -1;
    return 0;
}

static int extract_file(FILE *fp, uint64_t offset, uint64_t len, const char *full_path) {
    printf("%08llx-%08llx %s\n", (unsigned long long)offset, (unsigned long long)len, full_path);
    FILE *out = fopen(full_path, "wb");
    if (!out) return -1;
    fseeko(fp, offset, SEEK_SET);
    char buf[4096];
    uint64_t remaining = len;
    while (remaining > 0) {
        size_t to_read = (remaining < sizeof(buf)) ? remaining : sizeof(buf);
        size_t r = fread(buf, 1, to_read, fp);
        if (r == 0) break;
        fwrite(buf, 1, r, out);
        remaining -= r;
    }
    fclose(out);
    return (remaining == 0) ? 0 : -1;
}

int unpack_rkfw(const char *file_path, const char *dst_path) {
    FILE *fp = fopen(file_path, "rb");
    if (!fp) return 1;
    uint8_t buf[0x66];
    if (fread(buf, 1, 0x66, fp) != 0x66) {
        fclose(fp);
        return 1;
    }
    mkdir_p(dst_path);
    char out_path[4096];
    snprintf(out_path, sizeof(out_path), "%s/rkfw-header.bin", dst_path);
    FILE *hf = fopen(out_path, "wb");
    if (hf) { fwrite(buf, 1, 0x66, hf); fclose(hf); }

    uint64_t ioff = get_u32_le(&buf[0x19]);
    uint64_t isize = get_u32_le(&buf[0x1d]);
    snprintf(out_path, sizeof(out_path), "%s/BOOT", dst_path);
    extract_file(fp, ioff, isize, out_path);

    ioff = get_u32_le(&buf[0x21]);
    uint32_t stored_size = get_u32_le(&buf[0x25]);
    
    fseeko(fp, 0, SEEK_END);
    uint64_t filesize = ftello(fp);
    uint64_t isize2 = recover_true_size(stored_size, filesize - ioff);
    snprintf(out_path, sizeof(out_path), "%s/embedded-update.img", dst_path);
    extract_file(fp, ioff, isize2, out_path);

    fclose(fp);
    return 0;
}

int unpack_rkafp(const char *file_path, const char *dst_path) {
    FILE *fp = fopen(file_path, "rb");
    if (!fp) return 1;
    uint8_t buf[UPDATE_HEADER_SIZE];
    if (fread(buf, 1, UPDATE_HEADER_SIZE, fp) != UPDATE_HEADER_SIZE) {
        fclose(fp);
        return 1;
    }
    UpdateHeader *hdr = (UpdateHeader *)buf;
    mkdir_p(dst_path);
    
    char out_path[4096];
    snprintf(out_path, sizeof(out_path), "%s/rkaf-header.bin", dst_path);
    FILE *hf = fopen(out_path, "wb");
    if (hf) { fwrite(buf, 1, UPDATE_HEADER_SIZE, hf); fclose(hf); }

    snprintf(out_path, sizeof(out_path), "%s/Image", dst_path);
    mkdir_p(out_path);

    for (uint32_t i = 0; i < hdr->num_parts; i++) {
        UpdatePart *p = &hdr->parts[i];
        if (strcmp((char *)p->full_path, "SELF") == 0 || strcmp((char *)p->full_path, "RESERVED") == 0) continue;
        
        snprintf(out_path, sizeof(out_path), "%s/%s", dst_path, p->full_path);
        uint64_t true_count = p->part_byte_count; // Simplified size recovery
        
        uint64_t offset = p->part_offset;
        if (strcmp((char *)p->name, "parameter") == 0) {
            offset += 8;
            true_count -= 12; // PARM wrapper overhead simplified
        }
        extract_file(fp, offset, true_count, out_path);
    }

    fclose(fp);
    return 0;
}

int unpack_file(const char *file_path, const char *dst_path) {
    FILE *fp = fopen(file_path, "rb");
    if (!fp) {
        fprintf(stderr, "Cannot open %s\n", file_path);
        return 1;
    }
    uint8_t signature[4];
    if (fread(signature, 1, 4, fp) != 4) {
        fclose(fp);
        return 1;
    }
    fclose(fp);

    if (memcmp(signature, RKAF_SIGNATURE, 4) == 0) {
        return unpack_rkafp(file_path, dst_path);
    } else if (memcmp(signature, RKFW_SIGNATURE, 4) == 0) {
        return unpack_rkfw(file_path, dst_path);
    } else {
        fprintf(stderr, "Unknown signature\n");
        return 1;
    }
}
