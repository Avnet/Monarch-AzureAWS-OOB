/**	
 * \file            gsm_sms.h
 * \brief           SMS API
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
#ifndef GSM_HDR_SMS_H
#define GSM_HDR_SMS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "gsm.h"

/**
 * \ingroup         GSM
 * \defgroup        GSM_SMS SMS API
 * \brief           SMS manager
 * \{
 */

gsmr_t      gsm_sms_enable(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t      gsm_sms_disable(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

gsmr_t      gsm_sms_send(const char* num, const char* text, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t      gsm_sms_read(gsm_mem_t mem, size_t pos, gsm_sms_entry_t* entry, uint8_t update, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t      gsm_sms_delete(gsm_mem_t mem, size_t pos, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t      gsm_sms_delete_all(gsm_sms_status_t status, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t      gsm_sms_list(gsm_mem_t mem, gsm_sms_status_t stat, gsm_sms_entry_t* entries, size_t etr, size_t* er, uint8_t update, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t      gsm_sms_set_preferred_storage(gsm_mem_t mem1, gsm_mem_t mem2, gsm_mem_t mem3, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif

#endif /* GSM_HDR_SMS_H */
