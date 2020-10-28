/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#include "CellIoT_lib.h"
#include "CellIoT_types.h"
#include "aws_clientcredential.h"

#ifdef DEBUG_NAMING
#include "fsl_debug_console.h"
#endif

uint8_t g_txBuffer[AT_BUFFER_SIZE] = {0};
uint8_t g_rxBuffer_1[AT_BUFFER_SIZE] = {0};
uint8_t g_rxBuffer_2[AT_BUFFER_SIZE] = {0};

st_RXData sRXData[GSM_CFG_MAX_CONNS][BUFFER_POLLS_NB] = {
											{
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][0].RxBuffer ,
													.ptr_end = sRXData[0][0].RxBuffer
												},
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][1].RxBuffer ,
													.ptr_end = sRXData[0][1].RxBuffer
												},
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][2].RxBuffer ,
													.ptr_end = sRXData[0][2].RxBuffer
												},
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][3].RxBuffer ,
													.ptr_end = sRXData[0][3].RxBuffer
												},
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][4].RxBuffer ,
													.ptr_end = sRXData[0][4].RxBuffer
												},
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][5].RxBuffer ,
													.ptr_end = sRXData[0][5].RxBuffer
												},
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][6].RxBuffer ,
													.ptr_end = sRXData[0][6].RxBuffer
												},
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][7].RxBuffer ,
													.ptr_end = sRXData[0][7].RxBuffer
												},
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][8].RxBuffer ,
													.ptr_end = sRXData[0][8].RxBuffer
												},
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][9].RxBuffer ,
													.ptr_end = sRXData[0][9].RxBuffer
												},
												{	.BytesPending = 0,
													.connid = 0,
													.RxBuffer = {0},
													.ptr_start = sRXData[0][10].RxBuffer ,
													.ptr_end = sRXData[0][10].RxBuffer
												},
//												{	.waitingForData = 0,
//													.BytesPending = 0,
//													.connid = 0,
//													.RxBuffer = {0},
//													.ptr_start = sRXData[0][11].RxBuffer ,
//													.ptr_end = sRXData[0][11].RxBuffer
//												},
//												{	.waitingForData = 0,
//													.BytesPending = 0,
//													.connid = 0,
//													.RxBuffer = {0},
//													.ptr_start = sRXData[0][12].RxBuffer ,
//													.ptr_end = sRXData[0][12].RxBuffer
//												},
//												{	.waitingForData = 0,
//													.BytesPending = 0,
//													.connid = 0,
//													.RxBuffer = {0},
//													.ptr_start = sRXData[0][13].RxBuffer ,
//													.ptr_end = sRXData[0][13].RxBuffer
//												},
//												{	.waitingForData = 0,
//													.BytesPending = 0,
//													.connid = 0,
//													.RxBuffer = {0},
//													.ptr_start = sRXData[0][14].RxBuffer ,
//													.ptr_end = sRXData[0][14].RxBuffer
//												},
//												{	.waitingForData = 0,
//													.BytesPending = 0,
//													.connid = 0,
//													.RxBuffer = {0},
//													.ptr_start = sRXData[0][15].RxBuffer ,
//													.ptr_end = sRXData[0][15].RxBuffer
//												},
//												{	.waitingForData = 0,
//													.BytesPending = 0,
//													.connid = 0,
//													.RxBuffer = {0},
//													.ptr_start = sRXData[0][16].RxBuffer ,
//													.ptr_end = sRXData[0][16].RxBuffer
//												},
//												{	.waitingForData = 0,
//													.BytesPending = 0,
//													.connid = 0,
//													.RxBuffer = {0},
//													.ptr_start = sRXData[0][17].RxBuffer ,
//													.ptr_end = sRXData[0][17].RxBuffer
//												},
//												{	.waitingForData = 0,
//													.BytesPending = 0,
//													.connid = 0,
//													.RxBuffer = {0},
//													.ptr_start = sRXData[0][18].RxBuffer ,
//													.ptr_end = sRXData[0][18].RxBuffer
//												},
//												{	.waitingForData = 0,
//													.BytesPending = 0,
//													.connid = 0,
//													.RxBuffer = {0},
//													.ptr_start = sRXData[0][19].RxBuffer ,
//													.ptr_end = sRXData[0][19].RxBuffer
//												}

											}
};

uint8_t u8_nextFreeBufferPool = 0;				/*!< Variable that hold the next buffer structure where data can be stored */
uint8_t u8_nextBufferPoolForData = 0;			/*!< Variable that hold the 1st buffer structure where data is available to read */

/**
 * \brief           Write a Certificate or a private Key in Non-Volatile Memory
 * \param[in]       certkey: Access to the certificate or private key
 * \param[in]       type: Specify if certificate or private key will be used
 * \param[in]       index: Memory slot
 * \param[in]       certkeysize: Size of the certificate or private key
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_WriteCertKeyInNVM(const char * certkey, SQNS_MQTT_CERTORKEY type, uint8_t index, size_t certkeysize, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);

    //GSM_ASSERT("certkey != NULL", certkey != NULL);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNSNVW_W;
    GSM_MSG_VAR_REF(msg).msg.modem_memory.cert_key_ptr = certkey;
    GSM_MSG_VAR_REF(msg).msg.modem_memory.data_type = type;
    GSM_MSG_VAR_REF(msg).msg.modem_memory.index = index;
    GSM_MSG_VAR_REF(msg).msg.modem_memory.certkeysize = certkeysize;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 30000);
}

/**
 * \brief           Query to DNS server to resolve the host name into an IP address
 * \param[in]       hostName: URL Host name
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_getHostIP(const char * hostName, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_ASSERT("hostName != NULL", hostName != NULL);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNDNSLKUP;
    GSM_MSG_VAR_REF(msg).msg.host_ip_config.hostName = hostName;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 30000);
}

/**
 * \brief           Opens a remote connection via socket
 * \param[in]       connId: Connection ID, must be between 1 and GSM_CFG_MAX_CONNS
 * \param[in]       txProt: Specify if certificate or private key will be used
 * \param[in]       rHostPort: Memory slot
 * \param[in]       ip: Host IP Address
 * \param[in]       closureType: Socket closure behaviour for TCP, has no effect for UDP connections
 * 						0: Local host closes immediately when remote host has closed (default)
 * 						255: Local host closes after an escape sequence (+++)
 * \param[in]       lPort: UDP connection local port, has no effect for TCP connections.
 * \param[in]       connMode: Connection mode
 * 						0: Online mode connection (default)
 * 						1: Command mode connection
 * \param[in]       acceptAnyRemote: Size of the certificate or private key
 * 						0: Disabled (default)
 * 						1: Enabled
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_socketDial(	uint8_t connId,
						uint8_t txProt,
						uint16_t rHostPort,
						const char* ip,
						uint8_t closureType,
						uint8_t lPort,
						uint8_t connMode,
						uint8_t acceptAnyRemote,
						const gsm_api_cmd_evt_fn evt_fn,
						void* const evt_arg,
						const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_ASSERT("ip != NULL", ip != NULL);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNSD;
    GSM_MSG_VAR_REF(msg).msg.socket_dial.connId = connId;
    GSM_MSG_VAR_REF(msg).msg.socket_dial.txProt = txProt;
    GSM_MSG_VAR_REF(msg).msg.socket_dial.rHostPort = rHostPort;
    GSM_MSG_VAR_REF(msg).msg.socket_dial.ip = ip;
    GSM_MSG_VAR_REF(msg).msg.socket_dial.closureType = closureType;
    GSM_MSG_VAR_REF(msg).msg.socket_dial.lPort = lPort;
    GSM_MSG_VAR_REF(msg).msg.socket_dial.connMode = connMode;
    GSM_MSG_VAR_REF(msg).msg.socket_dial.acceptAnyRemote = acceptAnyRemote;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}

/**
 * \brief           Send data over a socket which was established before
 * \param[in]       connId: Connection ID, must be between 1 and GSM_CFG_MAX_CONNS
 * \param[in]       pointer to the data to be sent
 * \param[in]       number of bytes to be sent
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_socketSend( uint8_t connId, const unsigned char * pTX , uint32_t sTx )
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, 1);// blocking
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNSSENDEXT;
    GSM_MSG_VAR_REF(msg).msg.tx_data.connId = connId;
    GSM_MSG_VAR_REF(msg).msg.tx_data.ptrTx = pTX;
    GSM_MSG_VAR_REF(msg).msg.tx_data.Txsize = sTx;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}

/**
 * \brief           Receive data
 * \param[in]       connId: Connection ID, must be between 1 and GSM_CFG_MAX_CONNS
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_socketRecv( uint8_t connId, uint32_t bytes_pending )
{
	GSM_MSG_VAR_DEFINE(msg);

	GSM_MSG_VAR_ALLOC(msg, 1);// blocking
	GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNS_RECV;
	GSM_MSG_VAR_REF(msg).msg.rx_data.connId = connId;
	GSM_MSG_VAR_REF(msg).msg.rx_data.Rxsize = bytes_pending;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}


/**
 * \brief           Receive data
 * \param[in]       connId: Connection ID, must be between 1 and GSM_CFG_MAX_CONNS
 * \param[in]       pointer to the data to be read
 * \param[in]       number of bytes to be read
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
uint32_t
CellIoT_lib_socketReadData( uint8_t connId, unsigned char * pRX , uint16_t rcvlen )
{
	uint32_t ret = 0;

	/* If no bytes to read, just return 0 to simulate a time out */
	if( sRXData[connId - 1][u8_nextBufferPoolForData].BytesPending == 0 )
	{
		return ret;
	}

	if( rcvlen >= sRXData[connId - 1][u8_nextBufferPoolForData].BytesPending )
	{
		/* The Application wants more bytes than the buffer can hold
		 * Therefore read all the pending bytes by copying in the provided pointer
		 * Clear the buffer, decrease the number of pending reads to do
		 * and change the buffer buffer to then next one
		 */
		memcpy( pRX,
				sRXData[connId - 1][u8_nextBufferPoolForData].ptr_start,
				sRXData[connId - 1][u8_nextBufferPoolForData].BytesPending
				);

		ret = sRXData[connId - 1][u8_nextBufferPoolForData].BytesPending;

		sRXData[connId - 1][u8_nextBufferPoolForData].BytesPending = 0;
		sRXData[connId - 1][u8_nextBufferPoolForData].ptr_start = sRXData[connId - 1][u8_nextBufferPoolForData].RxBuffer;
		sRXData[connId - 1][u8_nextBufferPoolForData].ptr_end = sRXData[connId - 1][u8_nextBufferPoolForData].RxBuffer;

		u8_nextBufferPoolForData == BUFFER_POLLS_NB - 1U ? u8_nextBufferPoolForData = 0U : u8_nextBufferPoolForData++;
	}
	else
	{
		/* The Application wants only a part of the received message
		 * Therefore read the required bytes by copying in the provided pointer
		 * Reduce the number of pending bytes and update the next start pointer
		 * to the next character to read
		 */
		memcpy( pRX,
				sRXData[connId - 1][u8_nextBufferPoolForData].ptr_start,
				rcvlen
				);

		sRXData[connId - 1][u8_nextBufferPoolForData].BytesPending -= rcvlen;
		sRXData[connId - 1][u8_nextBufferPoolForData].ptr_start += rcvlen;

		ret = rcvlen;
	}

    return ret;
}


/**
 * \brief           Enables or disables the use of SSL/TLS connection on a TCP or UDP socket
 * \param[in]       spId: Security Profile ID (must be defined between 1 and 6)
 * \param[in]       connId: Connection ID, must be between 1 and GSM_CFG_MAX_CONNS
 * \param[in]       enable: Enable/Disable security on socket
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_setSocketSecurity(uint8_t spId, uint8_t connId, uint8_t enable, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNSSCFG;
    GSM_MSG_VAR_REF(msg).msg.secure_socket_cfg.connId = connId;
    GSM_MSG_VAR_REF(msg).msg.secure_socket_cfg.enable = enable;
    GSM_MSG_VAR_REF(msg).msg.secure_socket_cfg.spId = spId;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 30000);
}

/**
 * \brief           Sets the security profile parameters required to configure the following SSL/TLS connections properties
 * \param[in]       spId: Security Profile ID (must be defined between 1 and 6)
 * \param[in]       version: TLS Profile version
 * \param[in]       cipherSpecs: Exact list of cipher suite to be used, 8-bit hexadecimal "0x" prefixed IANA numbers, semicolon delimited
 * \param[in]       certValidLevel: Server certificate validation 8-bit field
 * \param[in]       caCertificateID: Trusted Certificate Authority certificate ID, integer in range [0-19]
 * \param[in]       clientCertificateID: Client certificate ID, integer in range [0-19]
 * \param[in]       clientPrivateKeyID: Client private key ID, integer in range [0-19]
 * \param[in]       psk: Pre-shared key used for connection (when a TLS_PSK_* cipher suite is used)
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_setTLSSecurityProfileCfg(	uint8_t spId,
										uint8_t version,
										const char * cipherSpecs,
										uint8_t certValidLevel,
										uint8_t caCertificateID,
										uint8_t clientCertificateID,
										uint8_t clientPrivateKeyID,
										const char * psk,
										const gsm_api_cmd_evt_fn evt_fn,
										void* const evt_arg,
										const uint32_t blocking)
{
	GSM_MSG_VAR_DEFINE(msg);

	GSM_MSG_VAR_ALLOC(msg, blocking);
	GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
	GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNSPCFG;
	GSM_MSG_VAR_REF(msg).msg.tls_security_profile_cfg.spId = spId;
	GSM_MSG_VAR_REF(msg).msg.tls_security_profile_cfg.version = version;
	GSM_MSG_VAR_REF(msg).msg.tls_security_profile_cfg.cipherSpecs = cipherSpecs;
	GSM_MSG_VAR_REF(msg).msg.tls_security_profile_cfg.certValidLevel = certValidLevel;
	GSM_MSG_VAR_REF(msg).msg.tls_security_profile_cfg.caCertificateID = caCertificateID;
	GSM_MSG_VAR_REF(msg).msg.tls_security_profile_cfg.clientCertificateID = clientCertificateID;
	GSM_MSG_VAR_REF(msg).msg.tls_security_profile_cfg.clientPrivateKeyID = clientPrivateKeyID;
	GSM_MSG_VAR_REF(msg).msg.tls_security_profile_cfg.psk = psk;

	return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 30000);
}

/**
 * \brief           Sets the socket configuration extended parameters
 * \param[in]       connId: Connection ID, must be between 1 and GSM_CFG_MAX_CONNS
 * \param[in]       srMode: SQNSRING URC mode
 * \param[in]       recvDataMode: Received data view mode presentation format [0:text format OR 1:hexadecimal format]
 * \param[in]       keepalive: Currently unused
 * \param[in]       listenAutoRsp: Listen auto-response mode�, that affects AT+SQNSL command
 * \param[in]       sendDataMode: Sent data view mode presentation format
 * \param[in]       evt_fn: Callback function called when command has finished. Set to `NULL` when not used
 * \param[in]       evt_arg: Custom argument for event callback function
 * \param[in]       blocking: Status whether command should be blocking or not
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_setSocketCfgExt(uint8_t connId, uint8_t srMode, uint8_t recvDataMode, uint8_t keepalive, uint8_t listenAutoRsp, uint8_t sendDataMode, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNSCFGEXT;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg_ext.connId = connId;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg_ext.srMode = srMode;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg_ext.recvDataMode = recvDataMode;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg_ext.keepalive = keepalive;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg_ext.listenAutoRsp = listenAutoRsp;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg_ext.sendDataMode = sendDataMode;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 30000);
}


/**
 * \brief           Sets the socket configuration extended parameters
 * \param[in]       connId: Connection ID, must be between 1 and GSM_CFG_MAX_CONNS
 * \param[in]       cid
 * \param[in]       pktsz
 * \param[in]       maxto
 * \param[in]       connto
 * \param[in]       txto
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_setSocketCfg(uint8_t connId, uint8_t cid, uint16_t pktsize, uint16_t maxto, uint32_t connto , uint32_t txTo , const uint32_t blocking)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, blocking);
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNSCFG;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg.connId = connId;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg.cid = cid;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg.pktSz = pktsize;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg.maxTo = maxto;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg.connTo = connto;
    GSM_MSG_VAR_REF(msg).msg.socket_cfg.txTo = txTo;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 30000);
}

/**
 * \brief           Close a socket connection
 * \param[in]       connId: Connection ID, must be between 1 and GSM_CFG_MAX_CONNS
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_socketClose(uint32_t connId)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, 1);// blocking
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNSH;
    GSM_MSG_VAR_REF(msg).msg.socket_dial.connId = connId;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}

gsmr_t
CellIoT_lib_setLogInModule(void)
{
	GSM_MSG_VAR_DEFINE(msg);

	GSM_MSG_VAR_ALLOC(msg, 1);
	GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNSLG;

	gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);

	GSM_MEMSET(msg, 0x00, sizeof(*(msg)));

	GSM_MSG_VAR_ALLOC(msg, 1);
	GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNPLG;
	return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);

	/* Disable HpPlmn Search */
	GSM_MEMSET(msg, 0x00, sizeof(*(msg)));

	GSM_MSG_VAR_ALLOC(msg, 1);
	GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNDISHPPLMN;

	return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}

/**
 * \brief           Read the current conformance test mode
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_readConfTestMode(void)
{
    GSM_MSG_VAR_DEFINE(msg);

    GSM_MSG_VAR_ALLOC(msg, 1);// blocking
    GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNCTM_GET;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}

/**
 * \brief           Set the current conformance test mode
 * \param[in]       ctm: Conformance Test Mode
 * \return          \ref gsmOK on success, member of \ref gsmr_t enumeration otherwise
 */
gsmr_t
CellIoT_lib_setConfTestMode(const char * ctm, const gsm_api_cmd_evt_fn evt_fn, void* const evt_arg, const uint32_t blocking)
{
	GSM_MSG_VAR_DEFINE(msg);

	GSM_ASSERT("ctm != NULL", ctm != NULL);

	GSM_MSG_VAR_ALLOC(msg, blocking);
	GSM_MSG_VAR_SET_EVT(msg, evt_fn, evt_arg);
	GSM_MSG_VAR_REF(msg).cmd_def = GSM_CMD_SQNCTM_SET;
	GSM_MSG_VAR_REF(msg).msg.set_conformance_test.ctm = ctm;

    return gsmi_send_msg_to_producer_mbox(&GSM_MSG_VAR_REF(msg), gsmi_initiate_cmd, 60000);
}


