#include <stdio.h>
#include <stdlib.h>

#include "pico/stdlib.h"
#include "pico/stdio.h"
#include "pico/stdio/driver.h"

#include "hardware/adc.h"
#include "hardware/pwm.h"

#define ADC0_CAPTURE_CHANNEL 26
#define ADC1_CAPTURE_CHANNEL 27

#define PWM0_GPIO 16
#define PWM1_GPIO 18

#define CAPTURE_DEPTH 100
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

void decodeTriggerMessage(const char *input, uint8_t *triggerChannel, uint8_t *triggerValue, uint8_t *samplingMode)
{
    *triggerChannel = input[0];
    *samplingMode = input[1];
    *triggerValue = input[2];
}

inline static uint8_t myAdc_read(void)
{
    while (!(ADC_CS_READY_BITS & adc_hw->cs));
    hw_set_bits(&adc_hw->cs, ADC_CS_START_ONCE_BITS);
    return (uint8_t) ((adc_hw->result) >> 4);
}

bool newInput;

void callback_func(void *param)
{
    newInput = true;
}

int main() {
    stdio_init_all();

    uint8_t *sample_address_pointer = calloc(CAPTURE_DEPTH, sizeof(uint8_t));
    char *received_string = calloc(BUFFER_SIZE, sizeof(char));

    //PWM on gpio 16
    gpio_set_function(PWM0_GPIO, GPIO_FUNC_PWM);
    uint slice_num_1 = pwm_gpio_to_slice_num(PWM0_GPIO);
    pwm_set_enabled(slice_num_1, true);

    //10kHz, 50%
    pwm_set_freq_duty(slice_num_1, PWM_CHAN_A, 10000, 50);
    pwm_set_enabled(slice_num_1, true);

    //PWM on gpio 18
    gpio_set_function(PWM1_GPIO, GPIO_FUNC_PWM);
    uint slice_num_2 = pwm_gpio_to_slice_num(PWM1_GPIO);
    pwm_set_enabled(slice_num_2, true);

    //5kHz, 50%
    pwm_set_freq_duty(slice_num_2, PWM_CHAN_A, 5000, 50);
    pwm_set_enabled(slice_num_2, true);

    adc_gpio_init(ADC0_CAPTURE_CHANNEL);
    adc_gpio_init(ADC1_CAPTURE_CHANNEL);

    adc_init();

    adc_set_clkdiv(0); // Run at max speed

    uint8_t triggerChannel, samplingMode, triggerValue, new_value, old_value;

    stdio_usb.set_chars_available_callback(callback_func, NULL);

    while(1)
    {
        fgets(received_string, BUFFER_SIZE, stdin); //waiting for input

        decodeTriggerMessage(received_string, &triggerChannel, &triggerValue, &samplingMode); //Decode message with trigger and sampling setup

        adc_select_input(triggerChannel); //set input for trigger
        adc_set_round_robin(samplingMode); //set sampling mode

        newInput = false;

        hw_set_bits(&adc_hw->cs, ADC_CS_START_ONCE_BITS); //start conversion
        old_value = myAdc_read(); //read old value for checking trigger

        (void) myAdc_read(); //read value from second channel and do nothing with that, due to robin mode sampling
        while(!newInput)
        {
            new_value = myAdc_read();
            if ( ( new_value >= triggerValue) && ( old_value < triggerValue) )
            {
                sample_address_pointer[triggerChannel] = new_value; //set bits in correct order, CH1 first, CH2 second
                sample_address_pointer[1-triggerChannel] = myAdc_read();

                if((samplingMode == 3) && (triggerChannel == 1))
                {
                    (void) myAdc_read();
                }

                //trigger triggered
                /*if(samplingMode != 3)
                {
                    sample_address_pointer[0] = new_value; //set bits in correct order, CH1 first, CH2 second
                    sample_address_pointer[1] = myAdc_read();
                }else
                {
                    sample_address_pointer[triggerChannel] = new_value; //set bits in correct order, CH1 first, CH2 second
                    sample_address_pointer[1-triggerChannel] = myAdc_read();
                    if(triggerChannel == 0)
                    {
                        adc_select_input(0); //starting acquisitions from correct channel according to sampling mode
                    }
                }*/

                for(int i=2;i<CAPTURE_DEPTH;i++)
                {
                    sample_address_pointer[i] = myAdc_read(); //reading values from adc
                }

                stdio_usb.out_chars((const char *)&sample_address_pointer[0], CAPTURE_DEPTH ); //sending bytes
                break;
            }
            (void) myAdc_read(); //read value from second channel and do nothing with that, due to robin mode sampling
            old_value = new_value;
        }

        stdio_flush(); //flush the buffer
    }

    free(sample_address_pointer);
    free(received_string);

    return 0;
}