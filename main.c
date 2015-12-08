#define TEMP_ADDR  0x4F		// Address for Temp Sensor

// Define needed for pin_map.h
#define PART_TM4C123GH6PM

#include <stdint.h>
#include <stdbool.h>

#include "LaunchPad.h"
#include "OrbitBoosterPackDefs.h"

#include "OrbitOled.h"
#include "OrbitOledGrph.h"
#include "OrbitOledChar.h"
#include "FillPat.h"

#include "delay.h"

#include "inc/hw_i2c.h"
#include "driverlib/i2c.h"

void DeviceInit();
void OrbitSetOled();

void OrbitDemo2();
void Read_temp(unsigned char*, char);	// Read Temperature sensor
float read_float_temp();		// Read Temperature sensor
char* ftos(float, char);		// convert float to string (char*)
void init_i2c();

int main() {

	DeviceInit();

 	init_i2c(); // Initiate i2c

	while(1) {
		OrbitDemo2();
	}
}

/*
/***	DeviceInit
*/
void DeviceInit(void) {

	/*
	 * First, Set Up the Clock.
	 * Main OSC				-> SYSCTL_OSC_MAIN
	 * Runs off 16MHz clock -> SYSCTL_XTAL_16MHZ
	 * Use PLL				-> SYSCTL_USE_PLL
	 * Divide by 4			-> SYSCTL_SYSDIV_4
	 */
	SysCtlClockSet(SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ | SYSCTL_USE_PLL | SYSCTL_SYSDIV_4);

	/*
	 * Enable and Power On All GPIO Ports
	 */
	SysCtlPeripheralEnable(	SYSCTL_PERIPH_GPIOA);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOC);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOD);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOE);
	SysCtlPeripheralEnable( SYSCTL_PERIPH_GPIOF);


	/*
	 * Pad Configure.. Setting as per the Button Pullups on
	 * the Launch pad (active low).. changing to pulldowns for Orbit
	 */
	GPIOPadConfigSet(SWTPort, SWT1 | SWT2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
	GPIOPadConfigSet(BTN1Port, BTN1, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
	GPIOPadConfigSet(BTN2Port, BTN2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD_WPD);
	GPIOPadConfigSet(LED1Port, LED1, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
	GPIOPadConfigSet(LED2Port, LED2, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
	GPIOPadConfigSet(LED3Port, LED3, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);
	GPIOPadConfigSet(LED4Port, LED4, GPIO_STRENGTH_8MA_SC, GPIO_PIN_TYPE_STD);

	/*
	 * Initialize Switches as Input
	 */
	GPIOPinTypeGPIOInput(SWTPort, SWT1 | SWT2);

	/*
	 * Initialize Buttons as Input
	 */
	GPIOPinTypeGPIOInput(BTN1Port, BTN1);
	GPIOPinTypeGPIOInput(BTN2Port, BTN2);

	/*
	 * Initialize LEDs as Output
	 */
	GPIOPinTypeGPIOOutput(LED1Port, LED1);
	GPIOPinTypeGPIOOutput(LED2Port, LED2);
	GPIOPinTypeGPIOOutput(LED3Port, LED3);
	GPIOPinTypeGPIOOutput(LED4Port, LED4);

	/*
	 * Enable ADC Periph
	 */
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);

	GPIOPinTypeADC(AINPort, AIN);

	/*
	 * Enable ADC with this Sequence
	 * 1. ADCSequenceConfigure()
	 * 2. ADCSequenceStepConfigure()
	 * 3. ADCSequenceEnable()
	 * 4. ADCProcessorTrigger();
	 * 5. Wait for sample sequence ADCIntStatus();
	 * 6. Read From ADC
	 */
	ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH0);
	ADCSequenceEnable(ADC0_BASE, 0);

	/*
	 * Initialize the OLED
	 */
	OrbitOledInit();

	OrbitSetOled();

}

/*
/***	OrbitSetOled
 * Set message on on OLED
*/
void OrbitSetOled() {
	char *name = "Martin Jaime";
	char *label = "CpE403:Lab11-T02";
	char *temp_label = "Temp:";

	OrbitOledSetCursor(0, 0);
	OrbitOledPutString(name);

	OrbitOledSetCursor(0, 1);
	OrbitOledPutString(label);

	OrbitOledMoveTo(0,19);
	OrbitOledLineTo(127, 19);

	OrbitOledSetCursor(0, 4);
	OrbitOledPutString(temp_label);
}

/* ------------------------------------------------------------ */
/***	OrbitDemo
*/
void OrbitDemo2() {
	float 	temp;
	char	temp_str[5];

	/*
	 * Read temperature and display.
	 */

	Read_temp(temp_str, 'C');

	OrbitOledSetCursor(8, 4);
	OrbitOledPutString(temp_str);

	SysCtlDelay(20000000); // Delay

	Read_temp(temp_str, 'F');

	OrbitOledSetCursor(8, 4);
	OrbitOledPutString(temp_str);

	SysCtlDelay(20000000); // Delay

}

void Read_temp(unsigned char *data, char t){	// Read Temperature sensor
	unsigned char temp[2];				//  storage for data
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);	// Start condition
	SysCtlDelay(20000);													// Delay
	temp[0] = I2CMasterDataGet(I2C0_BASE);								// Read first char
	SysCtlDelay(20000);													// Delay
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);		// Push second Char
	SysCtlDelay(20000);													// Delay
	temp[1] = I2CMasterDataGet(I2C0_BASE);								// Read second char
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);	// Stop Condition

	if(t == 'F')
		temp[0] = (unsigned char)(temp[0]*(9.0/5) + 32);

	data[0] = (temp[0] / 10) + 0x30;									// convert 10 place to ASCII
	data[1] = (temp[0] - ((temp[0] / 10)*10)) + 0x30;					// Convert 1's place to ASCII
	data[2] = t;
	data[3] = '\0';
}

float read_float_temp(){	// Read Temperature sensor
	unsigned char temp[2];	//  storage for data
	float value;

	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_START);	// Start condition
	SysCtlDelay(20000);													// Delay
	temp[0] = I2CMasterDataGet(I2C0_BASE);								// Read first char
	SysCtlDelay(20000);													// Delay
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_CONT);		// Push second Char
	SysCtlDelay(20000);													// Delay
	temp[1] = I2CMasterDataGet(I2C0_BASE);								// Read second char
	I2CMasterControl(I2C0_BASE, I2C_MASTER_CMD_BURST_RECEIVE_FINISH);	// Stop Condition

	value = temp[0];

	if (temp[1] != 128)
		value += 0.5;
	return value;
}

void init_i2c()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);		// Enable I2C hardware

	GPIOPinConfigure(GPIO_PB3_I2C0SDA);				// Configure GPIO pin for I2C Data line
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);				// Configure GPIO Pin for I2C clock line

	GPIOPinTypeI2C(GPIO_PORTB_BASE, GPIO_PIN_2 | GPIO_PIN_3); 	// Set Pin Type

	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_2, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_STD);// SDA MUST BE STD
	GPIOPadConfigSet(GPIO_PORTB_BASE, GPIO_PIN_3, GPIO_STRENGTH_2MA, GPIO_PIN_TYPE_OD);	// SCL MUST BE OPEN DRAIN
	I2CMasterInitExpClk(I2C0_BASE, SysCtlClockGet(), false); 	// The False sets the controller to 100kHz communication
	I2CMasterSlaveAddrSet(I2C0_BASE, TEMP_ADDR, true);  		// false means transmit
}

char* ftos(float fVal, char t)
// convert float to char*. t must be 'F' or 'C'
{
    char result[10];
    int dVal, dec, i;

    if (t == 'F') // if type is Farenheit, convert.
    	fVal = fVal*( 9 / 5 ) + 32;

    fVal += 0.005; // round to nearest hundedth.

    dVal = fVal;
    dec = (int)(fVal * 100) % 100;

    result[0] = (dec % 10) + '0';
    result[1] = (dec / 10) + '0';
    result[2] = '.';

    while (dVal > 0)
    for(i = 3; i <= 4; i++)
    {
        result[i] = (dVal % 10) + '0';
        dVal /= 10;
    }
    result[6] = t;
    result[7] = '\0';

    return result;
}
