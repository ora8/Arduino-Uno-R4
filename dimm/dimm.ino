extern "C" {
#include "r_gpt.h"
#include "r_ioport.h"
#include "r_ioport_api.h"
#include "r_timer_api.h"
#include "hal_data.h"
}

#define GPT_MAX_PERCENT 100

// #define OUTPUT_PIN BSP_IO_PORT_04_PIN_11  // Pin 411
#define OUTPUT_PIN BSP_IO_PORT_01_PIN_03   // Pin 103

/*  chatGPT
| Channel | Timer Instance | Pin Options (GTIOCA/GTIOCB)     |
| ------- | -------------- | ------------------------------- |
| GPT0    | GPT channel 0  | Often on `P4_11`, `P0_01`, etc. |
| GPT1    | GPT channel 1  | Often on `P1_03`, `P2_04`, etc. |
| GPT2    | GPT channel 2  | May or may not be available     |
| GPT3+   | Depends on MCU | Often reserved or internal only |
*/

//#define GPT_CHANNEL 6     // Pin 411
#define GPT_CHANNEL 2       // Pin 103

// @ renesas
/*****************************************************************************************************************
 *  @brief       set  duty cycle of PWM timer.
 *  @param[in]   duty_cycle_percent.
 *  @retval      FSP_SUCCESS on correct duty cycle set.
 *  @retval      FSP_INVALID_ARGUMENT on invalid info.
 ****************************************************************************************************************/
fsp_err_t set_timer_duty_cycle(timer_ctrl_t *const p_timer_ctrl, uint8_t duty_cycle_percent)
{
    fsp_err_t err                           = FSP_SUCCESS;
    uint32_t duty_cycle_counts              = 0U;
    uint32_t current_period_counts          = 0U;
    timer_info_t info                       = {TIMER_DIRECTION_DOWN, 0U, 0U};

    /* Get the current period setting. */
    err = R_GPT_InfoGet(p_timer_ctrl, &info);
    if (FSP_SUCCESS != err)
    {
        /* GPT Timer InfoGet Failure message */
        Serial.print ("\r\n ** R_GPT_InfoGet API failed ** \r\n");
    }
    else
    {
        /* update period counts locally. */
        current_period_counts = info.period_counts;

        /* Calculate the desired duty cycle based on the current period. Note that if the period could be larger than
         * UINT32_MAX / 100, this calculation could overflow. A cast to uint64_t is used to prevent this. The cast is
         * not required for 16-bit timers. */
        duty_cycle_counts =(uint32_t) ((uint64_t) (current_period_counts * duty_cycle_percent) /
                GPT_MAX_PERCENT);

        //Serial.println(duty_cycle_counts);

        /* Duty Cycle Set API set the desired intensity on the on-board LED */
        err = R_GPT_DutyCycleSet(p_timer_ctrl, duty_cycle_counts, GPT_IO_PIN_GTIOCA);
        if(FSP_SUCCESS != err)
        {
            /* GPT Timer DutyCycleSet Failure message */
            /* In case of GPT_open is successful and DutyCycleSet fails, requires a immediate cleanup.
             * Since, cleanup for GPT open is done in timer_duty_cycle_set,Hence cleanup is not required */
            Serial.print ("\r\n ** R_GPT_DutyCycleSet API failed ** \r\n");
        }
    }
    return err;
}

const ioport_pin_cfg_t IOPORT_PIN_CRG_DATA[] =
{
    { ( (uint32_t) IOPORT_CFG_PORT_DIRECTION_OUTPUT | 
        (uint32_t) IOPORT_CFG_PERIPHERAL_PIN), // Set as peripheral pin for PWM
      OUTPUT_PIN },
};

const ioport_cfg_t IOPORT_CFG =
{ sizeof(IOPORT_PIN_CRG_DATA) / sizeof(ioport_pin_cfg_t), IOPORT_PIN_CRG_DATA };

gpt_extended_cfg_t timer_extended_cfg = {
    {
        true,
        GPT_PIN_LEVEL_LOW
    }, 
    {
        false,
        GPT_PIN_LEVEL_LOW
    },
    (gpt_source_t) GPT_SOURCE_NONE,  // Start source
    (gpt_source_t) GPT_SOURCE_NONE,  // Stop source
    (gpt_source_t) GPT_SOURCE_NONE,  // Clear source
    (gpt_source_t) GPT_SOURCE_NONE,  // Capture A source
    (gpt_source_t) GPT_SOURCE_NONE,  // Capture B source
    (gpt_source_t) GPT_SOURCE_NONE,  // Count up source
    (gpt_source_t) GPT_SOURCE_NONE,  // Count down source
    GPT_CAPTURE_FILTER_NONE, // Capture filter for GTIOCA
    GPT_CAPTURE_FILTER_NONE, // Capture filter for GTIOCB
    (BSP_IRQ_DISABLED), // Capture A interrupt priority
    (BSP_IRQ_DISABLED), // Capture B interrupt priority
    FSP_INVALID_VECTOR, // Capture A interrupt (make sure this is of type IRQn_Type)
    FSP_INVALID_VECTOR, // Capture B interrupt (make sure this is of type IRQn_Type)
    nullptr,           // Pointer to PWM configuration
    0U,                // Custom GTIOR settings
};


timer_cfg_t timer_cfg = {
  TIMER_MODE_PWM,         // mode
  10000,                  // period_counts
  TIMER_SOURCE_DIV_1,     // source_div
  0,                      // duty_cycle_counts
  GPT_CHANNEL,            // channel (GPT channel number)
  (BSP_IRQ_DISABLED),     // cycle_end_ipl
  FSP_INVALID_VECTOR,     // cycle_end_irq
  nullptr,                // p_callback
  nullptr,                // p_context
  &timer_extended_cfg     // p_extend
};

gpt_instance_ctrl_t g_timer_ctrl; // This is typically the control structure for the GPT

void setup() {
  Serial.begin(9600);

 fsp_err_t err;

// Open IOPORT
err = R_IOPORT_Open(&g_ioport_ctrl, &IOPORT_CFG);
if (err != FSP_SUCCESS) Serial.println("IOPORT open failed");

// Configure pin
err = R_IOPORT_PinCfg(&g_ioport_ctrl, OUTPUT_PIN, (IOPORT_CFG_PERIPHERAL_PIN | IOPORT_PERIPHERAL_GPT1));
if (err != FSP_SUCCESS) Serial.println("PinCfg failed");

// Open GPT
err = R_GPT_Open(&g_timer_ctrl, &timer_cfg);
if (err != FSP_SUCCESS) Serial.println("GPT open failed");

// Enable output
//err = R_GPT_OutputEnable(&g_timer_ctrl, GPT_IO_PIN_GTIOCA);
//if (err != FSP_SUCCESS) Serial.println("GPT output enable failed");

// Enable
err = R_GPT_Enable(&g_timer_ctrl);
if (err != FSP_SUCCESS) Serial.println("GPT enable failed");

// Start GPT
err = R_GPT_Start(&g_timer_ctrl);
if (err != FSP_SUCCESS) Serial.println("GPT start failed");

  Serial.println("PWM started on D11 (P4_11)");
}

  void loop() {
  // Example: Change duty cycle from 0% to 100%

  for (int duty_cycle = 0; duty_cycle <= 100; ++duty_cycle) {
    set_timer_duty_cycle(&g_timer_ctrl, duty_cycle);  // Ensure this is the correct pin
    R_BSP_SoftwareDelay(20, BSP_DELAY_UNITS_MILLISECONDS);
  }
  // Change duty cycle back from 100% to 0%
  for (int duty_cycle = 100; duty_cycle >= 0; duty_cycle--) {
    set_timer_duty_cycle(&g_timer_ctrl, duty_cycle);  // Ensure this is the correct pin
    R_BSP_SoftwareDelay(20, BSP_DELAY_UNITS_MILLISECONDS);
  }
}