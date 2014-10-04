
#include "netx_io_areas.h"
#include "delay.h"


void delay_us(unsigned int uiDelayUs)
{
	HOSTDEF(ptGpioArea);
	unsigned long ulTimerValue;
	unsigned long ulTimerCtrl;
	
	
	/*  Clear the timer register */
	ptGpioArea->aulGpio_counter_ctrl[0] = 0;
	
	/*  convert delay value to 10ns units */
	ulTimerValue = uiDelayUs * 100;
	
	/*  wait for 0.5 sec */
	ptGpioArea->aulGpio_counter_max[0] = ulTimerValue;
	
	/*  Clear the current timer value */
	ptGpioArea->aulGpio_counter_cnt[0] = 0;
	
	/*  Enable the timer */
	ptGpioArea->aulGpio_counter_ctrl[0] = HOSTMSK(gpio_counter0_ctrl_run)|HOSTMSK(gpio_counter0_ctrl_once);
	
	do
	{
		ulTimerCtrl  = ptGpioArea->aulGpio_counter_ctrl[0];
		ulTimerCtrl &= HOSTMSK(gpio_counter0_ctrl_run);
	} while( ulTimerCtrl!=0 );
	
	/*  Clear the timer register */
	ptGpioArea->aulGpio_counter_ctrl[0] = 0;
	
	/*  Reset the counter max value */
	ptGpioArea->aulGpio_counter_max[0] = 0;
}

