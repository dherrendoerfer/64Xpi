/*
  8bit PI calculation w. 32bit numerator (slow but gets to 1000+ digits)
  Uses Leibniz series as described by Carl D. Offner

  original by 比尔盖子 posted on stack overflow in 2019
  see: https://stackoverflow.com/questions/56022623/avoid-overflow-when-calculating-%CF%80-by-evaluating-a-series-using-16-bit-arithmetic
 
  adapted for cc65 and 64Xpi on bad6502 by dherrendoerfer
*/

#include <stdint.h>
#include "pi_libc.h"

#define N 2160
#define PRECISION 10

#define BUF_SIZE 4

uint16_t terms[N + 1] = {0};

int main(void)
{
    size_t i,j;
    uint16_t plc = 0;
    uint16_t carry = 0;
    uint16_t digit[BUF_SIZE];
    int8_t idx = 0;

    /* initialize the initial terms */
    for (i = 0; i < N + 1; i++) {
        terms[i] = 2;
    }

    printf("HAPPY PI DAY 2025 !!!\n Pi:\n");

    for (j = 0; j < N / 4; j++) {
        uint32_t numerator = 0;
        uint16_t denominator;

        for (i = N; i > 0; i--) {
            numerator += terms[i] * PRECISION;
            denominator = 2 * i + 1;

            terms[i] = numerator % denominator;
            numerator /= denominator;
            numerator *= i;
        }
        numerator += terms[0] * PRECISION;
        digit[idx] = numerator / PRECISION + carry;


        /* over 9, needs at least one carry op. */
        if (digit[idx] > 9) {
            for (i = 1; i <= 4; i++) {
                if (i > 3) {
                    /* allow up to 3 consecutive carry ops */
                    printf("ERROR: too many carry ops!\n");
                    return 1;
                }
                /* erase a digit */
                putchar('\b');

                /* carry */
                digit[idx] -= 10;
                idx--;
                if (idx < 0) {
                    idx = BUF_SIZE - 1;
                }
                digit[idx]++;  
                if (digit[idx] < 10) {
                    /* done! reprint the digits */
                    for (j = 0; j <= i; j++) {
                        printf("%01u", digit[idx]);
                        idx++;
                        if (idx > BUF_SIZE - 1) {
                            idx = 0;
                        }
                    }
                    break;
                }
            }
        }
        else {
            printf("%01u", digit[idx]);
            if (plc == 0) {
                printf(".");
                plc++;
            }
            fsync(stdout);         
        }

        carry = numerator % PRECISION;
        terms[0] = 0;

        /* put an element to the ring buffer */
        idx++;
        if (idx > BUF_SIZE - 1) {
            idx = 0;
        }
    }
    putchar('\n');
}