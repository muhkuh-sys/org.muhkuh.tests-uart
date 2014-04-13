
#ifndef __MAIN_TEST_H__
#define __MAIN_TEST_H__



typedef enum UARTTEST_PARAMETER_MMIO_INDEX_ENUM
{
	UARTTEST_PARAMETER_MMIO_INDEX_RXD = 0,
	UARTTEST_PARAMETER_MMIO_INDEX_TXD = 1,
	UARTTEST_PARAMETER_MMIO_INDEX_CTS = 2,
	UARTTEST_PARAMETER_MMIO_INDEX_RTS = 3
} UARTTEST_PARAMETER_MMIO_INDEX_T;


typedef enum UARTTEST_FLAGS_ENUM
{
	UARTTEST_FLAGS_Use_CTS_RTS = 1
} UARTTEST_FLAGS_T;


typedef struct UARTTEST_PARAMETER_STRUCT
{
	unsigned long ulVerboseLevel;
	unsigned int uiUnit;
	unsigned long ulBaudDivider;
	unsigned char aucMMIO[4];
	unsigned long ulFlags;
} UARTTEST_PARAMETER_T;



#endif  /* __MAIN_TEST_H__ */

