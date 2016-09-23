#include <lm4f120h5qr.h>
#include <util.h>

void delay(unsigned long max)
{
	unsigned long counter;

	if (max == 0)
		max = 20000;

	for (counter = 0; counter < max; counter++)
		__asm("nop");
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

	 /* Enable PORTF */
	SYSCTL_RCGC2_R = SYSCTL_RCGC2_GPIOF;

	__asm("nop");

	/* Set PF1, PF2, PF3 as output */
	GPIO_PORTF_DIR_R = 0x0e;

	/* Enable Digital IO on PF1, PF2, PF3 */
	GPIO_PORTF_DEN_R = 0x0e;

	while(1) {
		for (i = 0; i < sizeof(rgb); i++) {
			for (j = 0; j < 25; j++) {
				GPIO_PORTF_DATA_R = rgb[i];
				delay(500);
				GPIO_PORTF_DATA_R = 0;
				delay(5000);
			}
		}
	}
}
