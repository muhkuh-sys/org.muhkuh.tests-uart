
#include "main_test.h"

#include "netx_test.h"
#include "rdy_run.h"
#include "systime.h"
#include "uprintf.h"
#include "version.h"

/*-----------------------------------*/

TEST_RESULT_T test(TEST_PARAMETER_T *ptTestParam)
{
	TEST_RESULT_T tTestResult;
	UARTTEST_PARAMETER_T *ptTestParams;


	systime_init();

	uprintf("\f. *** UART test by doc_bacardi@users.sourceforge.net ***\n");
	uprintf("V" VERSION_ALL "\n\n");

	/* Switch off SYS led. */
	rdy_run_setLEDs(RDYRUN_OFF);

	/* Get the test parameter. */
	ptTestParams = (UARTTEST_PARAMETER_T*)(ptTestParam->pvInitParams);

	uprintf(". verbose: 0x%08x\n", ptTestParams->ulVerboseLevel);
	uprintf(". unit: 0x%08x\n", ptTestParams->uiUnit);
	uprintf(". MMIO: %d %d %d %d\n\n", ptTestParams->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RXD], ptTestParams->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_TXD], ptTestParams->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_CTS], ptTestParams->aucMMIO[UARTTEST_PARAMETER_MMIO_INDEX_RTS]);
	

	rdy_run_setLEDs(RDYRUN_GREEN);
	tTestResult = TEST_RESULT_OK;

	return tTestResult;
}

/*-----------------------------------*/

