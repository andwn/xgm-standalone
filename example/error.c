#include <stdint.h>

#include "vdp.h"

extern uint32_t v_err_reg[8+8];
extern uint32_t v_err_pc;
extern uint32_t v_err_addr;
extern uint16_t v_err_ext1;
extern uint16_t v_err_ext2;
extern uint16_t v_err_sr;
extern uint8_t v_err_type;

static const char STR_ERROR[4][16] = {
        "Fatal Error", "Address Error", "Bad Instruction", "Divide by Zero"
};
static const char hexchars[16] = "0123456789ABCDEF";

void print_reg(char *str, char kind, uint16_t index, uint16_t x, uint16_t y) {
    index &= 7;
    str[0] = kind;
    str[1] = '0' + index;
    str[2] = '=';
    str[3] = 0;
    vdp_puts(VDP_PLANE_A, str, x, y);
}

void print_hex(char *str, uint32_t val, uint16_t digits, uint16_t x, uint16_t y) {
    if(digits > 8) digits = 8;
    for(uint16_t i = 0; i < digits; i++) {
        str[digits - i - 1] = hexchars[(val >> (i << 2)) & 0xF];
    }
    str[digits] = 0;
    vdp_puts(VDP_PLANE_A, str, x, y);
}

__attribute__((noreturn))
void _error() {
    char buf[16];

    vdp_init();
    // Error type
    vdp_puts(VDP_PLANE_A, STR_ERROR[v_err_type & 3], 2, 2);
    // Registers
    uint16_t x = 2, y = 4;
    for(uint16_t i = 0; i < 8; i++) {
        print_reg(buf, 'D', i, x, y);
        print_hex(buf, v_err_reg[i], 8, x + 3, y);
        y++;
    }
    for(uint16_t i = 0; i < 8; i++) {
        print_reg(buf, 'A', i, x, y);
        print_hex(buf, v_err_reg[i+8], 8, x + 3, y);
        y++;
    }
    // Other values
    x = 15;
    switch(v_err_type) {
        case 0:
        case 1: // Bus / Address Error
            y = 20;
            vdp_puts(VDP_PLANE_A, "FUNC=", x, y);
            print_hex(buf, v_err_ext1, 4, x + 5, y);
            vdp_puts(VDP_PLANE_A, "INST=", x + 12, y);
            print_hex(buf, v_err_ext2, 4, x + 17, y);
            y++;
            vdp_puts(VDP_PLANE_A, "ADDR=", x, y);
            print_hex(buf, v_err_addr, 6, x + 5, y);
            break;
        case 2: // Illegal
            y = 21;
            vdp_puts(VDP_PLANE_A, "OV=", x, y);
            print_hex(buf, v_err_ext1, 4, x + 3, y);
            break;
    }
    x = 2; y = 20;
    vdp_puts(VDP_PLANE_A, "PC=", x, y);
    print_hex(buf, v_err_pc, 6, x + 3, y);
    y++;
    vdp_puts(VDP_PLANE_A, "SR=  ", x, y);
    print_hex(buf, v_err_sr, 4, x + 5, y);

    // Stack dump
    x = 25; y = 4;
    uint32_t *sp = (uint32_t*) v_err_reg[15];
    for(uint16_t i = 0; i < 16; i++) {
        // Prevent wrapping around after reaching top of the stack
        if((uint32_t) sp < 0xFFF000) break;
        x = 15;
        vdp_puts(VDP_PLANE_A, "SP+", x, y);
        print_hex(buf, i << 3, 2, x + 3, y);
        vdp_puts(VDP_PLANE_A, "=", x + 5, y);
        print_hex(buf, *sp, 8, x + 6, y);
        sp++;

        if((uint32_t) sp < 0xFFF000) break;
        x = 30;
        print_hex(buf, *sp, 8, x, y);
        sp++;

        y++;
    }
    // R.I.P
    while(1);
}
