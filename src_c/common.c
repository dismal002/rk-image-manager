#include "rkimgman.h"
#include <string.h>

const uint8_t RKAF_SIGNATURE[4] = {'R', 'K', 'A', 'F'};
const uint8_t RKFW_SIGNATURE[4] = {'R', 'K', 'F', 'W'};
const uint8_t RKFP_SIGNATURE[4] = {'R', 'K', 'F', 'P'};

uint64_t recover_true_size(uint32_t stored, uint64_t available) {
    uint64_t stored_u64 = stored;
    if (available <= stored_u64) {
        return stored_u64;
    }
    return stored_u64 + (((available - stored_u64) >> 32) << 32);
}

void put_u32_le(uint8_t *slice, uint32_t value) {
    slice[0] = value & 0xFF;
    slice[1] = (value >> 8) & 0xFF;
    slice[2] = (value >> 16) & 0xFF;
    slice[3] = (value >> 24) & 0xFF;
}

uint32_t get_u32_le(const uint8_t *slice) {
    return (uint32_t)slice[0] |
           ((uint32_t)slice[1] << 8) |
           ((uint32_t)slice[2] << 16) |
           ((uint32_t)slice[3] << 24);
}
