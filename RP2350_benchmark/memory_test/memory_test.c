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
        PROC;                                                    \
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

#define FLASH_AREA_SIZE ((1 * 1024 * 1024) / 4)
static const uint32_t dammy_flash_area[FLASH_AREA_SIZE + 16] = {0};

#define SRAM_AREA_SIZE ((192 * 1024) / 4)
static uint32_t sram_buffer[SRAM_AREA_SIZE + 16] = {0};

static volatile uint32_t read_dat;
static volatile uint32_t read_dat16[16];
static uint32_t next = 1;

uint32_t rand()
{
    next = next * 1103515245 + 12345;
    return next;
}

void srand(uint32_t seed)
{
    next = seed;
}

#define NUM_OPERATIONS 1000000
void flash_seq_read()
{
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        uint32_t x = i % FLASH_AREA_SIZE;
        read_dat = dammy_flash_area[x];
    }
}

void flash_random_read()
{
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        uint32_t x = rand() % FLASH_AREA_SIZE;
        read_dat = dammy_flash_area[x];
    }
}

void flash_bulk_read()
{
    for (int i = 0; i < NUM_OPERATIONS; i+=16)
    {
        uint32_t x = i % FLASH_AREA_SIZE;
        read_dat16[0] = dammy_flash_area[x + 0];
        read_dat16[1] = dammy_flash_area[x + 1];
        read_dat16[2] = dammy_flash_area[x + 2];
        read_dat16[3] = dammy_flash_area[x + 3];
        read_dat16[4] = dammy_flash_area[x + 4];
        read_dat16[5] = dammy_flash_area[x + 5];
        read_dat16[6] = dammy_flash_area[x + 6];
        read_dat16[7] = dammy_flash_area[x + 7];
        read_dat16[8] = dammy_flash_area[x + 8];
        read_dat16[9] = dammy_flash_area[x + 9];
        read_dat16[10] = dammy_flash_area[x + 10];
        read_dat16[11] = dammy_flash_area[x + 11];
        read_dat16[12] = dammy_flash_area[x + 12];
        read_dat16[13] = dammy_flash_area[x + 13];
        read_dat16[14] = dammy_flash_area[x + 14];
        read_dat16[15] = dammy_flash_area[x + 15];
    }
}

void sram_random_read()
{
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        uint32_t x = rand() % SRAM_AREA_SIZE;
        read_dat = sram_buffer[x];
    }
}

void sram_random_write()
{
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        uint32_t x = rand() % SRAM_AREA_SIZE;
        sram_buffer[x] = i;
    }
}

void sram_seq_read()
{
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        uint32_t x = i % SRAM_AREA_SIZE;
        read_dat = sram_buffer[x];
    }
}

void sram_seq_write()
{
    for (int i = 0; i < NUM_OPERATIONS; i++)
    {
        uint32_t x = i % SRAM_AREA_SIZE;
        sram_buffer[x] = i;
    }
}

void sram_bulk_read()
{
    for (int i = 0; i < NUM_OPERATIONS; i+=16)
    {
        uint32_t x = i % SRAM_AREA_SIZE;
        read_dat16[0] = sram_buffer[x + 0];
        read_dat16[1] = sram_buffer[x + 1];
        read_dat16[2] = sram_buffer[x + 2];
        read_dat16[3] = sram_buffer[x + 3];
        read_dat16[4] = sram_buffer[x + 4];
        read_dat16[5] = sram_buffer[x + 5];
        read_dat16[6] = sram_buffer[x + 6];
        read_dat16[7] = sram_buffer[x + 7];
        read_dat16[8] = sram_buffer[x + 8];
        read_dat16[9] = sram_buffer[x + 9];
        read_dat16[10] = sram_buffer[x + 10];
        read_dat16[11] = sram_buffer[x + 11];
        read_dat16[12] = sram_buffer[x + 12];
        read_dat16[13] = sram_buffer[x + 13];
        read_dat16[14] = sram_buffer[x + 14];
        read_dat16[15] = sram_buffer[x + 15];
    }
}

void sram_bulk_write()
{
    for (int i = 0; i < NUM_OPERATIONS; i+=16)
    {
        uint32_t x = i % SRAM_AREA_SIZE;
        sram_buffer[x + 0] = i;
        sram_buffer[x + 1] = i;
        sram_buffer[x + 2] = i;
        sram_buffer[x + 3] = i;
        sram_buffer[x + 4] = i;
        sram_buffer[x + 5] = i;
        sram_buffer[x + 6] = i;
        sram_buffer[x + 7] = i;
        sram_buffer[x + 8] = i;
        sram_buffer[x + 9] = i;
        sram_buffer[x + 10] = i;
        sram_buffer[x + 11] = i;
        sram_buffer[x + 12] = i;
        sram_buffer[x + 13] = i;
        sram_buffer[x + 14] = i;
        sram_buffer[x + 15] = i;
    }
}

int main()
{
    stdio_init_all();
    sleep_ms(1000);
    printf("Start Memory R/W Tests\n");
    srand(0xdeadbeef);
    for (int i = 0; i < 100; i++)
    {
        printf("%u\n", rand());
    }
    MEASURE_N(1, flash_seq_read());
    MEASURE_N(1, flash_random_read());
    MEASURE_N(1, flash_bulk_read());
    MEASURE_N(1, sram_seq_read());
    MEASURE_N(1, sram_seq_write());
    MEASURE_N(1, sram_random_read());
    MEASURE_N(1, sram_random_write());
    MEASURE_N(1, sram_bulk_read());
    MEASURE_N(1, sram_bulk_write());
    sleep_ms(0xffffffff);
}
