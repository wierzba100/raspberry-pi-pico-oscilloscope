#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/adc.h"
#include "hardware/dma.h"

#define ADC0_CAPTURE_CHANNEL 0
#define ADC1_CAPTURE_CHANNEL 1

#define ADC_VREF 3.3
#define ADC_RANGE (1 << 8)
#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

#define CAPTURE_DEPTH 10

uint8_t capture_buf_0[CAPTURE_DEPTH];
uint8_t capture_buf_1[CAPTURE_DEPTH];

int main() {
    stdio_init_all();

    adc_gpio_init(26 + ADC0_CAPTURE_CHANNEL);
    adc_gpio_init(26 + ADC1_CAPTURE_CHANNEL);

    adc_init();

    adc_fifo_setup(
            true,    // Write each completed conversion to the sample FIFO
            true,    // Enable DMA data request (DREQ)
            1,       // DREQ (and IRQ) asserted when at least 1 sample present
            false,   // We won't see the ERR bit because of 8 bit reads; disable.
            true     // Shift each sample to 8 bits when pushing to FIFO
    );

    adc_set_clkdiv(0); //max speed

    sleep_ms(1000);
    // Set up the DMA to start transferring data as soon as it appears in FIFO

    //channel 0 dma
    uint dma_chan_0 = dma_claim_unused_channel(true);
    dma_channel_config cfg_0 = dma_channel_get_default_config(dma_chan_0);

    //channel 1 dma
    uint dma_chan_1 = dma_claim_unused_channel(true);
    dma_channel_config cfg_1 = dma_channel_get_default_config(dma_chan_1);

    while(1)
    {
        adc_select_input(ADC0_CAPTURE_CHANNEL);

        // Reading from constant address, writing to incrementing byte addresses
        channel_config_set_transfer_data_size(&cfg_0, DMA_SIZE_8);
        channel_config_set_read_increment(&cfg_0, false);
        channel_config_set_write_increment(&cfg_0, true);

        // Pace transfers based on availability of ADC samples
        channel_config_set_dreq(&cfg_0, DREQ_ADC);

        dma_channel_configure(dma_chan_0, &cfg_0,
                              capture_buf_0,    // dst
                              &adc_hw->fifo,  // src
                              CAPTURE_DEPTH,  // transfer count
                              true            // start immediately
        );

        adc_run(true);

        // Once DMA finishes, stop any new conversions from starting, and clean up
        // the FIFO in case the ADC was still mid-conversion.
        dma_channel_wait_for_finish_blocking(dma_chan_0);
        printf("Capture 0 finished\n");
        adc_run(false);
        adc_fifo_drain();

        adc_select_input(ADC1_CAPTURE_CHANNEL);

        // Reading from constant address, writing to incrementing byte addresses
        channel_config_set_transfer_data_size(&cfg_1, DMA_SIZE_8);
        channel_config_set_read_increment(&cfg_1, false);
        channel_config_set_write_increment(&cfg_1, true);

        // Pace transfers based on availability of ADC samples
        channel_config_set_dreq(&cfg_1, DREQ_ADC);

        dma_channel_configure(dma_chan_1, &cfg_1,
                              capture_buf_1,    // dst
                              &adc_hw->fifo,  // src
                              CAPTURE_DEPTH,  // transfer count
                              true            // start immediately
        );

        adc_run(true);

        // Once DMA finishes, stop any new conversions from starting, and clean up
        // the FIFO in case the ADC was still mid-conversion.
        dma_channel_wait_for_finish_blocking(dma_chan_1);
        printf("Capture 1 finished\n");
        adc_run(false);
        adc_fifo_drain();


        // Print samples to stdout so you can display them in pyplot, excel, matlab
        for (int i = 0; i < CAPTURE_DEPTH; ++i) {
            printf("1:%.2f, 2:%.2f, ", (capture_buf_0[i] * ADC_CONVERT), (capture_buf_1[i] * ADC_CONVERT));
            if (i % 6 == 5)
                printf("\n");
        }
        sleep_ms(100);
    }

    return 0;
}
