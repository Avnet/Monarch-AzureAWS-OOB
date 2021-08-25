/**	
 * \file            gsm_http.c
 * \brief           HTTP API
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
#include "gsm_private.h"
#include "gsm_http.h"
#include "gsm_mem.h"

#if GSM_CFG_HTTP || __DOXYGEN__

#endif /* GSM_CFG_HTTP || __DOXYGEN__ */

void gsm_http_response_reset(void) {
	gsm_core_lock();
	gsm.m.http_response.prof_id = 0;
	gsm.m.http_response.status_code = -1;
	gsm.m.http_response.type[0] = '\0';
	gsm.m.http_response.data_size = 0;
	gsm.m.http_response.data[0] = '\0';
	gsm.m.http_response.is_valid = 0;
	gsm_core_unlock();
}

uint8_t gsm_http_response_is_ready(void) {
	uint8_t is_valid;
	gsm_core_lock();
	is_valid = gsm.m.http_response.status_code != -1;
	gsm_core_unlock();
	return is_valid;
}

uint8_t gsm_http_response_is_valid(void) {
	uint8_t is_valid;
	gsm_core_lock();
	is_valid = gsm.m.http_response.is_valid;
	gsm_core_unlock();
	return is_valid;
}

void gsm_http_response_get(gsm_http_response_t *response) {
	gsm_core_lock();
	*response = gsm.m.http_response;
	gsm_core_unlock();
}
