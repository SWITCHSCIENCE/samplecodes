#include <stdio.h>
#include <math.h>
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
void double_add()
{
    volatile double a = 0;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a += 1.0;
    }
}

void double_sub()
{
    volatile double a = 0;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a -= 1.0;
    }
}

void double_mul()
{
    volatile double a = 1.0;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a *= 1.000001;
    }
}

void double_div()
{
    volatile double a = 1.0;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a /= 1.000001;
    }
}

void double_sin()
{
    volatile double a;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a = sin((double)i / (double)NUM_OPERATIONS);
    }
}

void double_cos()
{
    volatile double a;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a = cos((double)i / (double)NUM_OPERATIONS);
    }
}

void double_atan2()
{
    volatile double a;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        double x = (double)i / (double)NUM_OPERATIONS;
        double y = 1.0 - x;
        a = atan2(x, y);
    }
}

void double_sqrt()
{
    volatile double a;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a = sqrt(i);
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(1000);
    printf("Start double Benchmark\n");
    MEASURE_N(1, double_add());
    MEASURE_N(1, double_sub());
    MEASURE_N(1, double_mul());
    MEASURE_N(1, double_div());
    MEASURE_N(1, double_sin());
    MEASURE_N(1, double_cos());
    MEASURE_N(1, double_atan2());
    MEASURE_N(1, double_sqrt());
    sleep_ms(0xffffffff);
}
