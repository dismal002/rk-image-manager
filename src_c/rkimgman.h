#ifndef AFPTOOL_H
#define AFPTOOL_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define MAX_PARTS 16
#define MAX_NAME_LEN 32
#define MAX_FULL_PATH_LEN 60
#define MAX_MODEL_LEN 34
#define MAX_ID_LEN 30
#define MAX_MANUFACTURER_LEN 56
#define UPDATE_HEADER_SIZE 2048
#define UPDATE_PART_SIZE 112

extern const uint8_t RKAF_SIGNATURE[4];
extern const uint8_t RKFW_SIGNATURE[4];
extern const uint8_t RKFP_SIGNATURE[4];

#pragma pack(push, 1)

typedef struct {
    uint8_t name[MAX_NAME_LEN];
    uint8_t full_path[MAX_FULL_PATH_LEN];
    uint32_t flash_size;
    uint32_t part_offset;
    uint32_t flash_offset;
    uint32_t padded_size;
    uint32_t part_byte_count;
} UpdatePart;

typedef struct {
    uint8_t magic[4];
    uint32_t length;
    uint8_t model[MAX_MODEL_LEN];
    uint8_t id[MAX_ID_LEN];
    uint8_t manufacturer[MAX_MANUFACTURER_LEN];
    uint32_t unknown1;
    uint32_t version;
    uint32_t num_parts;
    UpdatePart parts[MAX_PARTS];
    uint8_t reserved[116];
} UpdateHeader;

#pragma pack(pop)

// Utility functions
uint64_t recover_true_size(uint32_t stored, uint64_t available);
uint32_t rkcrc32(uint32_t crc, const uint8_t *data, size_t len);
uint32_t parm_crc32(const uint8_t *data, size_t len);
bool is_valid_parm_blob(const uint8_t *data, size_t len);
void put_u32_le(uint8_t *slice, uint32_t value);
uint32_t get_u32_le(const uint8_t *slice);
uint8_t chip_name_to_code(const char *chip);

// Core commands
int unpack_file(const char *file_path, const char *dst_path);
int pack_rkfw(const char *input_dir, const char *output_file, const char *chip, const char *version, int64_t timestamp, const char *code_hex);
int pack_rkaf(const char *input_dir, const char *output_file, const char *model, const char *manufacturer);

#endif // AFPTOOL_H
