#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/stdio/driver.h"

#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/pwm.h"

#define ADC0_CAPTURE_CHANNEL 26
#define ADC1_CAPTURE_CHANNEL 27

#define CAPTURE_DEPTH 1000
#define BUFFER_SIZE 128

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

void decodeTriggerMessage(const char *input, char *triggerChannel, uint16_t *triggerValue)
{
    *triggerChannel = input[0];
    *triggerValue = (input[3] << 8) | input[2];
}


int main() {
    stdio_init_all();

    uint8_t * sample_address_pointer = calloc(CAPTURE_DEPTH, sizeof(char));

    //PWM on gpio 16
    gpio_set_function(16, GPIO_FUNC_PWM);
    uint slice_num_1 = pwm_gpio_to_slice_num(16);
    pwm_set_enabled(slice_num_1, true);

    //100kHz, 50%
    pwm_set_freq_duty(slice_num_1, PWM_CHAN_A, 10000, 50);
    pwm_set_enabled(slice_num_1, true);

    //PWM on gpio 18
    gpio_set_function(18, GPIO_FUNC_PWM);
    uint slice_num_2 = pwm_gpio_to_slice_num(18);
    pwm_set_enabled(slice_num_2, true);

    //50kHz, 50%
    pwm_set_freq_duty(slice_num_2, PWM_CHAN_A, 5000, 50);
    pwm_set_enabled(slice_num_2, true);

    adc_gpio_init(ADC0_CAPTURE_CHANNEL);
    adc_gpio_init(ADC1_CAPTURE_CHANNEL);

    adc_init();

    adc_set_clkdiv(0); // Run at max speed

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
                          sample_address_pointer,    // Destination
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

    char *buffer = calloc(BUFFER_SIZE, sizeof(char));

    dma_start_channel_mask((1u << samp_chan));

    uint16_t new_value, old_value, triggerValue;
    char triggerChannel;

    while(1)
    {
        fgets(buffer, BUFFER_SIZE, stdin); //waiting for input

        decodeTriggerMessage(buffer, &triggerChannel, &triggerValue); //Decode message with trigger setup

        adc_set_round_robin(0x0); // Disable round-robin
        adc_select_input(triggerChannel); // Set starting ADC channel for round-robin mode.

        while (!adc_fifo_is_empty())
        {
            (void) adc_fifo_get();
        }

        old_value = adc_read();
        (void) adc_fifo_get();
        while (1)
        {
            new_value = adc_read();
            (void) adc_fifo_get();
            if ( ( new_value >= triggerValue) && ( old_value < triggerValue) )
            {
                break;
            }
            old_value = new_value;
        }

        dma_channel_start(control_chan); //restart the sample channel

        adc_select_input(0);
        adc_set_round_robin(0x3); // Enable round-robin sampling of 2 inputs.

        adc_run(true); //starting adc conversion
        dma_channel_wait_for_finish_blocking(samp_chan); //wait for DMA to finish transfer
        adc_run(false); //stopping adc conversion

        stdio_usb.out_chars((const char *)&sample_address_pointer[0], CAPTURE_DEPTH); //sending bytes
        stdio_flush(); //flush the buffer
    }

    free(sample_address_pointer);
    free(buffer);

    return 0;
}
