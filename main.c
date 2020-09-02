/**
 * Digital correlator algorithm
 * Developed by Team STI Digital Correlator
 * This program will link up with the GUI to enable the digital correlator
 *
 * Note: you have to go to project->properties->ARM linker->Basic options and change the stack and heap size
 * to 16384, otherwise the correlation won't work
 */
#include <stdbool.h>
#include <stdint.h>
#include <math.h>
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_adc.h"
#include "inc/hw_types.h"
#include "inc/hw_udma.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/interrupt.h"
#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/adc.h"
#include "driverlib/udma.h"
#include "driverlib/timer.h"
#include "driverlib/rom.h"
#include "driverlib/rom_map.h"


#define ADC_BUF_SIZE 1024 //The size of two buffers used for transferring data
#define ARRAYSIZE 4096
#define PULSESNUMBER 16
#define DATAFREQ 1300

uint32_t sampling_rate = 1.0*ARRAYSIZE*DATAFREQ/(PULSESNUMBER);
static uint16_t numTimesInt = 1; //Tracks how many data transfers has occurred
static uint16_t numTransfer = ARRAYSIZE / 1024; //Tracks how many transfers need to be done
#pragma DATA_ALIGN(ucControlTable, 1024) //Aligns ucControlTable on a 1024-byte boundary
uint8_t ucControlTable[1024];


uint16_t data[ARRAYSIZE]; //Array storing all ADC conversions


// Global variables to change the analyzing wave's frequency and the number of samples for the GUI's benefit
bool runXCorr;
double analyzingFreq;
uint8_t samplesNumber;
uint32_t averageValue;
uint8_t buttonCount = 0;
uint8_t analyzingWave[ARRAYSIZE];
uint16_t counter = 0;
uint32_t sum = 0;
uint8_t analyzingWave[ARRAYSIZE];

/*
 * Creates analyzing square wave. This square wave has unity (1) magnitude.
 * The number of high values in each period is determined by high values = (analyzingT/2) / time increment
 */
void createAnalyzingWave(double analyzingFreq, uint8_t wave[]){
    uint16_t highValues = round((DATAFREQ * ARRAYSIZE) / (2 * PULSESNUMBER * analyzingFreq));
    uint16_t counter = 0;
    uint16_t p;

    for(p = 1; p <= ARRAYSIZE; p++){
        if ((counter % 2) == 0){
            wave[p - 1] = 1;
        } else{
            wave[p - 1] = 0;
        }
        if (p % highValues == 0){
            counter++;
        }
    }
}


// Cross-Correlation algorithm. Cross-correlates 2 vectors of equal size with each other.
// Returns maximum cross-correlation value as a double.
// Inputs:
// 2 vectors, must be equal size
// Size of each vector
uint32_t crossCorrelationV4(uint16_t dataWave[], uint8_t analyzingWave[]){

    uint32_t maxCorr = 0;
    uint32_t i, j;
    uint32_t dec = 0;
    uint32_t lastArrayIndex = ARRAYSIZE - 1;
    uint32_t inc = lastArrayIndex;
    uint32_t sum1 = 0;
    uint32_t sum2 = 0;

    for (i = 0; i < ARRAYSIZE; i++){
        for (j = 0; j <= i; j++){
            sum1 = dataWave[j] * analyzingWave[j + ARRAYSIZE - 1 - dec] + sum1;
            sum2 = dataWave[j + inc] * analyzingWave[j] + sum2;
        }
        inc--;
        dec++;
        if (sum1 - sum2 > 0 && sum1 > maxCorr){
            maxCorr = sum1;
        } else if (sum1 - sum2 <= 0 && sum2 > maxCorr){
            maxCorr = sum2;
        }
        sum1 = 0; sum2 = 0;
    }


    return maxCorr;
}

void setupADC(void);
void setupTimer(void);
void setupDMA(void);

/*
 * ADC interrupt handler:
 * NumTransfer will keep track of how many transfer(s) we need to (completely) fill the data array. This number
 * is decremented each time the uDMA sends an interrupt which occurs after the maximum number, 1024, of transfer - ultimately, it will
 * stop the ADC, timer, and their interrupts once all the data has been collected for 1 sample.
 */
void ADCseq0Handler()
{
    if(numTransfer >= 3) //If the number of transfers needed is greater than 3, then re-configuration of the uDMA is needed
    {
        if(numTimesInt % 2 != 0) //If numTimesInt is odd, reconfigure the primary buffer. (numTransfer must be greater than or equal to 3)
        {
            uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT, UDMA_MODE_PINGPONG,
                                   (void*) (ADC0_BASE + ADC_O_SSFIFO0), &data[1024 + (1024*numTimesInt)], ADC_BUF_SIZE);
            uDMAChannelEnable(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT);
            numTimesInt++;
            numTransfer--;
        }
        else //If numTimesInt is even, reconfigure the alternate buffer. (numTransfer must be greater than or equal to 3)
        {
            uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT, UDMA_MODE_PINGPONG,
                                   (void*) (ADC0_BASE + ADC_O_SSFIFO0), &data[1024 + (1024*numTimesInt)], ADC_BUF_SIZE);
            uDMAChannelEnable(UDMA_CHANNEL_ADC0|UDMA_ALT_SELECT);
            numTimesInt++;
            numTransfer--;
        }
    }

    if(numTransfer == 2)
    {
        numTimesInt++;
        numTransfer--;
    }
    else if (numTransfer == 1) //No more transfers are needed. The data array is completely filled
    {
        MAP_SysCtlDelay(40945); //Delay. Gives the uDMA enough time to transfer the last 1024 samples before turning the ADC and Timer off

        TimerDisable(TIMER0_BASE, TIMER_A);
        uDMADisable();    //Have to re-enable by using uDMAEnable();

        numTransfer = ARRAYSIZE / 1024; //Have to reset numTransfer in the case that multiple samples need to be captured
        numTimesInt = 1; //Have to reset numTimesInt in the case that multiple samples need to be captured
    }

}

int main(void)
{
    //Setting the clock to 80 MHz
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN| SYSCTL_XTAL_16MHZ);
    SysCtlDelay(20u); //Provides a small delay. Parameter provides the loop count - each loop is 3 CPU cycles or 3*period of the clock seconds

    /* "MAP_..." keyword: (actual function calls are after "MAP_")
     * For code intended to be shared between projects, and some of the projects
     * run on devices with a ROM and some without a ROM, it is convenient to have the
     * code automatically call the ROM
     */
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0); //Enables the TIMER0 module - used for letting the ADC know when to sample data
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);   //Enables the ADC0 module
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);   //Enables the uDMA controller - used for data transfer from buffer to data array
    MAP_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);  //Enables the PORT E of the LaunchPad to be used
    MAP_SysCtlDelay(30u);

    setupADC();
    setupTimer();
    MAP_IntMasterEnable();

    while(1){
        SysCtlDelay(2000000);
        runXCorr = false;

        if (buttonCount > 0)
        {
            runXCorr = true;
            createAnalyzingWave(analyzingFreq, analyzingWave);
            sum = 0;
            uint8_t i;
            for (i = 0; i < samplesNumber; i++){
                setupDMA();
                MAP_TimerEnable(TIMER0_BASE, TIMER_A); //Start everything - Timer starts which leads to data capture and eventually stopping the Timer

                sum = crossCorrelationV4(data, analyzingWave) + sum;
            }
            averageValue = sum / samplesNumber;

            buttonCount = 0;
        }
    }
}

/*
 *  This function sets up the uDMA (Micro Direct Memory Access) controller. The uDMA API provides
 *  functions to configure the Tiva uDMA controller. The uDMA controller is designed to work
 *  with the ARM Cortex-M processor and provides an efficient and low-overhead means of
 *  transferring blocks of data in the system.
 */
void setupDMA()
{
    //Must first enable the uDMA controller so that it can be configured and used
    uDMAEnable();

    /*
     *  Sets the base address for the channel control table. The parameter
     * is a pointer to the 1024-byte-aligned base address of the uDMA
     * channel control table (defined above main).
     *  This function table resides in system memory and holds control information
     *for each uDMA channel. The table must be aligned on a 1024-byte
     *boundary. The base address must be configured before any of the channel functions
     *can be used.
     *  The size of the channel control table depends on the number of uDMA
     *channels and the transfer modes that are used.
     */
    uDMAControlBaseSet(ucControlTable); //ucControlTable is defined under the header files

    /*
     *  Disable/Enable attributes of the specific uDMA channel selected. The first
     * parameter is the specific channel, the second is the attribute(s) you want to
     * disable/enable (if multiple, OR them)
     */
    uDMAChannelAttributeDisable(
            UDMA_CHANNEL_ADC0,
            UDMA_ATTR_ALTSELECT | UDMA_ATTR_HIGH_PRIORITY | UDMA_ATTR_REQMASK);
    uDMAChannelAttributeEnable(UDMA_CHANNEL_ADC0, UDMA_ATTR_USEBURST);

    /*
     *  Used to set control parameters for a uDMA transfer. These parameters are typically
     * not changed often. 2 parameters:
     *  1.) logical OR of channel number with UDMA_PRI_SELECT or UDMA_ALT_SELECT to choose
     * between the primary or alternate data structure is used.
     *  2.) logical OR of 5 values - data size, source address increment, destination
     * address increment, arbitration size, and the use burst flag. Possible values are
     * listed on page 591 of the TivaWare Peripheral User's Guide
     */
    uDMAChannelControlSet(
            UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
            UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);
    uDMAChannelControlSet(
            UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT,
            UDMA_SIZE_16 | UDMA_SRC_INC_NONE | UDMA_DST_INC_16 | UDMA_ARB_1);

    /*
     *  Used to configure the parameters for a uDMA transfer. uDMAChannelControlSet()
     * MUST be called at least once for this channel prior to calling this function. 5 Params:
     *  1.)Logical OR of the channel number and structure (Pri or alt)
     *  2.)Type of uDMA transfer
     *  3.)source address for the transfer
     *  4.)destination address for the transfer
     *  5.)number of data items to transfer
     */
    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_PRI_SELECT,
                           UDMA_MODE_PINGPONG,
                           (void*) (ADC0_BASE + ADC_O_SSFIFO0), &data[0],
                           ADC_BUF_SIZE);
    uDMAChannelTransferSet(UDMA_CHANNEL_ADC0 | UDMA_ALT_SELECT,
                           UDMA_MODE_PINGPONG,
                           (void*) (ADC0_BASE + ADC_O_SSFIFO0), &data[1024],
                           ADC_BUF_SIZE);

    //Enables a channel to perform data transfers
    uDMAChannelEnable(UDMA_CHANNEL_ADC0);
}

void setupADC()
{
    /*
     * GPIOPinTypeADC configures pin(s) to use as Analog-to-Digital Converter (ADC)
     * inputs. The first parameter is the base address of the GPIO port and the second
     * is the bit-packed representation of the pin(s). We've arbitrarily selected
     * Port E, Pin 2 as the ADC pin (PE2 on launchPad)
     */
    GPIOPinTypeADC(GPIO_PORTE_BASE, GPIO_PIN_2);
    SysCtlDelay(80u); // delay

    /* Found in Chapter 17 - Interrupt Controller (NVIC) p. 345
     *  This call disables an interrupt source - in this case "INT_ADC0SS0." The specified
     *  interrupt/param must be one of the valid INT_* values listed in the
     *  Driver Library User's Guide and defined in the inc/hw_ints.h header file.
     *  Other enables for the interrupt (such as at the peripheral level) are
     *  unaffected by this.
     */
    IntDisable(INT_ADC0SS0);

    /*
     * Disables a sample sequence. Parameters are the base address of the ADC module
     * and the sample sequence number. This prevents the specified sequence from
     * being captured when its trigger is detected. A sample sequence must be disabled before it
     * is configured.
     */
    ADCSequenceDisable(ADC0_BASE, 0u);

    // With sequence disabled, it is now safe to load the new configuration parameters

    /*
     *  Configures the trigger source and priority of a sample sequence. The third
     * and fourth parameter is the trigger source that initiates the sample sequence
     * and priority respectively.
     *  For this example, the trigger source is a timer. The timer is enabled
     * and initiated elsewhere.
     *  The priority is 0 (highest)
     */
    ADCSequenceConfigure(ADC0_BASE, 0u, ADC_TRIGGER_TIMER, 0u);

    /*
     *  This function configures the ADC for one step of a sample sequence. The parameters
     *are: base address of the ADC module, sample sequence number, step to be configured,
     *and an OR combination.
     *  The first-two are self-explanatory. The third parameter determines the order
     *in which samples are captured by the ADC when the trigger occurs. It ranges from
     *0 to number of sample sequencer-1 of the module. E.g., sample sequencer 0
     *ranges from 0 to 7 (it has 8 sample sequencer) and sample sequencer 0 was
     *chosen by the function call. The third parameter "0" suggests that it is the highest
     *chosen priority amongst the samples so do it first.
     *  The last parameter is a logical OR of many possibilities.
     */
    ADCSequenceStepConfigure(ADC0_BASE, 0u, 0u,
                             ADC_CTL_CH1 | ADC_CTL_END | ADC_CTL_IE);

    ADCSequenceEnable(ADC0_BASE, 0u);

    ADCIntClear(ADC0_BASE, 0u); //Clears the interrupts
    ADCSequenceDMAEnable(ADC0_BASE, 0);
    IntEnable(INT_ADC0SS0);
}

/*
 *  Setting up the timer peripheral. Because we are sampling at a particular frequency (sample rate),
 *  a timer is used to trigger the ADC to capture data. Once the timer is enabled, it will periodically
 *  trigger the ADC to collect samples.
 */
void setupTimer()
{
    /*
     *  This function configures the timer(s). The first parameter specifies the
     * base address of the (arbitrary) timer module meanwhile the second is the
     * configuration for the timer.
     *  We are using a (16-bit) half-width timer. Specified by "TIMER_CFG_SPLIT_PAIR."
     * In addition Timer A (Timer B is not used) is used and is a half-width periodic timer
     * specified by TIMER_CFG_A_PERIODIC.
     */
    MAP_TimerConfigure(TIMER0_BASE, TIMER_CFG_SPLIT_PAIR | TIMER_CFG_A_PERIODIC); //Configuring (16-bit) half-width timer Module 0, Timer A, that will trigger periodically

    /*  Setting the timer so it triggers the ADC to fulfill a certain sampling rate:
     *  First param is the base address of the timer module, second specifies the
     * timer(s) ; must be either TIMER_A or TIMER_B or TIMER_BOTH (if
     * the timer is configured for full-width only use TIMER_A), and the last one
     * loads the value. The loaded value, 3rd parameter, is the number of clock cycles
     * that the MCU will count down from or up to from 0 - every time it finishes
     * counting up to or down from that value, the timer triggers the ADC to collect
     * a sample.
     *  The frequency of the system clock is the value returned by SysCtlClockGet()
     */
    MAP_TimerLoadSet(TIMER0_BASE, TIMER_A, MAP_SysCtlClockGet() / sampling_rate - 1);

    /*
     *  Enables or disables trigger output. The first parameter specifies the
     * base address of the timer module, the second specifies which half-width timer we are using,
     * and the last parameter specifies whether the timer is
     * enabled, if value is "true", or disabled, if "false."
     */
    MAP_TimerControlTrigger(TIMER0_BASE, TIMER_A, true);

    /* Assist in debug by stalling timer at breakpoints
     *      This function controls the stall handling. If the last parameter is
     * "true" then the timer stops counting if the processor enters debug mode;
     * otherwise the timer keeps running while in debug mode.
     */
    MAP_TimerControlStall(TIMER0_BASE, TIMER_A, true);
}
