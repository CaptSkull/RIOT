/*
 * Copyright (C) 2017 Unwired Devices [info@unwds.com]
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @defgroup
 * @ingroup
 * @brief
 * @{
 * @file	umdk-dali.c
 * @brief       umdk-dali module implementation
 * @author      Mikhail Perkov

 */

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#include "board.h"
#include "periph/gpio.h"

#include "unwds-common.h"
#include "include/umdk-dali.h"

#include "thread.h"
#include "xtimer.h"

static uwnds_cb_t *callback;

static gpio_t dali_chs[UMDK_DALI_NUM_CHANNELS] = { UMDK_DALI_1, UMDK_DALI_2, UMDK_DALI_3, UMDK_DALI_4 };

static uint32_t pack_dali = 0;

static void umdk_dali_transmit(uint32_t dali_pack, gpio_t pin)
{
  /* Sending DALI pack(19 bits) */
  for(int i = UMDK_DALI_LENGTH_PACK - 1; i >= 0; i--)  {
      if(_CHKBIT(dali_pack, i)) {		/* If value of bit = 1 */
	  /* Set  01 (1 in manchester code) */
	  gpio_clear(pin);
	  xtimer_usleep(UMDK_DALI_TIME_MANCHESTER_USEC);
	  gpio_set(pin);
	  xtimer_usleep(UMDK_DALI_TIME_MANCHESTER_USEC);
      }
      else {					/* If value of bit = 0 */
	  /* Set  10 (0 in manchester code) */
	  gpio_set(pin);
	  xtimer_usleep(UMDK_DALI_TIME_MANCHESTER_USEC);
	  gpio_clear(pin);
	  xtimer_usleep(UMDK_DALI_TIME_MANCHESTER_USEC);
      }
  }
}

void umdk_dali_init(uint32_t *non_gpio_pin_map, uwnds_cb_t *event_callback)
{
    (void)non_gpio_pin_map;

    callback = event_callback;
    for(int i = 0; i < UMDK_DALI_NUM_CHANNELS; i++) {
	    gpio_init(dali_chs[i], GPIO_OUT);
    }
}


bool umdk_dali_cmd(module_data_t *cmd, module_data_t *reply)
{
    /* Check minimum command length */
    if (cmd->length < 2) {
	printf("[umdk-dali] Invalid command - wrong length of command\n");
        return false;
    }

    /* Check on the address type */
    /* if it isn't number of group or  isn't broadcast  */
    if((cmd->data[0] >> 7) == 1){
	if(( (cmd->data[0] >> 5) & 0x03) != 0) {
	    /* if it isn't broadcast  */
	    if((cmd->data[0] >> 1) != 0x7F) {
		printf("[umdk-dali] Invalid broadcast command\n");
		return false;
	    }
	    printf("[umdk-dali] Invalid type of address\n");
	    return false;
	}
    }

    /* Pack DALI is 19 bits */
    pack_dali = ((1 << 18) + (cmd->data[0] << 10) + (cmd->data[1] << 2) + (0 << 1) + (0 << 0)) & 0x0007FFFF;

    /* TODO: Now only for one channel */
    umdk_dali_transmit(pack_dali, dali_chs[0]);

    reply->length = 4;
    reply->data[0] = UNWDS_DALI_MODULE_ID;
    reply->data[1] = 'o';
    reply->data[2] = 'k';
    reply->data[3] = '\0';

    return true; /* Allow reply */

}


#ifdef __cplusplus
}
#endif
