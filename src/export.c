#include "defines.h"

#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#ifndef CALCULATOR

#define m8(x)  ((x)&255)
#define mr8(x) (((x)>>8)&255)
#define m16(x) ((x)&65535)

static char *str_dup(const char *s) {
    char *d = malloc(strlen(s)+1);  // allocate memory
    if (d) strcpy(d, s);                 // copy string if okay
    return d;                            // return new memory
}

char *str_dupcat(const char *s, const char *c) {
    if (!s) {
        return str_dup(c);
    } else
    if (!c) {
        return str_dup(s);
    }
    char *d = malloc(strlen(s)+strlen(c)+1);
    if (d) { strcpy(d, s); strcat(d, c); }
    return d;
}

void export_program(const char *name, uint8_t *data, size_t size) {
    const uint8_t header[] = {0x2A, 0x2A, 0x54, 0x49, 0x38, 0x33, 0x46, 0x2A, 0x1A, 0x0A};
    uint8_t len_high;
    uint8_t len_low;
    unsigned int data_size;
    unsigned int i,checksum;
    FILE *out_file;

    // gather structure information
    uint8_t *output = calloc(0x10100, sizeof(uint8_t));
    unsigned int offset = size + 0x4A;

    // Write header bytes
    memcpy(output, header, sizeof header);

    // write name
    memcpy(&output[0x3C], name, strlen(name));
    memcpy(&output[0x4A], data, size);

    // write config bytes
    output[0x37] = 0x0D;
    output[0x3B] = 0x06;
    output[0x45] = 0x00;

    data_size = offset - 0x37;
    len_high = mr8(data_size);
    len_low = m8(data_size);
    output[0x35] = len_low;
    output[0x36] = len_high;

    data_size = offset - 0x4A;
    len_high = mr8(data_size);
    len_low = m8(data_size);
    output[0x48] = len_low;
    output[0x49] = len_high;

    // size bytes
    data_size += 2;
    len_high = mr8(data_size);
    len_low = m8(data_size);
    output[0x39] = len_low;
    output[0x3A] = len_high;
    output[0x46] = len_low;
    output[0x47] = len_high;

    // calculate checksum
    checksum = 0;
    for (i = 0x37; i < offset; ++i) {
        checksum = m16(checksum + output[i]);
    }

    output[offset++] = m8(checksum);
    output[offset++] = mr8(checksum);

    // write the buffer to the file
    char *file_name = str_dupcat(name, ".8xp");

    if (file_name && !(out_file = fopen(file_name, "wb"))) {
        free(file_name);
        free(output);
        fprintf(stderr, "Unable to open output program file.");
        exit(1);
    }

    fwrite(output, 1, offset, out_file);

    // close the file
    fclose(out_file);

    // free the memory
    free(file_name);
    free(output);
}

#endif
