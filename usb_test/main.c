#include <stdio.h>
#include "pico/stdlib.h"
#include "tusb.h"

int main() {
    stdio_init_all();

    // Wait until USB CDC is connected
    while (!tud_cdc_connected()) {
        sleep_ms(100);
    }

    while (1) {
        printf("Hello from Pico USB Serial!\n");
        sleep_ms(1000);
    }
}
