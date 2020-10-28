/**
 * \file            gsm.h
 * \brief           GSM AT commands parser
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
#ifndef GSM_HDR_H
#define GSM_HDR_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Get most important include files */
#include "gsm_includes.h"

/**
 * \defgroup        GSM GSM-AT Lib
 * \brief           GSM stack
 * \{
 */

gsmr_t      gsm_init(gsm_evt_fn evt_func, const uint32_t blocking);
gsmr_t      gsm_reset(const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t      gsm_reset_with_delay(uint32_t delay, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

gsmr_t      gsm_set_func_mode(uint8_t mode, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

gsmr_t      gsm_core_lock(void);
gsmr_t      gsm_core_unlock(void);

gsmr_t      gsm_device_set_present(uint8_t present, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
uint8_t     gsm_device_is_present(void);

uint8_t     gsm_delay(uint32_t ms);

/**
 * \}
 */

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* GSM_HDR_H */
