#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"

#define ADC0_CAPTURE_CHANNEL 26
#define ADC1_CAPTURE_CHANNEL 27

#define ADC_VREF 3.3
#define ADC_RANGE (1 << 8)
#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

#define CAPTURE_DEPTH 10

uint8_t capture_buf[CAPTURE_DEPTH];
uint8_t * sample_address_pointer = &capture_buf[0];

int main() {
    stdio_init_all();

    adc_gpio_init(ADC0_CAPTURE_CHANNEL);
    adc_gpio_init(ADC1_CAPTURE_CHANNEL);

    adc_init();

    adc_set_round_robin(00001); // Enable round-robin sampling of all 5 inputs.
    adc_select_input(0); // Set starting ADC channel for round-robin mode.

    adc_set_clkdiv(0); // Run at max speed.

    adc_fifo_setup(
            true,    // Write each completed conversion to the sample FIFO
            true,    // Enable DMA data request (DREQ)
            1,       // Assert DREQ (and IRQ) at least 1 sample present
            false,   // Omit ERR bit (bit 15) since we have 8 bit reads.
            true     // shift each sample to 8 bits when pushing to FIFO
    );

    //Channel 1
    uint samp_chan = dma_claim_unused_channel(true);
    dma_channel_config samp_conf = dma_channel_get_default_config(samp_chan);

    channel_config_set_transfer_data_size(&samp_conf, DMA_SIZE_8);
    channel_config_set_read_increment(&samp_conf, false); // read from adc FIFO reg.
    channel_config_set_write_increment(&samp_conf, true);
    channel_config_set_dreq(&samp_conf, DREQ_ADC); // pace data according to ADC

    dma_channel_configure(samp_chan, &samp_conf,
                          capture_buf,    // dst
                          &adc_hw->fifo,  // src
                          CAPTURE_DEPTH,  // transfer count
                          false            // start immediately
    );

    //Channel 2
    uint control_chan = dma_claim_unused_channel(true);
    dma_channel_config control_conf = dma_channel_get_default_config(control_chan);

    channel_config_set_transfer_data_size(&control_conf, DMA_SIZE_32);
    channel_config_set_read_increment(&control_conf, false); // read from adc FIFO reg.
    channel_config_set_write_increment(&control_conf, false);
    channel_config_set_chain_to(&control_conf, samp_chan); // pace data according to ADC

    dma_channel_configure(
            control_chan,                         // Channel to be configured
            &control_conf,                                  // The configuration we just created
            &dma_hw->ch[samp_chan].write_addr,  // Write address (channel 0 read address)
            &sample_address_pointer,              // Read address (POINTER TO AN ADDRESS)
            1,                                    // Number of transfers, in this case each is 4 byte
            false                                 // Don't start immediately.
    );

    dma_start_channel_mask((1u << samp_chan));
    adc_run(true) ;


    while(1)
    {
        dma_channel_wait_for_finish_blocking(samp_chan);
        adc_run(false);
        adc_fifo_drain();

        // Print samples to stdout so you can display them in pyplot, excel, matlab
        for (int i = 0; i < CAPTURE_DEPTH; ++i) {
            printf("%.2f, ;", (capture_buf[i] * ADC_CONVERT));
            if (i % 10 == 9)
                printf("\n");
        }
        dma_channel_start(control_chan);
        adc_run(true);
        sleep_ms(1000);
    }

    return 0;
}
