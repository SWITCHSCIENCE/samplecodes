#include "pico/stdlib.h"

const int GPIO_PIN = 0;

int main() {
    gpio_init(GPIO_PIN);
    gpio_set_dir(GPIO_PIN, GPIO_OUT);
    do {
        gpio_put(GPIO_PIN, 1);
        gpio_put(GPIO_PIN, 0);
    } while(1);
}
