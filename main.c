#include <stdio.h>
#include "pico/stdlib.h"

#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "hardware/pwm.h"

#define ADC0_CAPTURE_CHANNEL 26
#define ADC1_CAPTURE_CHANNEL 27

#define ADC_VREF 3.3
#define ADC_RANGE (1 << 8)
#define ADC_CONVERT (ADC_VREF / (ADC_RANGE - 1))

#define CAPTURE_DEPTH 100

uint8_t capture_buf[CAPTURE_DEPTH];
uint8_t * sample_address_pointer = &capture_buf[0];

uint32_t pwm_set_freq_duty(uint slice_num, uint chan,uint32_t freq, int duty)
{
    uint32_t clock = 125000000;
    uint32_t divider16 = clock / freq / 4096 + (clock % (freq * 4096) != 0);
    if (divider16 / 16 == 0)
    {
        divider16 = 16;
    }
    uint32_t wrap = clock * 16 / divider16 / freq - 1;
    pwm_set_clkdiv_int_frac(slice_num, divider16/16,divider16 & 0xF);
    pwm_set_wrap(slice_num, wrap);
    pwm_set_chan_level(slice_num, chan, wrap * duty / 100);

    return wrap;
}

int main() {
    stdio_init_all();

    //PWM on gpio 16
    gpio_set_function(16, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(16);
    pwm_set_enabled(slice_num, true);

    //wypelnienie 50%, 50Hz
    pwm_set_freq_duty(slice_num, PWM_CHAN_A, 50, 50);
    pwm_set_enabled(slice_num, true);

    adc_gpio_init(ADC0_CAPTURE_CHANNEL);
    adc_gpio_init(ADC1_CAPTURE_CHANNEL);

    adc_init();

    adc_set_round_robin(0x3); // Enable round-robin sampling of 2 inputs.
    adc_select_input(0); // Set starting ADC channel for round-robin mode.

    adc_set_clkdiv(0); // Run at max speed.

    adc_fifo_setup(
            true,    // Write each completed conversion to the sample FIFO
            true,    // Enable DMA data request (DREQ)
            1,       // Assert DREQ (and IRQ) at least 1 sample present
            false,   // Omit ERR bit (bit 15) since we have 8 bit reads.
            true     // shift each sample to 8 bits when pushing to FIFO
    );

    //Channel 1 DMA
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

    //Channel 2 DMA
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

    while(1)
    {
        adc_run(true) ;
        dma_channel_wait_for_finish_blocking(samp_chan);
        adc_run(false);
        adc_fifo_drain();

        // Print samples to stdout so you can display them in pyplot, excel, matlab
        for (int i = 0; i < CAPTURE_DEPTH; i=i+2) {
            printf("%.2f\n", (capture_buf[i] * ADC_CONVERT));
            //if (i == CAPTURE_DEPTH - 1)
                //printf("KOniec\r");
        }
        dma_channel_start(control_chan);
    }

    return 0;
}
