#define TARGET_IS_BLIZZARD_RA1
#include <lm4f120h5qr.h>
#include <rom.h>
#include <rom_map.h>
#include <hw_types.h>
#include <hw_memmap.h>
#include <hw_ints.h>
#include <pin_map.h>
#include <util.h>
#include <gpio.h>
#include <adc.h>
#include <uart.h>

#define UARTDR_0   *(int *)(0x4000c000)
#define UARTRSR_0  *(int *)(0x4000c004)
#define UARTECR_0  *(int *)(0x4000c004)
#define UARTFR_0   *(int *)(0x4000c018)
#define UARTILPR_0 *(int *)(0x4000c020)
#define UARTIBRD_0 *(int *)(0x4000c024)
#define UARTFBRD_0 *(int *)(0x4000c028)
#define UARTLCRH_0 *(int *)(0x4000c02c)
#define UARTCTL_0  *(int *)(0x4000c030)
#define UARTIFLS_0 *(int *)(0x4000c034)
#define UARTIM_0   *(int *)(0x4000c038)
#define UARTCC_0   *(int *)(0x4000cfc8)

void delay(unsigned long max)
{
	unsigned long counter;

	if (max == 0)
		max = 20000;

	for (counter = 0; counter < max; counter++)
		__asm("nop");
}

void uart_putchar(char c)
{
	ROM_UARTCharPutNonBlocking(UART0_BASE, c);
}

void uart_putdigit(unsigned char digit)
{
	if (digit < 10)
		uart_putchar('0' + digit);
	else
		uart_putchar('A' + digit - 10);
}

void uart_putfloat(float num)
{
	if (num > 10000)
		uart_putdigit(((int)num % 100000) / 10000);
	if (num > 1000)
		uart_putdigit(((int)num % 10000) / 1000);
	if (num > 100)
		uart_putdigit(((int)num % 1000) / 100);
	if (num > 10)
		uart_putdigit(((int)num % 100) / 10);
	uart_putdigit((int)num % 10);
	uart_putchar('.');
	uart_putdigit((int)(num * 10) % 10);
	uart_putdigit((int)(num * 100) % 10);
}

void UARTIntHandler(void)
{
	unsigned long ulStatus;

	ulStatus = ROM_UARTIntStatus(UART0_BASE, true);
	ROM_UARTIntClear(UART0_BASE, ulStatus);
	while (ROM_UARTCharsAvail(UART0_BASE)) {
		ROM_UARTCharPutNonBlocking(UART0_BASE, ROM_UARTCharGetNonBlocking(UART0_BASE));
	}
}

void UARTSend(const unsigned char *buf, unsigned long len)
{
	while (len--) {
		ROM_UARTCharPutNonBlocking(UART0_BASE, *buf++);
	}
}

int main(void)
{
	unsigned char rgb[] = {
		_BV(1),
		_BV(1) | _BV(2),
		_BV(2),
		_BV(2) | _BV(3),
		_BV(3),
		_BV(1) | _BV(3)
	};
	unsigned char i, j;
	unsigned long adcval;

	ROM_SysCtlClockSet(SYSCTL_SYSDIV_1 | SYSCTL_USE_OSC | SYSCTL_OSC_MAIN |
		SYSCTL_XTAL_16MHZ);

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);
	ROM_GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, GPIO_PIN_3 | GPIO_PIN_2 | GPIO_PIN_1);
	ROM_IntMasterEnable();
	ROM_GPIOPinConfigure(GPIO_PA0_U0RX);
	ROM_GPIOPinConfigure(GPIO_PA1_U0TX);
	ROM_GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

	ROM_UARTConfigSetExpClk(UART0_BASE, ROM_SysCtlClockGet(), 115200,
		(UART_CONFIG_WLEN_8 | UART_CONFIG_STOP_ONE |
		UART_CONFIG_PAR_NONE));

	ROM_IntEnable(INT_UART0);
	ROM_UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);

	ROM_SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	ROM_ADCSequenceConfigure(ADC0_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	ROM_ADCSequenceStepConfigure(ADC0_BASE, 0, 0, ADC_CTL_TS | ADC_CTL_IE | ADC_CTL_END);
	ROM_ADCSequenceEnable(ADC0_BASE, 0);
	ROM_ADCIntClear(ADC0_BASE, 0);

	while(1) {
		for (i = 0; i < sizeof(rgb); i++) {
			for (j = 0; j < 25; j++) {
				GPIO_PORTF_DATA_R = rgb[i];
				delay(400);
				GPIO_PORTF_DATA_R = 0;
				delay(4000);
			}
		}
		ROM_ADCProcessorTrigger(ADC0_BASE, 0);
		while(!ROM_ADCIntStatus(ADC0_BASE, 0, false)) ;
		ROM_ADCIntClear(ADC0_BASE, 0);
		ROM_ADCSequenceDataGet(ADC0_BASE, 0, &adcval);
		uart_putfloat(adcval);
		uart_putchar(' ');
		uart_putfloat(147 - ((75 * adcval * 33)) / 40960);
		uart_putchar('C');
		delay(1000);
		uart_putchar(' ');
	}
}
