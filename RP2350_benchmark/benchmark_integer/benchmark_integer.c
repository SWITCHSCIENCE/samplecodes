#include <stdio.h>
// #include <stdlib.h>
#include <string.h>
#include "pico/time.h"
#include "pico/rand.h"
#include "pico/stdlib.h"

#define MEASURE(PROC)                                              \
    do                                                             \
    {                                                              \
        printf(PROCSTR(PROC) "...");                               \
        uint32_t start_time = time_us_32();                        \
        PROC;                                                      \
        uint32_t end_time = time_us_32();                          \
        printf(" processing time %u us\n", end_time - start_time); \
    } while (0);

#define PROCSTR(p) #p
#define MEASURE_N(N, PROC)    \
    for (int i; i < (N); i++) \
    {                         \
        MEASURE(PROC);        \
        sleep_ms(100);        \
    }

#define NUM_OPERATIONS 1000000
void int_add()
{
    volatile int a = 0;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a += 1;
    }
}

void int_sub()
{
    volatile int a = 0;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a -= 1;
    }
}

void int_mul()
{
    volatile int a = 1;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a *= 2;
    }
}

void int_div()
{
    volatile int a = 100000000;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a /= 2;
        if (a <= 0)
        {
            a = 100000000;
        }
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(1000);
    printf("Start Integer Benchmark\n");
    MEASURE_N(1, int_add());
    MEASURE_N(1, int_sub());
    MEASURE_N(1, int_mul());
    MEASURE_N(1, int_div());
    sleep_ms(0xffffffff);
}
