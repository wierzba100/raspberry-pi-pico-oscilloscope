#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/adc.h"

#define ADC_NUM 0
#define ADC_PIN (26 + ADC_NUM)
#define ADC_VREF 3.3
#define ADC_RANGE (1 << 12)
#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

int main() {
    stdio_init_all();

    adc_init();
    adc_gpio_init(ADC_PIN);
    adc_select_input(ADC_NUM);

    uint adc_raw;
    while (1) {
        adc_raw = adc_read(); // raw voltage from ADC
        printf("%.2f\n", (adc_raw * ADC_CONVERT));
        sleep_ms(10);
    }

    return 0;
}
