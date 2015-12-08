#include <stdint.h>
#include <stdbool.h>
#include "LaunchPad.h"
#include "OrbitBoosterPackDefs.h"

#include "OrbitOled.h"
#include "OrbitOledGrph.h"
#include "OrbitOledChar.h"
#include "FillPat.h"

#include "delay.h"

void DeviceInit();
void OrbitSetOled();
void OrbitDemo();
void Read_temp(unsigned char*);	// Read Temperature sensor

/* ------------------------------------------------------------ */
/***	main()
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Main program loop
*/
int main() {

	DeviceInit();

	while(1) {

		OrbitDemo();

	}

	//return 0;
}

/* ------------------------------------------------------------ */
/***	DeviceInit
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Initialize I2C Communication, and GPIO
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

/* ------------------------------------------------------------ */
/***	OrbitSetOled
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Set the OLED for Analog Demo
**
*/
void OrbitSetOled() {

	char szAnalog[] = {'A', 'n', 'a', 'l', 'o', 'g', ':', ' ', '\0'};
	char szDemo1[]	= {'O', 'r', 'b', 'i', 't', ' ', 'D', 'e', 'm', 'o', '!', '\0'};
	char szDemo2[]	= {'B', 'y', ' ', 'D', 'i', 'g', 'i', 'l', 'e', 'n', 't', '\0'};

	OrbitOledSetCursor(0, 0);
	OrbitOledPutString(szDemo1);

	OrbitOledSetCursor(0, 1);
	OrbitOledPutString(szDemo2);

	OrbitOledMoveTo(0,19);
	OrbitOledLineTo(127, 19);

	OrbitOledSetCursor(0, 4);
	OrbitOledPutString(szAnalog);

}

/* ------------------------------------------------------------ */
/***	OrbitDemo
**
**	Parameters:
**		none
**
**	Return Value:
**		none
**
**	Errors:
**		none
**
**	Description:
**		Switches and buttons turn on LEDs, and the ADC reading
**		(altered with the potentiometer, VR1) is continuously
**		output to the OLED.
*/
void OrbitDemo() {

	uint32_t	ulAIN0;
	long 			lSwt1;
	long 			lSwt2;
	long 			lBtn1;
	long 			lBtn2;
	char			szAIN[6] = {0};
	char			cMSB = 0x00;
	char			cMIDB = 0x00;
	char			cLSB = 0x00;

	/* Check SWT and BTN states and update LEDs
	 *
	 */
	lSwt1 = GPIOPinRead(SWT1Port, SWT1);
	lSwt2 = GPIOPinRead(SWT2Port, SWT2);
	lBtn1 = GPIOPinRead(BTN1Port, BTN1);
	lBtn2 = GPIOPinRead(BTN2Port, BTN2);

	if(lSwt1 == SWT1) {
		GPIOPinWrite(LED3Port, LED3, LED3);
	}
	else {
		GPIOPinWrite(LED3Port, LED3, LOW);
	}
	if(lSwt2 == SWT2) {
		GPIOPinWrite(LED4Port, LED4, LED4);
	}
	else {
		GPIOPinWrite(LED4Port, LED4, LOW);
	}
	if(lBtn1 == BTN1) {
		GPIOPinWrite(LED1Port, LED1, LED1);
	}
	else {
		GPIOPinWrite(LED1Port, LED1, LOW);
	}
	if(lBtn2 == BTN2) {
		GPIOPinWrite(LED2Port, LED2, LED2);
	}
	else {
		GPIOPinWrite(LED2Port, LED2, LOW);
	}

	/*
	 * Initiate ADC Conversion and update the OLED
	 */
	ADCProcessorTrigger(ADC0_BASE, 0);

	while(!ADCIntStatus(ADC0_BASE, 0, false));

	ADCSequenceDataGet(ADC0_BASE, 0, &ulAIN0);

	/*
	 * Process data
	 */
	cMSB = (0xF00 & ulAIN0) >> 8;
	cMIDB = (0x0F0 & ulAIN0) >> 4;
	cLSB = (0x00F & ulAIN0);

	szAIN[0] = '0';
	szAIN[1] = 'x';
	szAIN[2] = (cMSB > 9) ? 'A' + (cMSB - 10) : '0' + cMSB;
	szAIN[3] = (cMIDB > 9) ? 'A' + (cMIDB - 10) : '0' + cMIDB;
	szAIN[4] = (cLSB > 9) ? 'A' + (cLSB - 10) : '0' + cLSB;
	szAIN[5] = '\0';

	OrbitOledSetCursor(8, 4);
	OrbitOledPutString(szAIN);

}
