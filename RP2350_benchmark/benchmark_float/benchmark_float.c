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
void float_add()
{
    volatile float a = 0;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a += 1.0f;
    }
}

void float_sub()
{
    volatile float a = 0;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a -= 1.0f;
    }
}

void float_mul()
{
    volatile float a = 1.0;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a *= 1.000001f;
    }
}

void float_div()
{
    volatile float a = 1.0;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a /= 1.000001f;
    }
}

void float_sinf()
{
    volatile float a;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a = sinf((float)i / (float)NUM_OPERATIONS);
    }
}

void float_cosf()
{
    volatile float a;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a = cosf((float)i / (float)NUM_OPERATIONS);
    }
}

void float_atan2f()
{
    volatile float a;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        float x = (float)i / (float)NUM_OPERATIONS;
        float y = 1.0f - x;
        a = atan2f(x, y);
    }
}

void float_sqrtf()
{
    volatile float a;
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        a = sqrtf(i);
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(1000);
    printf("Start Float Benchmark\n");
    MEASURE_N(1, float_add());
    MEASURE_N(1, float_sub());
    MEASURE_N(1, float_mul());
    MEASURE_N(1, float_div());
    MEASURE_N(1, float_sinf());
    MEASURE_N(1, float_cosf());
    MEASURE_N(1, float_atan2f());
    MEASURE_N(1, float_sqrtf());
    sleep_ms(0xffffffff);
}
