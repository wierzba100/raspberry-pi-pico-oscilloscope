#include <stdio.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/stdio/driver.h"

#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"
#include "hardware/timer.h"

#define ADC0_CAPTURE_CHANNEL 26
#define ADC1_CAPTURE_CHANNEL 27

#define CAPTURE_DEPTH 200000
#define BUFFER_SIZE 1024

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
    uint slice_num_1 = pwm_gpio_to_slice_num(16);
    pwm_set_enabled(slice_num_1, true);

    //100kHz, 50%
    pwm_set_freq_duty(slice_num_1, PWM_CHAN_A, 100000, 50);
    pwm_set_enabled(slice_num_1, true);

    //PWM on gpio 18
    gpio_set_function(18, GPIO_FUNC_PWM);
    uint slice_num_2 = pwm_gpio_to_slice_num(18);
    pwm_set_enabled(slice_num_2, true);

    //50kHz, 50%
    pwm_set_freq_duty(slice_num_2, PWM_CHAN_A, 50000, 50);
    pwm_set_enabled(slice_num_2, true);

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

    dma_channel_configure(samp_chan,    // Channel
                          &samp_conf,   // Configuration
                          capture_buf,    // Destination
                          &adc_hw->fifo,  // Source
                          CAPTURE_DEPTH,  // Transfer count
                          false            // Don't start immediately
    );

    //Channel 2 DMA
    uint control_chan = dma_claim_unused_channel(true);
    dma_channel_config control_conf = dma_channel_get_default_config(control_chan);

    channel_config_set_transfer_data_size(&control_conf, DMA_SIZE_32);
    channel_config_set_read_increment(&control_conf, false); // read from adc FIFO reg.
    channel_config_set_write_increment(&control_conf, false);
    channel_config_set_chain_to(&control_conf, samp_chan); // pace data according to ADC

    dma_channel_configure(
            control_chan,   // Channel
            &control_conf,  // Configuration
            &dma_hw->ch[samp_chan].write_addr,  // Destination
            &sample_address_pointer,    // Read address
            1,  // Number of transfers
            false   // Don't start immediately.
    );

    char buffer[BUFFER_SIZE];

    while(1)
    {
        scanf("%1023s", buffer);
        if(strcmp(buffer, "ON") == 0)
        {
            break;
        }
    }

    dma_start_channel_mask((1u << samp_chan));

    adc_run(true) ;
    dma_channel_wait_for_finish_blocking(samp_chan);
    adc_run(false);
    adc_fifo_drain();
    dma_channel_start(control_chan);

    uint32_t t_start = 0;
    uint32_t t_end = 0;

    t_start = time_us_32();
    stdio_usb.out_chars((const char *)&capture_buf[0], CAPTURE_DEPTH);
    stdio_flush();
    t_end = time_us_32();

    printf("%d\n", t_end - t_start);

    while(1)
    {
        printf("Endless Loop\n");
        sleep_ms(10);
    }

    return 0;
}
