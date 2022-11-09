/**********************************
 * ADC Controlled LED
 *
 * Example which controls the LED Brightness via PWM which is varied based on an ADC Reading
 *
 *
 * Pin Map:
 *               MSP430FR2355
 *            -----------------
 *        /|\.  |                             |
 *         |.    |                               |
 *         --   |RST                       |
 *               |                               |
 *       >---|P1.1/A1      P6.0|--> LED --> Wired to LED 2 through external wire to P6.6
 *
 *       ****WARNING****
 *       Please disconnect J11 and connect to Pin 6.0 to the lower part of the jumper
 *
 *       ____
 *      | NC | <-- Keep this Disconnected
 *      |----  |
 *      | **    | <-- Connect to this part of J11
 *      |____. |
 *
 *
 * Peripherals Used:
 *      ADC12 - Channel 1 (Pin 1.1)
 *      GPIO - Pin 6.0
 *      TimerB0 - UP Mode, CCR1 for PWM Control, controls Pin 6.0
 */
#include <msp430.h>
/**
 * main.c
 */
void gpioInit();        // Setup the GPIO Pins for controlling the LEDS through PWM (no input arguments)
void timerInit();       // Setup Timer to control the LEDS (no input arguments)
void adcInit();         // Setup ADC to do single, on demand samples (no input arguments)
int readADCSingle();    // Single Read ADC, no input arguments, returns current ADC reading
void setLEDBrightness(int DutyCycle);   // Takes in a duty cycle from 0-100 and converts it to an appropriate CCR value



#include <msp430.h> 

unsigned int ADC_Result;

int main(void)
{
    WDTCTL = WDTPW | WDTHOLD; // stop watchdog timer

    PM5CTL0 &= ~LOCKLPM5;

    gpioInit();
    adcInit();
    timerInit();

    while(1){
        ADC_Result = readADCSingle();
        //TB0CCR1= ADC_data/40; //CCR1 Duty Cycle
        //TB0CCR1 = ADC_Result;
        //setLEDBrightness(TB0CCR1);
        int DutyCycle = (int)ADC_Result/40;
        setLEDBrightness(DutyCycle);
    }

    return 0;
}
/**
 * GPIO INIT
 * Initialize all GPIO Pins. Current Pin configuration:
 * Pin 6.0: OUTPUT, GPIO, TIMER
 * Pin 6.6: OUTPUT, LED
 * Pin 1.1: INPUT,  ADC Channel 1
 */
void gpioInit()
{
    // Configure GPIO Pin
    P6OUT &= ~BIT6;
    P6DIR |= BIT6;  // Sets as output

    // Configure ADC Pin
    P1SEL0 |= BIT1; //This sets P1. to ADC mode Edit: Does This set p1.0 or all?
    P1SEL1 |= BIT1; //So 11 is needed, check Datasheet

    //configure timer channel
    P6SEL0 |= BIT0;
     P6DIR |= BIT0; //Same For This
}
/**
 * Timer INIT
 * Initialize the Timer Peripheral to control a GPIO Pin with PWM in up mode
 * Timer Mode: Up Mode
 */
void timerInit()
{
    // Configure CCR Registers
    TB3CCR0 = 1000;                         // PWM Period
    TB3CCTL1 = OUTMOD_7;                    // CCR1 reset/set, check Datasheet
                                            // For the output wave control
    TB3CCR1 = 300;                           // CCR1 PWM duty cycle CCR1/CCR0
    // Configure Timer Module to Up Mode
    TB3CTL = TBSSEL__SMCLK | MC__UP | TBCLR; //Sets Control to Select
                                             //SMCLK & UP & Clears TBxR
}
/**
 * ADC INIT
 * Initialize the ADC Peripheral to read Single measurements on demand.
 * ADC MODE: Single Sample, Single Conversion
 */
void adcInit()
{
    //Setting ADC Controls
    ADCCTL0 |= ADCSHT_2 | ADCON; // ADC ON, Sample&HoldTime=16 ADC clks
    //ADCSHP = Sample-And-Hold Pulse-Mode Select: Register Within ADCCTL1
    ADCCTL1 |= ADCSHP;           // ADCCLK = MODOSC; sampling timer
    //ADCRES = Result Resolution, 00 = 8bit
    ADCCTL2 &= ~ADCRES;          // clear ADCRES in ADCCTL
    //Yep the 10 which is set to 12bit, which makes it ADC12
    ADCCTL2 |= ADCRES_2;         // 12-bit conversion results
    //Somehow setting A1 Select
    //Conversion Memory Control Register
    ADCMCTL0 |= ADCINCH_1;       //A1 ADC input select; Vref=AVCC
}
/**
 * Read Single ADC Sample from Channel 0 of the ADC12 Peripheral
 * Returns an ADC Sample without interrupts.
 */
int readADCSingle()
{
    ADCCTL0 |= ADCENC | ADCSC;     // Sampling and conversion start
    while (!(ADCIFG & ADCIFG0));   // Wait for sample to be sampled and converted
    return ADCMEM0;
}
/**

 * Set LED Brightness on the LED configured from gpioInit. This modifies the

 * CCR register based on the input.

 * int DutyCycle: Value between 0-100

 */

void setLEDBrightness(int DutyCycle)
{
    TB3CCR1 = (int)DutyCycle/10;
}
