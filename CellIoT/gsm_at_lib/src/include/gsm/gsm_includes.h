/**
 * \file            gsm_includes.h
 * \brief           All main includes
 */

/*
 * Copyright (c) 2019 Tilen MAJERLE
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge,
 * publish, distribute, sublicense, and/or sell copies of the Software,
 * and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE
 * AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 *
 * This file is part of GSM-AT library.
 *
 * Author:          Tilen MAJERLE <tilen@majerle.eu>
 * Version:         v0.6.0
 */
#ifndef GSM_HDR_INCLUDES_H
#define GSM_HDR_INCLUDES_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "gsm_config.h"
#include "gsm_typedefs.h"
#include "gsm_buff.h"
#include "gsm_input.h"
#include "gsm_debug.h"
#include "gsm_utils.h"
#include "gsm_pbuf.h"
#include "gsm_sim.h"
#include "gsm_operator.h"
#include "gsm_evt.h"
#include "gsm_network.h"
#include "gsm_device_info.h"
#include "gsm_sys.h"

#if GSM_CFG_SMS || __DOXYGEN__
#include "gsm_sms.h"
#endif /* GSM_CFG_SMS || __DOXYGEN__ */
#if GSM_CFG_CALL || __DOXYGEN__
#include "gsm_call.h"
#endif /* GSM_CFG_CALL || __DOXYGEN__ */
#if GSM_CFG_PHONEBOOK || __DOXYGEN__
#include "gsm_phonebook.h"
#endif /* GSM_CFG_PHONEBOOK || __DOXYGEN__ */
#if GSM_CFG_CONN || __DOXYGEN__
#include "gsm_conn.h"
#endif /* GSM_CFG_CONN || __DOXYGEN__ */
#if GSM_CFG_NETCONN || __DOXYGEN__
#include "gsm_netconn.h"
#endif /* GSM_CFG_NETCONN || __DOXYGEN__ */
#if GSM_CFG_USSD || __DOXYGEN__
#include "gsm_ussd.h"
#endif /* GSM_CFG_USSD || __DOXYGEN__ */
#if GSM_SEQUANS_SPECIFIC_CMD || __DOXYGEN__
#include "CellIoT_types.h"
#endif /* GSM_SEQUANS_SPECIFIC_CMD || __DOXYGEN__ */


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GSM_HDR_INCLUDES_H */
