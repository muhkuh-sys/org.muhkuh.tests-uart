
#include "main_test.h"

#include <string.h>

#include "delay.h"
#include "netx_io_areas.h"
#include "netx_test.h"
#include "rdy_run.h"
#include "systime.h"
#include "uprintf.h"
#include "version.h"


/*-------------------------------------------------------------------------*/


typedef struct
{
	HOSTADEF(UART) * const ptArea;
#if ASIC_TYP==10 || ASIC_TYP==50 || ASIC_TYP==56 || ASIC_TYP==6
	MMIO_CFG_T tMmioRx;
	MMIO_CFG_T tMmioTx;
	MMIO_CFG_T tMmioRts;
	MMIO_CFG_T tMmioCts;
#endif
} UART_INSTANCE_T;


static const UART_INSTANCE_T atUartInstances[] =
{
#if ASIC_TYP==10
	{
		(NX10_UART_AREA_T * const)Addr_NX10_uart0,
		MMIO_CFG_uart0_rxd,
		MMIO_CFG_uart0_txd,
		MMIO_CFG_uart0_rtsn,
		MMIO_CFG_uart0_ctsn
	},

	{
		(NX10_UART_AREA_T * const)Addr_NX10_uart1,
		MMIO_CFG_uart1_rxd,
		MMIO_CFG_uart1_txd,
		MMIO_CFG_uart1_rtsn,
		MMIO_CFG_uart1_ctsn
	}
#elif ASIC_TYP==50
	{
		(NX50_UART_AREA_T * const)Addr_NX50_uart0,
		MMIO_CFG_uart0_rxd,
		MMIO_CFG_uart0_txd,
		MMIO_CFG_uart0_rts,
		MMIO_CFG_uart0_cts
	},

	{
		(NX50_UART_AREA_T * const)Addr_NX50_uart1,
		MMIO_CFG_uart1_rxd,
		MMIO_CFG_uart1_txd,
		MMIO_CFG_uart1_rts,
		MMIO_CFG_uart1_cts
	},

	{
		(NX50_UART_AREA_T * const)Addr_NX50_uart2,
		MMIO_CFG_uart2_rxd,
		MMIO_CFG_uart2_txd,
		MMIO_CFG_uart2_rts,
		MMIO_CFG_uart2_cts
	}
#elif ASIC_TYP==56
	{
		(NX56_UART_AREA_T * const)Addr_NX56_uart0,
		MMIO_CFG_uart0_rxd,
		MMIO_CFG_uart0_txd,
		MMIO_CFG_uart0_rtsn,
		MMIO_CFG_uart0_ctsn
	},

	{
		(NX56_UART_AREA_T * const)Addr_NX56_uart1,
		MMIO_CFG_uart1_rxd,
		MMIO_CFG_uart1_txd,
		MMIO_CFG_uart1_rtsn,
		MMIO_CFG_uart1_ctsn
	},

	{
		(NX56_UART_AREA_T * const)Addr_NX56_uart2,
		MMIO_CFG_uart2_rxd,
		MMIO_CFG_uart2_txd,
		MMIO_CFG_uart2_rtsn,
		MMIO_CFG_uart2_ctsn
	}
#elif ASIC_TYP==6
	{
		(NX6_UART_AREA_T * const)Addr_NX6_uart0,
		MMIO_CFG_uart0_rxd,
		MMIO_CFG_uart0_txd,
		MMIO_CFG_uart0_rtsn,
		MMIO_CFG_uart0_ctsn
	},

	{
		(NX6_UART_AREA_T * const)Addr_NX6_uart1,
		MMIO_CFG_uart1_rxd,
		MMIO_CFG_uart1_txd,
		MMIO_CFG_uart1_rtsn,
		MMIO_CFG_uart1_ctsn
	},

	{
		(NX6_UART_AREA_T * const)Addr_NX6_uart2,
		MMIO_CFG_uart2_rxd,
		MMIO_CFG_uart2_txd,
		MMIO_CFG_uart2_rtsn,
		MMIO_CFG_uart2_ctsn
	}
#elif ASIC_TYP==500 || ASIC_TYP==100
	{
		(NX500_UART_AREA_T * const)Addr_NX500_uart0
	},

	{
		(NX500_UART_AREA_T * const)Addr_NX500_uart1
	},

	{
		(NX500_UART_AREA_T * const)Addr_NX500_uart2
	}
#endif
};


typedef struct UART_CONFIGURATION_STRUCT
{
	unsigned int uiUnit;
	unsigned long ulBaudDiv;
	HOSTADEF(UART) * ptArea;
	unsigned char aucMMIO[4];
	unsigned long ulFlags;
} UART_CONFIGURATION_T;


/*-------------------------------------------------------------------------*/

#if ASIC_TYP==56
static void mmio_set(unsigned int uiIdx, unsigned int uiValue)
{
	HOSTDEF(ptMmioCtrlArea);
	unsigned long aulOe[2];
	unsigned long aulOut[2];
	
	
	aulOe[0] = ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[0];
	aulOe[1] = ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[1];
	
	aulOe[0] |= 1U << uiIdx;
	aulOe[1] |= 1U << (uiIdx-32U);
	
	ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[0] = aulOe[0];
	ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[1] = aulOe[1];
	
	
	aulOut[0] = ptMmioCtrlArea->aulMmio_pio_out_line_cfg[0];
	aulOut[1] = ptMmioCtrlArea->aulMmio_pio_out_line_cfg[1];
	
	if( uiValue==0 )
	{
		aulOut[0] &= ~(1U << uiIdx);
		aulOut[1] &= ~(1U << (uiIdx-32U));
	}
	else
	{
		aulOut[0] |= 1U << uiIdx;
		aulOut[1] |= 1U << (uiIdx-32U);
	}
	
	ptMmioCtrlArea->aulMmio_pio_out_line_cfg[0] = aulOut[0];
	ptMmioCtrlArea->aulMmio_pio_out_line_cfg[1] = aulOut[1];
}


static unsigned long mmio_get(unsigned int uiIdx)
{
	HOSTDEF(ptMmioCtrlArea);
	unsigned long ulResult;
	
	
	if( uiIdx<32 )
	{
		ulResult  = ptMmioCtrlArea->aulMmio_in_line_status[0];
		ulResult &= 1U << uiIdx;
	}
	else
	{
		ulResult  = ptMmioCtrlArea->aulMmio_in_line_status[1];
		ulResult &= 1U << (uiIdx - 32);
	}
	
	return ulResult;
}


static void mmio_set_to_input(unsigned int uiIdx)
{
	HOSTDEF(ptMmioCtrlArea);
	unsigned long aulOe[2];
	
	
	aulOe[0] = ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[0];
	aulOe[1] = ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[1];
	
	aulOe[0] &= ~(1U << uiIdx);
	aulOe[1] &= ~(1U << (uiIdx-32U));
	
	ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[0] = aulOe[0];
	ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[1] = aulOe[1];
}
#endif


static void io_stabilize(void)
{
	unsigned long ulTimerHandle;
	int iElapsed;
	
	
	/* Delay a few cycles. */
	ulTimerHandle = systime_get_ms();
	do
	{
		iElapsed = systime_elapsed(ulTimerHandle, 2);
	} while( iElapsed==0 );
}

static int rtscts_io_test(UART_CONFIGURATION_T *ptCfg)
{
	int iResult;
#if ASIC_TYP==10 || ASIC_TYP==50 || ASIC_TYP==56 || ASIC_TYP==6
	unsigned int uiIdxCts;
	unsigned int uiIdxRts;
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
	unsigned long ulValue;
#elif ASIC_TYP==100 || ASIC_TYP==500
/*	HOSTDEF(ptGpioArea); */
#endif
	
	
	/* Be pessimistic. */
	iResult = -1;
	
#if ASIC_TYP==10
	
#elif ASIC_TYP==56
	/* Set the RTS and CTS MMIO pins to PIO mode. */
	uiIdxCts = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_CTS];
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
	ptMmioCtrlArea->aulMmio_cfg[uiIdxCts] = MMIO_CFG_PIO;
	
	uiIdxRts = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RTS];
	ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
	ptMmioCtrlArea->aulMmio_cfg[uiIdxRts] = MMIO_CFG_PIO;
	
	
	mmio_set_to_input(uiIdxCts);
	
	/* Drive RTS to 0. */
	uprintf("RTS -> 0\n");
	mmio_set(uiIdxRts, 0);
	io_stabilize();
	ulValue = mmio_get(uiIdxCts);
	if( ulValue!=0 )
	{
		uprintf("RTS set to 0, but CTS does not follow!\n");
	}
	else
	{
		/* Drive RTS to 1. */
		uprintf("RTS -> 1\n");
		mmio_set_to_input(uiIdxCts);
		mmio_set(uiIdxRts, 1);
		io_stabilize();
		ulValue = mmio_get(uiIdxCts);
		if( ulValue==0 )
		{
			uprintf("RTS set to 1, but CTS does not follow!\n");
		}
		else
		{
			iResult = 0;
		}
	}
	
	/* Set RTS back to input. */
	mmio_set_to_input(uiIdxRts);
#elif ASIC_TYP==50
	
#elif ASIC_TYP==100 || ASIC_TYP==500
	
#endif
	
	return iResult;
}


/*-------------------------------------------------------------------------*/

/* Setup the UART unit. */
static int uart_init(UART_CONFIGURATION_T *ptCfg)
{
	unsigned int uiUartUnit;
	unsigned long ulValue;
	HOSTADEF(UART) *ptUartArea;
	int iResult;
	unsigned int uiIdx;
	unsigned long ulUseRtsCts;
#if ASIC_TYP==10 || ASIC_TYP==50 || ASIC_TYP==56 || ASIC_TYP==6
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
#elif ASIC_TYP==100 || ASIC_TYP==500
	HOSTDEF(ptGpioArea);
#endif
	
	
	/* expect error */
	iResult = -1;
	
	ulUseRtsCts  = ptCfg->ulFlags;
	ulUseRtsCts &= (unsigned long)UARTTEST_FLAGS_Use_CTS_RTS;
	
	uiUartUnit = ptCfg->uiUnit;
	if( uiUartUnit<(sizeof(atUartInstances)/sizeof(atUartInstances[0])) )
	{
		/* Get the UART area. */
		ptUartArea = atUartInstances[uiUartUnit].ptArea;
		
		/* Remember the pointer to the area for later. */
		ptCfg->ptArea = ptUartArea;
		
		/* Disable the UART. */
		ptUartArea->ulUartcr = 0;
		
		/* Use baud rate mode 2. */
		ptUartArea->ulUartcr_2 = HOSTMSK(uartcr_2_Baud_Rate_Mode);
		
		/* Set the baud rate. */
		ulValue = ptCfg->ulBaudDiv;
		ptUartArea->ulUartlcr_l =  ulValue        & 0xffU;
		ptUartArea->ulUartlcr_m = (ulValue >> 8U) & 0xffU;
		
		/* Set the UART to 8N1 and enable the FIFO. */
		ulValue  = HOSTMSK(uartlcr_h_WLEN);
		ulValue |= HOSTMSK(uartlcr_h_FEN);
		ptUartArea->ulUartlcr_h = ulValue;
		
		/* Enable the drivers for the TXD and RTS line. */
		ulValue  = HOSTMSK(uartdrvout_DRVTX);
		if( ulUseRtsCts!=0 )
		{
			ulValue |= HOSTMSK(uartdrvout_DRVRTS);
		}
		ptUartArea->ulUartdrvout = ulValue;
		
		/* Enable RTS/CTS mode. */
		ulValue  = 0;
		if( ulUseRtsCts!=0 )
		{
			ulValue |= HOSTMSK(uartrts_AUTO);
			ulValue |= HOSTMSK(uartrts_CTS_ctr);
			ulValue |= HOSTMSK(uartrts_CTS_pol);
		}
		ptUartArea->ulUartrts = ulValue;
		
#if ASIC_TYP==50
		/* Loop over the complete MMIO pins and disable all UART instances. */
		for(uiIdx=0; uiIdx<(sizeof(ptMmioCtrlArea->aulMmio_cfg)/sizeof(ptMmioCtrlArea->aulMmio_cfg[0])); ++uiIdx)
		{
			ulValue  = ptMmioCtrlArea->aulMmio_cfg[uiIdx];
			ulValue &= HOSTMSK(mmio0_cfg_mmio0_sel);
			if( ulValue==((unsigned long)(atUartInstances[uiUartUnit].tMmioRx)) )
			{
				uprintf("Disable RXD at MMIO%d.\n", uiIdx);
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
				ptMmioCtrlArea->aulMmio_cfg[uiIdx] = ((unsigned long)MMIO_CFG_DISABLE);
			}
			else if( ulValue==((unsigned long)(atUartInstances[uiUartUnit].tMmioTx)) )
			{
				uprintf("Disable TXD at MMIO%d.\n", uiIdx);
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
				ptMmioCtrlArea->aulMmio_cfg[uiIdx] = ((unsigned long)MMIO_CFG_DISABLE);
			}
			
			if( ulUseRtsCts!=0 )
			{
				if( ulValue==((unsigned long)(atUartInstances[uiUartUnit].tMmioCts)) )
				{
					uprintf("Disable CTS at MMIO%d.\n", uiIdx);
					ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
					ptMmioCtrlArea->aulMmio_cfg[uiIdx] = ((unsigned long)MMIO_CFG_DISABLE);
				}
				else if( ulValue==((unsigned long)(atUartInstances[uiUartUnit].tMmioRts)) )
				{
					uprintf("Disable RTS at MMIO%d.\n", uiIdx);
					ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
					ptMmioCtrlArea->aulMmio_cfg[uiIdx] = ((unsigned long)MMIO_CFG_DISABLE);
				}
			}
		}
		
		
		/* Setup the MMIO pins. */
		uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RXD];
		if( uiIdx!=0xffU )
		{
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
			ptMmioCtrlArea->aulMmio_cfg[uiIdx] = atUartInstances[uiUartUnit].tMmioRx;
		}
		
		uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_TXD];
		if( uiIdx!=0xffU )
		{
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
			ptMmioCtrlArea->aulMmio_cfg[uiIdx] = atUartInstances[uiUartUnit].tMmioTx;
		}
		
		if( ulUseRtsCts!=0 )
		{
			uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_CTS];
			if( uiIdx!=0xffU )
			{
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
				ptMmioCtrlArea->aulMmio_cfg[uiIdx] = atUartInstances[uiUartUnit].tMmioCts;
			}
			
			uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RTS];
			if( uiIdx!=0xffU )
			{
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
				ptMmioCtrlArea->aulMmio_cfg[uiIdx] = atUartInstances[uiUartUnit].tMmioRts;
			}
		}
#elif ASIC_TYP==10 || ASIC_TYP==56 || ASIC_TYP==6
		/* Loop over the complete MMIO pins and disable all UART instances. */
		for(uiIdx=0; uiIdx<(sizeof(ptMmioCtrlArea->aulMmio_cfg)/sizeof(ptMmioCtrlArea->aulMmio_cfg[0])); ++uiIdx)
		{
			ulValue  = ptMmioCtrlArea->aulMmio_cfg[uiIdx];
			ulValue &= HOSTMSK(mmio0_cfg_mmio_sel);
			if( ulValue==((unsigned long)(atUartInstances[uiUartUnit].tMmioRx)) )
			{
				uprintf("Disable RXD at MMIO%d.\n", uiIdx);
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
				ptMmioCtrlArea->aulMmio_cfg[uiIdx] = ((unsigned long)MMIO_CFG_DISABLE);
			}
			else if( ulValue==((unsigned long)(atUartInstances[uiUartUnit].tMmioTx)) )
			{
				uprintf("Disable TXD at MMIO%d.\n", uiIdx);
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
				ptMmioCtrlArea->aulMmio_cfg[uiIdx] = ((unsigned long)MMIO_CFG_DISABLE);
			}
			
			if( ulUseRtsCts!=0 )
			{
				if( ulValue==((unsigned long)(atUartInstances[uiUartUnit].tMmioCts)) )
				{
					uprintf("Disable CTS at MMIO%d.\n", uiIdx);
					ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
					ptMmioCtrlArea->aulMmio_cfg[uiIdx] = ((unsigned long)MMIO_CFG_DISABLE);
				}
				else if( ulValue==((unsigned long)(atUartInstances[uiUartUnit].tMmioRts)) )
				{
					uprintf("Disable RTS at MMIO%d.\n", uiIdx);
					ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
					ptMmioCtrlArea->aulMmio_cfg[uiIdx] = ((unsigned long)MMIO_CFG_DISABLE);
				}
			}
		}
		
		
		/* Setup the MMIO pins. */
		uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RXD];
		if( uiIdx!=0xffU )
		{
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
			ptMmioCtrlArea->aulMmio_cfg[uiIdx] = atUartInstances[uiUartUnit].tMmioRx;
		}
		
		uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_TXD];
		if( uiIdx!=0xffU )
		{
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
			ptMmioCtrlArea->aulMmio_cfg[uiIdx] = atUartInstances[uiUartUnit].tMmioTx;
		}
		
		if( ulUseRtsCts!=0 )
		{
			uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_CTS];
			if( uiIdx!=0xffU )
			{
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
				ptMmioCtrlArea->aulMmio_cfg[uiIdx] = atUartInstances[uiUartUnit].tMmioCts;
			}
			
			uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RTS];
			if( uiIdx!=0xffU )
			{
				ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
				ptMmioCtrlArea->aulMmio_cfg[uiIdx] = atUartInstances[uiUartUnit].tMmioRts;
			}
		}
#elif ASIC_TYP==100 || ASIC_TYP==500
		uiIdx = uiUartUnit << 2;
		ptGpioArea->aulGpio_cfg[uiIdx+0] = 2;
		ptGpioArea->aulGpio_cfg[uiIdx+1] = 2;
		if( ulUseRtsCts!=0 )
		{
			ptGpioArea->aulGpio_cfg[uiIdx+2] = 2;
			ptGpioArea->aulGpio_cfg[uiIdx+3] = 2;
		}
#endif
		
		/* Enable the UART. */
		ptUartArea->ulUartcr = HOSTMSK(uartcr_uartEN);
		
		/* Clear the FIFO from any garbage. */
		uiIdx = 256;
		do
		{
			ulValue  = ptUartArea->ulUartfr;
			ulValue &= HOSTMSK(uartfr_RXFE);
			if( ulValue!=0 )
			{
				/* The receive FIFO is empty. */
				uprintf("The receive FIFO is clear!\n");
				break;
			}
			else
			{
				ulValue = ptUartArea->ulUartdr;
				uprintf("Remove garbage from the receive FIFO: 0x%02x\n", ulValue);
			}
			--uiIdx;
		} while( uiIdx!=0 );
		
		if( uiIdx==0 )
		{
			uprintf("The receive FIFO is continuously filled with garbage. Stopping the test!\n");
		}
		else
		{
			iResult = 0;
		}
	}
	
	return iResult;
}



static void init_io_output_driver(UART_CONFIGURATION_T *ptCfg)
{
#if ASIC_TYP==56
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
	unsigned long ulMmioIdx;
	unsigned long aulMask[2];
#endif
	
	
#if ASIC_TYP==56
	ulMmioIdx = 3U;
	/* Is this a valid MMIO index? */
	if( ulMmioIdx!=0xffU )
	{
		aulMask[0] = 1U <<  ulMmioIdx;
		aulMask[1] = 1U << (ulMmioIdx - 32U);
		
		/* Setup the MMIO pin. */
		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
		ptMmioCtrlArea->aulMmio_cfg[ulMmioIdx] = MMIO_CFG_PIO;
		
		/* Switch the pin to input. */
		ptMmioCtrlArea->aulMmio_pio_out_line_cfg[0] &= ~(aulMask[0]);
		ptMmioCtrlArea->aulMmio_pio_out_line_cfg[1] &= ~(aulMask[1]);
		ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[0]  &= ~(aulMask[0]);
		ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[1]  &= ~(aulMask[1]);
	}
#endif
}



/* Enable or disable an output driver. Delay after enabling the driver. */
static void set_io_output_driver(UART_CONFIGURATION_T *ptCfg, unsigned int uiEnable)
{
#if ASIC_TYP==56
	HOSTDEF(ptMmioCtrlArea);
	unsigned long ulMmioIdx;
	unsigned long aulMask[2];
#endif


#if ASIC_TYP==56
	ulMmioIdx = 3U;
	/* Is this a valid MMIO index? */
	if( ulMmioIdx!=0xffU )
	{
		/* Yes, the MMIO index is valid! */
		
		aulMask[0] = 1U <<  ulMmioIdx;
		aulMask[1] = 1U << (ulMmioIdx - 32U);
		
		/* Enable or disable the driver? */
		if( uiEnable==0 )
		{
			/* Disable the driver. */
			ptMmioCtrlArea->aulMmio_pio_out_line_cfg[0] &= ~(aulMask[0]);
			ptMmioCtrlArea->aulMmio_pio_out_line_cfg[1] &= ~(aulMask[1]);
		}
		else
		{
			/* Enable the driver. */
			ptMmioCtrlArea->aulMmio_pio_out_line_cfg[0] |=   aulMask[0];
			ptMmioCtrlArea->aulMmio_pio_out_line_cfg[1] |=   aulMask[1];
		}
		ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[0] |= aulMask[0];
		ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[1] |= aulMask[1];
		
		/* Delay if the driver was enabled. */
		if( uiEnable!=0 )
		{
			delay_us(10);
		}
	}
#endif
}



static void deinit_io_output_driver(UART_CONFIGURATION_T *ptCfg)
{
#if ASIC_TYP==56
	HOSTDEF(ptMmioCtrlArea);
	unsigned long ulMmioIdx;
	unsigned long aulMask[2];
#endif
	
	
#if ASIC_TYP==56
	ulMmioIdx = 3U;
	/* Is this a valid MMIO index? */
	if( ulMmioIdx!=0xffU )
	{
		aulMask[0] = 1U <<  ulMmioIdx;
		aulMask[1] = 1U << (ulMmioIdx - 32U);
		
		/* Switch the pin to input. */
		ptMmioCtrlArea->aulMmio_pio_out_line_cfg[0] &= ~(aulMask[0]);
		ptMmioCtrlArea->aulMmio_pio_out_line_cfg[1] &= ~(aulMask[1]);
		ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[0]  &= ~(aulMask[0]);
		ptMmioCtrlArea->aulMmio_pio_oe_line_cfg[1]  &= ~(aulMask[1]);
	}
#endif
}



static int uart_test(UART_CONFIGURATION_T *ptCfg)
{
	int iResult;
	HOSTADEF(UART) *ptUartArea;
	unsigned long ulLoopMax;
	unsigned long ulLoopCnt;
	unsigned long ulValue;
	unsigned long ulTimerHandle;
	unsigned long ulRxTimeoutMs;
	unsigned long ulSend;
	
	
	uprintf("Transfering data...\n");
	
	ulRxTimeoutMs = 500;
	
	/* Be pessimistic. */
	iResult = -1;
	
	ptUartArea = ptCfg->ptArea;
	
	
	init_io_output_driver(ptCfg);
	set_io_output_driver(ptCfg, 0);

	ulLoopMax = 512;
	ulLoopCnt = 0;
	while( ulLoopCnt<ulLoopMax )
	{
		if( ((ptCfg->ulFlags)&((unsigned long)UARTTEST_FLAGS_Has_Output_Driver))!=0 )
		{
			/* Enable the output driver. */
			set_io_output_driver(ptCfg, 1);
		}
		
		/* Send one byte. */
		ulSend = ulLoopCnt & 0xffU;
		ptUartArea->ulUartdr = ulSend;
		
		if( ((ptCfg->ulFlags)&((unsigned long)UARTTEST_FLAGS_Receive_Own_Echo))!=0 )
		{
			/* Wait until the UART is not busy anymore. */
			ulTimerHandle = systime_get_ms();
			do
			{
				iResult = systime_elapsed(ulTimerHandle, ulRxTimeoutMs);
				if( iResult!=0 )
				{
					break;
				}
				
				ulValue  = ptUartArea->ulUartfr;
				ulValue &= HOSTMSK(uartfr_BUSY);
			} while( ulValue!=0 );
			if( iResult!=0 )
			{
				uprintf("The UART could not send the data for loop %d after %dms!\n", ulLoopCnt, ulRxTimeoutMs);
				break;
			}
			
			/* Now there must be our own echo. */
			ulTimerHandle = systime_get_ms();
			do
			{
				iResult = systime_elapsed(ulTimerHandle, ulRxTimeoutMs);
				if( iResult!=0 )
				{
					break;
				}
				
				ulValue  = ptUartArea->ulUartfr;
				ulValue &= HOSTMSK(uartfr_RXFE);
			} while( ulValue!=0 );
		}
		
		if( ((ptCfg->ulFlags)&((unsigned long)UARTTEST_FLAGS_Has_Output_Driver))!=0 )
		{
			/* Disable the output driver. */
			set_io_output_driver(ptCfg, 0);
		}
		
		if( ((ptCfg->ulFlags)&((unsigned long)UARTTEST_FLAGS_Receive_Own_Echo))!=0 )
		{
			if( iResult!=0 )
			{
				/* The FIFO is still empty. */
				uprintf("Did not receive our own echo in loop %d!\n", ulLoopCnt);
				iResult = -1;
				break;
			}
			
			/* Get the received byte and compare it to the send data. */
			ulValue = ptUartArea->ulUartdr;
			if( ulValue!=ulSend )
			{
				uprintf("Sent 0x%02x, but received local echo 0x%02x!\n", ulSend, ulValue);
				iResult = -1;
				break;
			}
		}
		
		/* Wait for a data in the receive FIFO. */
		ulTimerHandle = systime_get_ms();
		do
		{
			iResult = systime_elapsed(ulTimerHandle, ulRxTimeoutMs);
			if( iResult!=0 )
			{
				break;
			}
			
			ulValue  = ptUartArea->ulUartfr;
			ulValue &= HOSTMSK(uartfr_RXFE);
		} while( ulValue!=0 );
		if( iResult!=0 )
		{
			uprintf("No response received for loop %d after %dms!\n", ulLoopCnt, ulRxTimeoutMs);
			break;
		}
		
		/* Get the received byte and compare it to the send data. */
		ulValue = ptUartArea->ulUartdr;
		if( ulValue!=ulSend )
		{
			uprintf("Sent 0x%02x, but received 0x%02x!\n", ulSend, ulValue);
			iResult = -1;
			break;
		}
		
		++ulLoopCnt;
	}
	
	deinit_io_output_driver(ptCfg);
	
	if( iResult==0 )
	{
		uprintf("Transfer OK!\n");
	}

	return iResult;
}



static void uart_shutdown(UART_CONFIGURATION_T *ptCfg)
{
	HOSTADEF(UART) *ptUartArea;
	unsigned long ulUseRtsCts;
#if ASIC_TYP==10 || ASIC_TYP==50 || ASIC_TYP==56 || ASIC_TYP==6
	unsigned int uiIdx;
	HOSTDEF(ptAsicCtrlArea);
	HOSTDEF(ptMmioCtrlArea);
#elif ASIC_TYP==100 || ASIC_TYP==500
	HOSTDEF(ptGpioArea)
	unsigned int uiIdx;
#endif


	ulUseRtsCts  = ptCfg->ulFlags;
	ulUseRtsCts &= (unsigned long)UARTTEST_FLAGS_Use_CTS_RTS;

	ptUartArea = ptCfg->ptArea;

#if ASIC_TYP==10 || ASIC_TYP==50 || ASIC_TYP==56 || ASIC_TYP==6
	/* Setup the MMIO pins. */
	uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RXD];
	if( uiIdx!=0xffU )
	{
		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
		ptMmioCtrlArea->aulMmio_cfg[uiIdx] = MMIO_CFG_DISABLE;
	}

	uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_TXD];
	if( uiIdx!=0xffU )
	{
		ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
		ptMmioCtrlArea->aulMmio_cfg[uiIdx] = MMIO_CFG_DISABLE;
	}

	if( ulUseRtsCts!=0 )
	{
		uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_CTS];
		if( uiIdx!=0xffU )
		{
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
			ptMmioCtrlArea->aulMmio_cfg[uiIdx] = MMIO_CFG_DISABLE;
		}

		uiIdx = ptCfg->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RTS];
		if( uiIdx!=0xffU )
		{
			ptAsicCtrlArea->ulAsic_ctrl_access_key = ptAsicCtrlArea->ulAsic_ctrl_access_key;
			ptMmioCtrlArea->aulMmio_cfg[uiIdx] = MMIO_CFG_DISABLE;
		}
	}
#elif ASIC_TYP==100 || ASIC_TYP==500
	uiIdx = ptCfg->uiUnit << 2;
	ptGpioArea->aulGpio_cfg[uiIdx+0] = 0;
	ptGpioArea->aulGpio_cfg[uiIdx+1] = 0;
	if( ulUseRtsCts!=0 )
	{
		ptGpioArea->aulGpio_cfg[uiIdx+2] = 0;
		ptGpioArea->aulGpio_cfg[uiIdx+3] = 0;
	}
#endif

	ptUartArea->ulUartcr = 0;
	ptUartArea->ulUartlcr_m = 0;
	ptUartArea->ulUartlcr_l = 0;
	ptUartArea->ulUartlcr_h = 0;
	ptUartArea->ulUartrts = 0;
	ptUartArea->ulUartdrvout = 0;
}


/*-------------------------------------------------------------------------*/


TEST_RESULT_T test(TEST_PARAMETER_T *ptTestParam)
{
	TEST_RESULT_T tTestResult;
	UARTTEST_PARAMETER_T *ptTestParams;
	UART_CONFIGURATION_T tCfg;
	int iResult;


	/* Be pessimistic. */
	tTestResult = TEST_RESULT_ERROR;

	systime_init();

	uprintf("\f. *** UART test by doc_bacardi@users.sourceforge.net ***\n");
	uprintf("V" VERSION_ALL "\n\n");

	/* Switch off SYS led. */
	rdy_run_setLEDs(RDYRUN_OFF);

	/* Get the test parameter. */
	ptTestParams = (UARTTEST_PARAMETER_T*)(ptTestParam->pvInitParams);

	uprintf(". verbose: 0x%08x\n", ptTestParams->ulVerboseLevel);
	uprintf(". unit: 0x%08x\n", ptTestParams->uiUnit);
	uprintf(". baud rate divider: 0x%08x\n", ptTestParams->ulBaudDivider);
	uprintf(". RXD: MMIO%d\n", ptTestParams->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RXD]);
	uprintf(". TXD: MMIO%d\n", ptTestParams->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_TXD]);
	uprintf(". CTS: MMIO%d\n", ptTestParams->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_CTS]);
	uprintf(". RTS: MMIO%d\n", ptTestParams->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RTS]);
	uprintf(". flags: 0x%08x\n", ptTestParams->ulFlags);
	uprintf("\n");
	
	tCfg.uiUnit = ptTestParams->uiUnit;
	tCfg.ulBaudDiv = ptTestParams->ulBaudDivider;
	tCfg.ptArea = NULL;
	memcpy(tCfg.aucMMIO, ptTestParams->aucMMIO, sizeof(tCfg.aucMMIO));
	tCfg.ulFlags = ptTestParams->ulFlags;

	iResult = 0;
	if( ((ptTestParams->ulFlags)&((unsigned long)UARTTEST_FLAGS_Use_CTS_RTS))!=0 )
	{
		iResult = rtscts_io_test(&tCfg);
		if( iResult!=0 )
		{
			uprintf("The RTS/CTS IO test failed!\n");
		}
	}

	if( iResult==0 )
	{
		iResult = uart_init(&tCfg);
		if( iResult!=0 )
		{
			uprintf("Failed to initialize the UART!\n");
		}
		else
		{
			iResult = uart_test(&tCfg);
		}
		uart_shutdown(&tCfg);
	}

	if( iResult==0 )
	{
		rdy_run_setLEDs(RDYRUN_GREEN);
		tTestResult = TEST_RESULT_OK;
	}
	else
	{
		rdy_run_setLEDs(RDYRUN_YELLOW);
	}

	return tTestResult;
}

/*-------------------------------------------------------------------------*/

