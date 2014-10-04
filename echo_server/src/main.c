
#include <string.h>

#include "netx_io_areas.h"

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


void echo_server_main(void) __attribute__ ((noreturn));
void echo_server_main(void)
{
#if ASIC_TYP==56
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
	unsigned long ulValue;
	unsigned long ulChipSubType;
	CHIP_SUBTYP_T tChipSubTyp;
#endif


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

	while(1);
}

/*-----------------------------------*/

