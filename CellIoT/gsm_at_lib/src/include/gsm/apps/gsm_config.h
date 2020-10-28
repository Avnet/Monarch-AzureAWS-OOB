/**
 * \file            gsm_config.h
 * \brief           Config file
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
#ifndef GSM_HDR_CONFIG_H
#define GSM_HDR_CONFIG_H

/* Rename this file to "gsm_config.h" for your application */

/*
 * Open "include gsm_config_default.h" and
 * copy & replace here settings you want to change values
 */
#define GSM_SEQUANS_SPECIFIC_CMD			1
#define GSM_CFG_AT_ECHO                     0
#define GSM_CFG_SYS_PORT                    GSM_SYS_PORT_USER
#define GSM_CFG_INPUT_USE_PROCESS           1	/* Need to be enabled when DMA support will be enabled */
#define GSM_CFG_AT_PORT_BAUDRATE_115200     115200U
#define GSM_CFG_AT_PORT_BAUDRATE_921600     921600U
#define GSM_CFG_AT_PORT_BAUDRATE			GSM_CFG_AT_PORT_BAUDRATE_115200


/* Enable network, conn and netconn APIs */
#define GSM_CFG_NETWORK                     1
#define GSM_CFG_CONN                        1
#define GSM_CFG_NETCONN                     1

#define SERIAL_DEBUG						1

/* After user configuration, call default config to merge config together */
#include "gsm_config_default.h"

#endif /* GSM_HDR_CONFIG_H */
