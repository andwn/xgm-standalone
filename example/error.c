#include <stdint.h>

#include "sys.h"
#include "vdp.h"

#define isdigit(c)      ((c) >= '0' && (c) <= '9')

typedef void *__gnuc_va_list;
typedef __gnuc_va_list va_list;

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#define va_start(AP, LASTARG)                                           \
 (AP = ((__gnuc_va_list) __builtin_next_arg (LASTARG)))

#define va_end(AP)      ((void)0)

#define va_arg(AP, TYPE)                                                \
 (AP = (__gnuc_va_list) ((char *) (AP) + __va_rounded_size (TYPE)),     \
  *((TYPE *) (void *) ((char *) (AP)                                    \
                       - ((sizeof (TYPE) < __va_rounded_size (char)     \
                           ? sizeof (TYPE) : __va_rounded_size (TYPE))))))

static const char hexchars[] = "0123456789ABCDEF";

uint16_t strlen(const char *str) {
    const char *src = str;
    while(*src++);
    return (src - str) - 1;
}

uint16_t strnlen(const char *str, uint16_t maxlen) {
    const char *src;
    for(src = str; maxlen-- && *src != '\0'; ++src);
    return src - str;
}

static uint16_t skip_atoi(const char **s) {
    uint16_t i = 0;
    while(isdigit(**s)) {
        i = (i * 10) + *((*s)++) - '0';
    }
    return i;
}

static uint16_t vsprintf(char *buf, const char *fmt, va_list args) {
    char tmp_buffer[12];
    char *str;
    for (str = buf; *fmt; ++fmt) {
        if (*fmt != '%') {
            *str++ = *fmt;
            continue;
        }
        int16_t field_width = -1;
        int16_t precision = -1;
        uint16_t longint = 0;
        uint16_t zero_pad = 0;
        uint16_t left_align = 0;
        char sign = 0;
        char *s;

        // Process the flags
        for(;;) {
            ++fmt;          // this also skips first '%'
            switch (*fmt) {
                case '-':
                    left_align = 1;
                    continue;
                case '+':
                    sign = '+';
                    continue;
                case ' ':
                    if (sign != '+') sign = ' ';
                    continue;
                case '0':
                    zero_pad = 1;
                    continue;
            }
            break;
        }

        // Process field width and precision
        field_width = precision = -1;
        if (isdigit(*fmt)) {
            field_width = skip_atoi(&fmt);
        } else if (*fmt == '*') {
            ++fmt;
            // it's the next argument
            field_width = va_arg(args, int16_t);
            if (field_width < 0) {
                field_width = -field_width;
                left_align = 1;
            }
        }

        if (*fmt == '.') {
            ++fmt;
            if (isdigit(*fmt)) {
                precision = skip_atoi(&fmt);
            } else if (*fmt == '*') {
                ++fmt;
                // it's the next argument
                precision = va_arg(args, int16_t);
            }
            if (precision < 0) {
                precision = 0;
            }
        }

        if (*fmt == 'h') ++fmt;
        if ((*fmt == 'l') || (*fmt == 'L')) {
            longint = 1;
            ++fmt;
        }
        if (left_align) {
            zero_pad = 0;
        }
        switch (*fmt) {
            case 'c': {
                if (!left_align) {
                    while (--field_width > 0) {
                        *str++ = ' ';
                    }
                }
                *str++ = (char) va_arg(args, int16_t);
                while (--field_width > 0) {
                    *str++ = ' ';
                }
                continue;
            }
            case 's': {
                s = va_arg(args, char*);
                if (!s) {
                    s = "<NULL>";
                }
                int16_t len = (int16_t) strnlen(s, precision);
                if (!left_align) {
                    while (len < field_width--) {
                        *str++ = ' ';
                    }
                }
                for (int16_t i = 0; i < len; ++i) {
                    *str++ = *s++;
                }
                while (len < field_width--) {
                    *str++ = ' ';
                }
                continue;
            }

            case 'p':
                longint = 1;
                if (field_width == -1) {
                    field_width = 2 * sizeof(void *);
                    zero_pad = 1;
                } /* fallthrough */
            case 'x':
            case 'X': {
                s = &tmp_buffer[12];
                *--s = 0;
                uint32_t num = longint ? va_arg(args, uint32_t) : va_arg(args, uint16_t);
                if (!num) {
                    *--s = '0';
                }
                while (num) {
                    *--s = hexchars[num & 0xF];
                    num >>= 4;
                }
                sign = 0;
                break;
            }
            case 'n': {
                int16_t *ip = va_arg(args, int16_t * );
                *ip = (str - buf);
                continue;
            }
            case 'u': {
                s = &tmp_buffer[12];
                *--s = 0;
                uint32_t num = longint ? va_arg(args, uint32_t) : va_arg(args, uint16_t);
                if (!num) {
                    *--s = '0';
                }
                while (num) {
                    *--s = (num % 10) + 0x30;
                    num /= 10;
                }
                sign = 0;
                break;
            }
            case 'd':
            case 'i': {
                s = &tmp_buffer[12];
                *--s = 0;
                int32_t num = longint ? va_arg(args, int32_t) : va_arg(args, int16_t);
                if (!num) {
                    *--s = '0';
                }
                if (num < 0) {
                    sign = '-';
                    while (num) {
                        *--s = 0x30 - (num % 10);
                        num /= 10;
                    }
                } else {
                    //num = 0;
                    while (num) {
                        *--s = (num % 10) + 0x30;
                        num /= 10;
                    }
                }
                break;
            }
            default: continue;
        }

        int16_t len = (int16_t) strnlen(s, precision);
        if (sign) {
            *str++ = sign; //'-';
            field_width--;
        }
        if (!left_align) {
            if (zero_pad) {
                while(len < field_width--)
                    *str++ = '0';
            } else {
                while(len < field_width--)
                    *str++ = ' ';
            }
        }
        for (int16_t i = 0; i < len; ++i) {
            *str++ = *s++;
        }
        while(len < field_width--) {
            *str++ = ' ';
        }
    }

    *str = '\0';
    return str - buf;
}

uint16_t sprintf(char *buffer, const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    uint16_t i = vsprintf(buffer, fmt, args);
    va_end(args);
    return i;
}

extern uint32_t v_err_reg[8+8];
extern uint32_t v_err_pc;
extern uint32_t v_err_addr;
extern uint16_t v_err_ext1;
extern uint16_t v_err_ext2;
extern uint16_t v_err_sr;
extern uint8_t v_err_type;

__attribute__((noreturn))
void _error() {
    char buf[40];
    vdp_init();

    // Error type
    uint16_t x = 2, y = 2;
    switch(v_err_type) {
        case 1:
            sprintf(buf, "%s", "Address Error");
            vdp_color(0, 0x800);
            break;
        case 2:
            sprintf(buf, "%s", "Bad Instruction");
            vdp_color(0, 0x008);
            break;
        case 3:
            sprintf(buf, "%s", "Divide by Zero");
            vdp_color(0, 0x046);
            break;
        case 4:
            sprintf(buf, "%s", "Out of Memory");
            vdp_color(0, 0x080);
            break;
        default:
            sprintf(buf, "%s", "Fatal Error");
            vdp_color(0, 0x404);
    }
    vdp_puts(VDP_PLANE_A, buf, x, y);

    // Registers
    x = 2; y = 4;
    for(uint16_t i = 0; i < 8; i++) {
        sprintf(buf, "D%hu=%08lX", i, v_err_reg[i]);
        vdp_puts(VDP_PLANE_A, buf, x, y + i);
    }
    for(uint16_t i = 0; i < 8; i++) {
        sprintf(buf, "A%hu=%08lX", i, v_err_reg[i+8]);
        vdp_puts(VDP_PLANE_A, buf, x, y + i + 8);
    }

    // Other values
    x = 15;
    switch(v_err_type) {
        case 0:
        case 1: // Bus / Address Error
            y = 20;
            sprintf(buf, "FUNC=%04hX   INST=%04hX", v_err_ext1, v_err_ext2);
            vdp_puts(VDP_PLANE_A, buf, x, y++);
            sprintf(buf, "ADDR=%06lX", v_err_addr);
            vdp_puts(VDP_PLANE_A, buf, x, y++);
            break;
        case 2: // Illegal
            y = 21;
            sprintf(buf, "OV=%04hX", v_err_ext1);
            vdp_puts(VDP_PLANE_A, buf, x, y++);
            break;
    }
    x = 2; y = 20;
    sprintf(buf, "PC=%06lX", v_err_pc);
    vdp_puts(VDP_PLANE_A, buf, x, y++);
    sprintf(buf, "SR=  %04hX", v_err_sr);
    vdp_puts(VDP_PLANE_A, buf, x, y++);

    // Stack dump
    x = 25; //y = 2;
    y = 4;
    uint32_t *sp = (uint32_t*) v_err_reg[15];
    for(uint16_t i = 0; i < 16; i++) {
        // Prevent wrapping around after reaching top of the stack
        if((uint32_t) sp < 0xFFF000) break;
        x = 15;
        sprintf(buf, "SP+%02X=%08lX", i << 3, *sp);
        vdp_puts(VDP_PLANE_A, buf, x, y);
        sp++;

        if((uint32_t) sp < 0xFFF000) break;
        x = 30;
        sprintf(buf, "%08lX", *sp);
        vdp_puts(VDP_PLANE_A, buf, x, y);
        sp++;

        y++;
    }
    // R.I.P
    while(1);
}
