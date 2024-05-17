/*

 This code is Copyright (c) 2015-2023 SPRINGCARD SAS, FRANCE - www.springcard.com

 Redistribution and use in source (source code) and binary (object code)
  forms, with or without modification, are permitted provided that the
  following conditions are met :
  1. Redistributed source code or object code must be used only in conjunction
	 with a genuine SpringCard product,
  2. Redistributed source code must retain the above copyright notice, this
	 list of conditions and the disclaimer below,
  3. Redistributed object code must reproduce the above copyright notice,
	 this list of conditions and the disclaimer below in the documentation
	 and/or other materials provided with the distribution,
  4. The name of SpringCard may not be used to endorse or promote products
	 derived from this software or in any other form without specific prior
	 written permission from SpringCard,
  5. Redistribution of any modified code must be labeled
	"Code derived from original SpringCard copyrighted source code".

  THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
  ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED
  TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
  PARTICULAR PURPOSE.
  SPRINGCARD SHALL NOT BE LIABLE FOR INFRINGEMENTS OF THIRD PARTIES RIGHTS
  BASED ON THIS SOFTWARE. ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
  PARTICULAR PURPOSE ARE DISCLAIMED. SPRINGCARD DOES NOT WARRANT THAT THE
  FUNCTIONS CONTAINED IN THIS SOFTWARE WILL MEET THE USER'S REQUIREMENTS OR
  THAT THE OPERATION OF IT WILL BE UNINTERRUPTED OR ERROR-FREE. IN NO EVENT,
  UNLESS REQUIRED BY APPLICABLE LAW, SHALL SPRINGCARD BE LIABLE FOR ANY DIRECT,
  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
  BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
  OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
  NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
  EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE. ALSO, SPRINGCARD IS UNDER
  NO OBLIGATION TO MAINTAIN, CORRECT, UPDATE, CHANGE, MODIFY, OR OTHERWISE
  SUPPORT THIS SOFTWARE.
*/

/**
 * @file pcsc-serial-sample-main-zephyr.c
 * @author Benjamin Lemouzy, Centralp
 * @date 2024-05-17
 * @brief Source code of the sample application (main function of the CLI program for Zephyr)
 *
 * @copyright Copyright (c) Centralp, France, 2024
 */

/**
 * @addtogroup sample
 */

#include <project.h>

#include "../../pcsc-serial.h"
#include "../../scard/scard.h"
#include "../../ccid/ccid.h"
#include "../../ccid/ccid_hal.h"

void sample(void);

int main(void)
{
	LONG rc;

	printf("\n");
	printf("SpringCard SDK for PC/SC Serial : demo for Zephyr\n");
	printf("------------------------------------------------------------\n");
	printf("Copyright (c) 2015-2023 SPRINGCARD SAS, FRANCE - www.springcard.com\n");
	printf("See LICENSE.txt for disclaimer and license requirements\n");
	printf("\n");

	fCcidUseNotifications = TRUE;
	fTestEchoControl = FALSE;
	fTestEchoTransmit = FALSE;
	fVerbose = TRUE;

	/* Prepare the underlying hardware and lower layer software */
	/* -------------------------------------------------------- */

	printf("\n\n***** Now running forever ****\n\n");

	for (;;)
	{
		/* Try and open the serial communication layer */
		/* ------------------------------------------- */
		printf("Opening the serial port\n");
		if (!CCID_LIB(SerialOpen)())
		{
			printf("Failed to open serial port\n");
			k_sleep(K_MSEC(1000));
			continue;
		}

		/* Initialize the CCID driver */
		/* -------------------------- */

		printf("Initializing the CCID driver\n");
		CCID_LIB(Init)();

		/* Connect or reconnect to the device */
		/* ---------------------------------- */

		printf("Pinging the device (if some)\n");
		rc = CCID_LIB(Ping)();
		if (rc != SCARD_ERR(S_SUCCESS))
		{
			printf("No device found (rc=%lX)\n", rc);
			/* Close the serial port */
			CCID_LIB(SerialClose)();
			/* In case of a communication error, we shall wait at least 1200ms so the device may reset its state machine */
			k_sleep(K_MSEC(1200));
			/* Try again */
			continue;
		}

		printf("*** It seems that we've found a device... ***\n");

		/* Initialize the PC/SC-like stack */
		/* ------------------------------- */

		printf("Initializing the PC/SC-Like stack\n");
		SCARD_LIB(Init)();

		/* Run the sample */
		/* --------------- */

		sample();

		/* The sample is a loop, if we come back here, this means that we have lost the device */
		/* ----------------------------------------------------------------------------------- */
		printf("*** It seems that we've lost the device... ***\n");

		/* Close the serial port */
		printf("Closing the serial port\n");
		CCID_LIB(SerialClose)();
	}

	return 0;
}

BOOL SCARD_LIB(IsCancelledHook)(void)
{
	/* Dummy function */
	return FALSE;
}
