
#include "main_test.h"

#include "crc.h"
#include "netx_test.h"
#include "rdy_run.h"
#include "systime.h"
#include "uprintf.h"
#include "version.h"

/*-----------------------------------*/

TEST_RESULT_T test(TEST_PARAMETER_T *ptTestParam)
{
	TEST_RESULT_T tTestResult;
	CRCTEST_PARAMETER_T *ptTestParams;
	const unsigned char *pucCnt;
	const unsigned char *pucEnd;
	unsigned long ulCrc32;


	systime_init();

	uprintf("\f. *** CRC test by doc_bacardi@users.sourceforge.net ***\n");
	uprintf("V" VERSION_ALL "\n\n");

	/* Switch off SYS led. */
	rdy_run_setLEDs(RDYRUN_OFF);

	/* Get the test parameter. */
	ptTestParams = (CRCTEST_PARAMETER_T*)(ptTestParam->pvInitParams);

	/* Build the CRC for the requested area. */
	pucCnt = ptTestParams->pucAreaStart;
	pucEnd = ptTestParams->pucAreaEnd;

	uprintf(". area start: 0x%08x\n", ptTestParams->pucAreaStart);
	uprintf(". area end  : 0x%08x\n\n", ptTestParams->pucAreaEnd);
	
	uprintf("Calculating CRC32 ...\n");
	crc_init_crc32();
	while( pucCnt<pucEnd )
	{
		crc_update( *(pucCnt++) );
	}
	ulCrc32 = crc_get_crc32();
	ptTestParams->ulCrc32 = ulCrc32;
	uprintf("CRC32 = 0x%08x\n", ulCrc32);

	rdy_run_setLEDs(RDYRUN_GREEN);
	tTestResult = TEST_RESULT_OK;

	return tTestResult;
}

/*-----------------------------------*/

