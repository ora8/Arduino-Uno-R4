//#define LED_PIN BSP_IO_PORT_01_PIN_02  // Internal LED
#define LED_PIN BSP_IO_PORT_04_PIN_11    // Output D11

const ioport_pin_cfg_t IOPORT_PIN_CRG_DATA[] =
        {
        { ((uint32_t) IOPORT_CFG_DRIVE_MID
                | (uint32_t) IOPORT_CFG_PORT_DIRECTION_OUTPUT | (uint32_t) IOPORT_CFG_PORT_OUTPUT_LOW),
                LED_PIN },          
 };


const ioport_cfg_t IOPORT_CFG =
{ sizeof(IOPORT_PIN_CRG_DATA) / sizeof(ioport_pin_cfg_t), IOPORT_PIN_CRG_DATA };


void setup() {
    // Initialize the I/O port
    R_IOPORT_Open(&g_ioport_ctrl, &IOPORT_CFG);
}

void loop() {
    R_IOPORT_PinWrite(&g_ioport_ctrl, LED_PIN, BSP_IO_LEVEL_HIGH);
    // Wait for 250 milliseconds
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
    
    R_IOPORT_PinWrite(&g_ioport_ctrl, LED_PIN, BSP_IO_LEVEL_LOW);
    // Wait for 250 milliseconds
    R_BSP_SoftwareDelay(1000, BSP_DELAY_UNITS_MILLISECONDS);
}