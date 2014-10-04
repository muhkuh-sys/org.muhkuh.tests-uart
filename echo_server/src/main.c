
#include <string.h>

#include "netx_io_areas.h"

#include "delay.h"
#include "rdy_run.h"
#include "serial_vectors.h"
#include "systime.h"
#include "uart.h"
#include "uprintf.h"
#include "version.h"


/*-------------------------------------------------------------------------*/


typedef enum ENUM_CHIP_SUBTYP
{
	CHIP_SUBTYP_NETX50              = 0,
	CHIP_SUBTYP_NETX51              = 1,
	CHIP_SUBTYP_NETX52              = 2
} CHIP_SUBTYP_T;


#if ASIC_TYP==10
/* NXHX10-ETM */
static const UART_CONFIGURATION_T tUartCfg =
{
        .uc_rx_mmio = 20U,
        .uc_tx_mmio = 21U,
        .uc_rts_mmio = 0xffU,
        .uc_cts_mmio = 0xffU,
        .us_baud_div = UART_BAUDRATE_DIV(UART_BAUDRATE_115200)
};
#elif ASIC_TYP==56
/* NXHX51-ETM */
static const UART_CONFIGURATION_T tUartCfg_netx51 =
{
        .uc_rx_mmio = 34U,
        .uc_tx_mmio = 35U,
        .uc_rts_mmio = 0xffU,
        .uc_cts_mmio = 0xffU,
        .us_baud_div = UART_BAUDRATE_DIV(UART_BAUDRATE_115200)
};
/* NXHX51-ETM */
static const UART_CONFIGURATION_T tUartCfg_netx52 =
{
        .uc_rx_mmio = 20U,
        .uc_tx_mmio = 21U,
        .uc_rts_mmio = 0xffU,
        .uc_cts_mmio = 0xffU,
        .us_baud_div = UART_BAUDRATE_DIV(UART_BAUDRATE_115200)
};
#elif ASIC_TYP==50
/* NXHX50-ETM */
static const UART_CONFIGURATION_T tUartCfg =
{
        .uc_rx_mmio = 34U,
        .uc_tx_mmio = 35U,
        .uc_rts_mmio = 0xffU,
        .uc_cts_mmio = 0xffU,
        .us_baud_div = UART_BAUDRATE_DIV(UART_BAUDRATE_115200)
};
#elif ASIC_TYP==100 || ASIC_TYP==500
static const UART_CONFIGURATION_T tUartCfg =
{
        .us_baud_div = UART_BAUDRATE_DIV(UART_BAUDRATE_115200)
};
#endif


#define IO_UART_UNIT 0

static unsigned char io_uart_get(void)
{
	return (unsigned char)uart_get(IO_UART_UNIT);
}


static void io_uart_put(unsigned int uiChar)
{
	uart_put(IO_UART_UNIT, (unsigned char)uiChar);
}


static unsigned int io_uart_peek(void)
{
	return uart_peek(IO_UART_UNIT);
}


static void io_uart_flush(void)
{
	uart_flush(IO_UART_UNIT);
}


static const SERIAL_COMM_UI_FN_T tSerialVectors_Uart =
{
	.fn =
	{
		.fnGet = io_uart_get,
		.fnPut = io_uart_put,
		.fnPeek = io_uart_peek,
		.fnFlush = io_uart_flush
	}
};


SERIAL_COMM_UI_FN_T tSerialVectors;


/*-------------------------------------------------------------------------*/


static const UART_CONFIGURATION_T tUart1Cfg =
{
	.uc_rx_mmio = 0U,
	.uc_tx_mmio = 1U,
	.uc_rts_mmio = 0xffU,
	.uc_cts_mmio = 0xffU,
	.us_baud_div = UART_BAUDRATE_DIV(UART_BAUDRATE_115200)
};



void echo_server_main(void) __attribute__ ((noreturn));
void echo_server_main(void)
{
	BLINKI_HANDLE_T tBlinkiHandle;
#if ASIC_TYP==56
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
	unsigned long ulValue;
	unsigned long ulChipSubType;
	CHIP_SUBTYP_T tChipSubTyp;
#endif
	unsigned int uiData;


	systime_init();

	/* Set the serial vectors. */
	memcpy(&tSerialVectors, &tSerialVectors_Uart, sizeof(SERIAL_COMM_UI_FN_T));

#if ASIC_TYP==56
	/* Get the chip subtype from mem_a18 and mem_a19:
	 *  18 19
	 *   0  0  netX50
	 *   1  0  netX51
	 *   0  1  netX52
	 *   1  1  reserved
	 */
	ulValue = ptAsicCtrlArea->ulSample_at_nres;
	ulChipSubType  = (ulValue&HOSTMSK(sample_at_nres_sar_mem_a18))>> HOSTSRT(sample_at_nres_sar_mem_a18);
	ulChipSubType |= (ulValue&HOSTMSK(sample_at_nres_sar_mem_a19))>>(HOSTSRT(sample_at_nres_sar_mem_a19)-1);
	tChipSubTyp = (CHIP_SUBTYP_T)ulChipSubType;

	/* netX51 and netX52 have different default UART pins. */
	if( tChipSubTyp==CHIP_SUBTYP_NETX52 )
	{
		uart_init(IO_UART_UNIT, &tUartCfg_netx52);
	}
	else
	{
		uart_init(IO_UART_UNIT, &tUartCfg_netx51);
	}
#else
	uart_init(IO_UART_UNIT, &tUartCfg);
#endif

	uprintf("\f. *** Echo server by Christoph Thelen <doc_bacardi@users.sourceforge.net> ***\n");
	uprintf("V" VERSION_ALL "\n\n");
	
	/* Switch all LEDs off. */
	rdy_run_setLEDs(RDYRUN_OFF);
	
	
	/* Init UART1. */
	uart_init(1, &tUart1Cfg);
	
	/* setup the MMIO pins */
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
	/* MMIO3 is connected to PB_ENB. This should be a PIO pin. */
	ptMmioCtrlArea->aulMmio_cfg[3] = MMIO_CFG_PIO;
	
	
	/* Switch the driver off -> drive MMIO3 to low. */
	ptMmioCtrlArea->aulMmio_pio_out_line_cfg[0] = 0;
	ptMmioCtrlArea->aulMmio_pio_out_line_cfg[1] = 0;
	
	ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[0] = 1U << 3U;
	ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[1] = 0;
	
	
	rdy_run_blinki_init(&tBlinkiHandle, 0x00000055, 0x00000150);
	while(1)
	{
		if( uart_peek(1)!=0 )
		{
			uiData = uart_get(1);
//			uprintf("%02x\n", uiData);
			if( uiData<0x100U )
			{
				/* Enable the output driver. */
				ptMmioCtrlArea->aulMmio_pio_out_line_cfg[0] = 1U << 3U;
				delay_us(10);
				
				/* Send the data. */
				uart_put(1, (unsigned char)uiData);
				
				/* Wait until the data has been sent. */
				uart_flush(1);
				
				/* Ignore the local echo. */
				uiData = uart_get(1);
				
				/* Disable the output driver. */
				ptMmioCtrlArea->aulMmio_pio_out_line_cfg[0] = 0U;
//				systime_delay(2);
			}
		}
		rdy_run_blinki(&tBlinkiHandle);
	}
}


/*-----------------------------------*/

