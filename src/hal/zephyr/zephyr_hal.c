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
 * @file zephyr_hal.c
 * @author Benjamin Lemouzy, Centralp
 * @date 2024-05-17
 * @brief Hardware abstraction layer for the CCID serial driver, Zephyr implementation
 *
 * @copyright Copyright (c) Centralp, France, 2024
 *
 * @addtogroup ccid
 */

#include <project.h>

#include "../../pcsc-serial.h"
#include "../../ccid/ccid_hal.h"
#include "../../scard/scard_errors.h"

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>

#define CCID_DEVICE_NODE DT_ALIAS(ccid0)
static const struct device *const uart_dev = DEVICE_DT_GET(CCID_DEVICE_NODE);
static BOOL uart_dev_open = FALSE;

#define WAKEUP_EVENT_MSG_AVAILABLE BIT(0)
K_EVENT_DEFINE(wakeup_event);

void serial_cb(const struct device *dev, void *user_data)
{
	BYTE bValue;

	if (!uart_irq_update(dev)) {
		return;
	}

	if (!uart_irq_rx_ready(dev)) {
		return;
	}

	while (uart_fifo_read(dev, &bValue, 1) == 1) {
		CCID_LIB(SerialRecvByteFromISR)(bValue);
	}
}

/**
 * @brief Prepare the serial library
 * @note This function may have a different prototype, depending on the OS/target requirements
 */
void CCID_LIB(SerialInit)(const char* szCommName)
{

}

/**
 * @brief Open the serial comm port and activate the RX interrupt
 * @note This function must be implemented specifically for the OS/target
 */
BOOL CCID_LIB(SerialOpen)(void)
{
	if (uart_dev == NULL) {
		return FALSE;
	}

	if (!device_is_ready(uart_dev)) {
		return FALSE;
	}

	uart_dev_open = TRUE;

	uart_irq_callback_user_data_set(uart_dev, serial_cb, NULL);
	uart_irq_rx_enable(uart_dev);

	return TRUE;
}

/**
 * @brief Close the serial comm port
 * @note This function must be implemented specifically for the OS/target
 */
void CCID_LIB(SerialClose)(void)
{
	uart_irq_rx_disable(uart_dev);

	uart_dev_open = FALSE;
}

/**
 * @brief Returns TRUE if the serial comm port is open, FALSE otherwise
 * @note This function must be implemented specifically for the OS/target
 */
BOOL CCID_LIB(SerialIsOpen)(void)
{
	return uart_dev_open;
}

/**
 * @brief Send one byte to the device through the UART
 * @note This function must be implemented specifically for the OS/target. It is acceptable to block the caller.
 */
BOOL CCID_LIB(SerialSendByte)(BYTE bValue)
{
	uart_poll_out(uart_dev, bValue);

	return TRUE;
}

/**
 * @brief Send a buffer to the device through the UART
 * @note This function must be implemented specifically for the OS/target. It is acceptable to block the caller.
 */
BOOL CCID_LIB(SerialSendBytes)(const BYTE* abValue, DWORD dwLength)
{
	for (DWORD i = 0; i < dwLength; i++) {
		if (!CCID_LIB(SerialSendByte)(abValue[i])) {
			return FALSE;
		}
	}

	return TRUE;
}

/**
 * @brief Notify the task/thread waiting over CCID_WaitWakeup that a message is available
 * @note This function must be implemented specifically for the OS/target
 */
void CCID_LIB(WakeupFromISR)(void)
{
	k_event_set(&wakeup_event, WAKEUP_EVENT_MSG_AVAILABLE);
}

/**
 * @brief Start waiting for a message
 * @note This function must be implemented specifically for the OS/target
 */
void CCID_LIB(ClearWakeup)(void)
{
	k_event_clear(&wakeup_event, WAKEUP_EVENT_MSG_AVAILABLE);
}

/**
 * @brief Wait until a message is available or a timeout occurs
 * @note This function must be implemented specifically for the OS/target.
 */
BOOL CCID_LIB(WaitWakeup)(DWORD timeout_ms)
{
	return k_event_wait(&wakeup_event, WAKEUP_EVENT_MSG_AVAILABLE, false, K_MSEC(timeout_ms)) != 0;
}
