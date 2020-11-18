/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CELLIOT_LIB_H_
#define CELLIOT_LIB_H_

#include "gsm_typedefs.h"
#include "gsm_private.h"
#include "fsl_dma.h"
#include "fsl_usart.h"
#include "fsl_usart_dma.h"
#include "fsl_usart_freertos.h"

#define BUFFER_POLLS_NB	11U
/* Max possible amount of data coming from the module is 1500 bytes
 * but since a byte is sent via 2 ASCII characters, 1500 bytes
 * will represented by 3000 ASCII characters
 */
#define CELLULAR_BUFFER_SIZE 1500U * 2U

#define AT_BUFFER_SIZE (0x400U)

typedef struct ST_RXDATAPENDING_TAG
{
	uint32_t BytesPending;					/*!< Number of Bytes pending to be read by the Application */
	uint32_t connid;						/*!< Connection ID of the received message */
	char RxBuffer[CELLULAR_BUFFER_SIZE];	/*!< RX Buffer poll */
	char * ptr_start;						/*!< Pointer of the next character to read */
	char * ptr_end;							/*!< Pointer of the last character to read */
} st_RXData;

extern st_RXData sRXData[GSM_CFG_MAX_CONNS][BUFFER_POLLS_NB];
extern uint8_t u8_nextFreeBufferPool;
extern uint8_t u8_nextBufferPoolForData;

extern uint8_t g_txBuffer[AT_BUFFER_SIZE];
extern uint8_t g_rxBuffer_1[AT_BUFFER_SIZE];
extern uint8_t g_rxBuffer_2[AT_BUFFER_SIZE];

/**********************************************************************************:
 *    Non-Volatile Memory
 **********************************************************************************/
void DMA_Callback(dma_handle_t *handle, void *param, bool transferDone, uint32_t tcds);
void Timer_CallbackHandler( uint32_t flags );
gsmr_t CellIoT_lib_WriteCertKeyInNVM(const char * certkey, SQNS_MQTT_CERTORKEY type, uint8_t index, size_t certkeysize, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
bool CellIoT_lib_ReadCertKeyInNVM(SQNS_MQTT_CERTORKEY type, uint8_t index);
bool CellIoT_lib_DeleteCertKeyInNVM(SQNS_MQTT_CERTORKEY type, uint8_t index);

gsmr_t CellIoT_lib_getHostIP(const char * hostName, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t CellIoT_lib_socketDial(uint8_t connId, uint8_t txProt, uint16_t rHostPort, const char* ip, uint8_t closureType, uint8_t lPort, uint8_t connMode, uint8_t acceptAnyRemote, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t CellIoT_lib_socketSend( uint8_t connId, const unsigned char * pTX , uint32_t sTx );
gsmr_t CellIoT_lib_socketRecv( uint8_t connId, uint32_t bytes_pending );
uint32_t CellIoT_lib_socketReadData( uint8_t connId, unsigned char * pRX , uint16_t rcvlen );
gsmr_t CellIoT_lib_setSocketSecurity(uint8_t spId, uint8_t connId, uint8_t enable, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t CellIoT_lib_setTLSSecurityProfileCfg(	uint8_t spId,
									uint8_t version,
									const char * cipherSpecs,
									uint8_t certValidLevel,
									uint8_t caCertificateID,
									uint8_t clientCertificateID,
									uint8_t clientPrivateKeyID,
									const char * psk,
									const gsm_api_cmd_evt_fn evt_fn,
									void* const evt_arg,
									const uint32_t blocking);
gsmr_t CellIoT_lib_setSocketCfgExt(uint8_t connId, uint8_t srMode, uint8_t recvDataMode, uint8_t keepalive, uint8_t listenAutoRsp, uint8_t sendDataMode, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t CellIoT_lib_setSocketCfg(uint8_t connId, uint8_t cid, uint16_t pktsize, uint16_t maxto, uint32_t connto , uint32_t txTo , const uint32_t blocking);
gsmr_t CellIoT_lib_socketClose( uint32_t connId);
gsmr_t CellIoT_lib_setLogInModule(void);
gsmr_t CellIoT_lib_readConfTestMode(void);
gsmr_t CellIoT_lib_setConfTestMode(const char * ctm, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);
gsmr_t CellIoT_lib_readBandSelection(void);
gsmr_t CellIoT_lib_setBandSelection(uint8_t id, const char * band, const char * band_number, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking);

#endif /* CELLIOT_LIB_H_ */
