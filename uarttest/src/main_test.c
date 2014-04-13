
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

	uprintf(". unit: 0x%08x\n", ptTestParams->uiUnit);
	uprintf(". MMIO: %d %d %d %d\n\n", ptTestParams->aucMMIO[0], ptTestParams->aucMMIO[1], ptTestParams->aucMMIO[2], ptTestParams->aucMMIO[3]);
	

	rdy_run_setLEDs(RDYRUN_GREEN);
	tTestResult = TEST_RESULT_OK;

	return tTestResult;
}

/*-----------------------------------*/

