/*
 * FreeRTOS Secure Sockets for NXP54018_IoT_Module V1.0.0 Beta 4
 * Copyright (C) 2017 Amazon.com, Inc. or its affiliates.  All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 *
 * http://aws.amazon.com/freertos
 * http://www.FreeRTOS.org
 */

/*
 * Copyright (C) NXP 2017-2019.
 */

/* Define _SECURE_SOCKETS_WRAPPER_NOT_REDEFINE to prevent secure sockets functions
 * from redefining in iot_secure_sockets_wrapper_metrics.h */
#define _SECURE_SOCKETS_WRAPPER_NOT_REDEFINE

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "iot_secure_sockets.h"
#include "iot_tls.h"
#include "task.h"
#include "timers.h"

/* Logging includes. */
//#include "iot_logging_task.h"

/* Third-party Cellular-IoT driver include. */
#include "gsm_includes.h"
#include "gsm_private.h"
#include "CellIoT_lib.h"
#undef _SECURE_SOCKETS_WRAPPER_NOT_REDEFINE

/**
 * @brief Flag indicating that socket send operations are not permitted.
 *
 * If a WR shutdown in SOCKETS_Shutdown() is invoked, this flag is
 * set in the socket's xShutdownFlags member.
 */
#define nxpsecuresocketsSOCKET_WRITE_CLOSED_FLAG    ( 1UL << 2 )

/**
 * @brief Flag indicating that socket receive operations are not permitted.
 *
 * If a RD shutdown in SOCKETS_Shutdown() is invoked, this flag is
 * set in the socket's xShutdownFlags member.
 */
#define nxpsecuresocketsSOCKET_READ_CLOSED_FLAG     ( 1UL << 1 )

/**
 * @brief Delay used between network select attempts when effecting a receive timeout.
 *
 * Timeouts are mocked in the secure sockets layer, and this constant sets the
 * sleep time between each read attempt during the receive timeout period.
 */
#define nxpsecuresocketsFIVE_MILLISECONDS           ( pdMS_TO_TICKS( 5 ) )

/**
 * @brief The timeout supplied to the t_select function.
 *
 * Receive timeout are emulated in secure sockets layer and therefore we
 * do not want the Inventek module to block. Setting to zero means
 * no timeout, so one is the smallest value we can set it to.
 */
#define nxpsecuresocketsONE_MILLISECOND             ( 1 )
#define nxpsecuresocketsSOCKET_CONNECTED_FLAG       ( 1 )

/* Internal context structure. */
typedef struct SSOCKETContext
{
    Socket_t xSocket;
    char * pcDestination;
    void * pvTLSContext;
    BaseType_t xRequireTLS;
    BaseType_t xSendFlags;
    BaseType_t xRecvFlags;
    BaseType_t xShutdownFlags;
    uint32_t ulSendTimeout;
    uint32_t ulRecvTimeout;
    char * pcServerCertificate;
    uint32_t ulServerCertificateLength;
    uint32_t ulState;
    char ** ppcAlpnProtocols;
    uint32_t ulAlpnProtocolsCount;
} SSOCKETContext_t, * SSOCKETContextPtr_t;

static uint8_t u8timeoutExpired;
TimerHandle_t xRcvTimer;

/*
 * Helper routines.
 */

/*
 * @brief Network send callback.
 */
static BaseType_t prvNetworkSend( void * pvContext,
                                  const unsigned char * pucData,
                                  size_t xDataLength )
{
    SSOCKETContextPtr_t pxContext = ( SSOCKETContextPtr_t ) pvContext;

    /* Do not send data on unconnected socket */
    if( !( pxContext->ulState & nxpsecuresocketsSOCKET_CONNECTED_FLAG ) )
    {
        return -1;
    }

#ifdef USE_WIFI
    char * sendBuf = custom_alloc( xDataLength );

    if( sendBuf == NULL )
    {
        return -1;
    }

    memcpy( sendBuf, pucData, xDataLength );
    int ret = qcom_send( ( int ) pxContext->xSocket,
                         sendBuf,
                         xDataLength,
                         pxContext->xSendFlags );
    custom_free( sendBuf );

    return ret;

#else
    /* Send Data using AT commands */
    if( CellIoT_lib_socketSend( gsm.m.conn_val_id , pucData , xDataLength ) == gsmOK )
    {
		#ifdef USE_AWS_CLOUD
    	/* AWS Cloud seems to need a delay here */
    	vTaskDelay(pdMS_TO_TICKS(200));
		#endif
    	return xDataLength;
    }
    return -1;
#endif

}

/*-----------------------------------------------------------*/

/*
 * @brief Network receive callback.
 */
static BaseType_t prvNetworkRecv( void * pvContext,
                                  unsigned char * pucReceiveBuffer,
                                  size_t xReceiveLength )
{
    SSOCKETContextPtr_t pxContext = ( SSOCKETContextPtr_t ) pvContext;
    int xRetVal = 0;

    /* Do not receive data on unconnected socket */
    if( !( pxContext->ulState & nxpsecuresocketsSOCKET_CONNECTED_FLAG ) )
    {
        return -1;
    }

    uint32_t i = 0;
    do
    {
    	xRetVal = CellIoT_lib_socketReadData( gsm.m.conn_val_id, pucReceiveBuffer , xReceiveLength );
    	if( 0 == xRetVal )
    		vTaskDelay(pdMS_TO_TICKS(1));
    }while( 0 == xRetVal && ++i < 2000);

    return xRetVal;
}

/*-----------------------------------------------------------*/

void vRcvTimerCallback( TimerHandle_t xTimer )
{
   /* Optionally do something if the pxTimer parameter is NULL. */
   configASSERT( xTimer );

   u8timeoutExpired = 1;

}

void xInitTimeoutNwkRecvTimer(uint32_t u32Timeout)
{
	xRcvTimer = xTimerCreate("Receive Timer",
							pdMS_TO_TICKS(u32Timeout),
							pdFALSE,
							( void * ) 0,
							vRcvTimerCallback );
}

/*
 * @brief Network receive callback.
 */
static BaseType_t prvTimeoutNetworkRecv( void * pvContext,
                                  	  	 unsigned char * pucReceiveBuffer,
										 size_t xReceiveLength,
										 uint32_t u32TimeoutPeriodInMs)
{
    SSOCKETContextPtr_t pxContext = ( SSOCKETContextPtr_t ) pvContext;
    int xRetVal = 0;

    /* Do not receive data on unconnected socket */
    if( !( pxContext->ulState & nxpsecuresocketsSOCKET_CONNECTED_FLAG ) )
    {
        return -1;
    }

    u8timeoutExpired = 0;
    xTimerStart( xRcvTimer, 0 );

    do
    {
    	xRetVal = CellIoT_lib_socketReadData( gsm.m.conn_val_id, pucReceiveBuffer , xReceiveLength );
    	if( 0 == xRetVal )
    		vTaskDelay(pdMS_TO_TICKS(1));
    }while( 0 == xRetVal && 0 == u8timeoutExpired);

    u8timeoutExpired = 0;

    return xRetVal;
}

/*-----------------------------------------------------------*/

/*
 * Interface routines.
 */

int32_t SOCKETS_Close( Socket_t xSocket )
{
    SSOCKETContextPtr_t pxContext = ( SSOCKETContextPtr_t ) xSocket;
    int32_t lStatus = SOCKETS_ERROR_NONE;

    if( ( NULL != pxContext ) && ( SOCKETS_INVALID_SOCKET != pxContext->xSocket ) )
    {
        pxContext->ulState = 0;

        if( NULL != pxContext->pcDestination )
        {
            vPortFree( pxContext->pcDestination );
        }

        if( NULL != pxContext->pcServerCertificate )
        {
            vPortFree( pxContext->pcServerCertificate );
        }

        if( pdTRUE == pxContext->xRequireTLS )
        {
            TLS_Cleanup( pxContext->pvTLSContext );
        }

        if( NULL != pxContext->ppcAlpnProtocols )
        {
            int i;
            for( i = 0; i < pxContext->ulAlpnProtocolsCount; i++ )
            {
                vPortFree( pxContext->ppcAlpnProtocols[i] );
            }
            vPortFree( pxContext->ppcAlpnProtocols );
            pxContext->ulAlpnProtocolsCount = 0;
        }


        // Close socket AT
        CellIoT_lib_socketClose( gsm.m.conn_val_id  );

        vPortFree( pxContext );
    }
    else
    {
        lStatus = SOCKETS_EINVAL;
    }

    return lStatus;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_Connect( Socket_t xSocket,
                         SocketsSockaddr_t * pxAddress,
                         Socklen_t xAddressLength )
{
    int32_t xStatus = SOCKETS_ERROR_NONE;
    SSOCKETContextPtr_t pxContext = ( SSOCKETContextPtr_t ) xSocket;
    TLSParams_t xTLSParams = { 0 };




	if( /*( SOCKETS_INVALID_SOCKET != pxContext ) &&*/ ( NULL != pxAddress ) &&
			(gsm_network_get_reg_status() == GSM_NETWORK_REG_STATUS_CONNECTED ||
			 gsm_network_get_reg_status() == GSM_NETWORK_REG_STATUS_CONNECTED_ROAMING) )
	{
        gsm_ip_t ip;

        ip.ip[0] = ( ( ( pxAddress->ulAddress ) >> 24 ) & 0xFF );
        ip.ip[1] = ( ( ( pxAddress->ulAddress ) >> 16 ) & 0xFF );
        ip.ip[2] = ( ( ( pxAddress->ulAddress ) >> 8  ) & 0xFF );
        ip.ip[3] = (   ( pxAddress->ulAddress )         & 0xFF );

		char ip_string[19];
		char temp_1[4];
		char temp_2[4];
		char temp_3[4];

		memset(ip_string, 0, sizeof(ip_string));
		memset(temp_1, 0, sizeof(temp_1));
		memset(temp_2, 0, sizeof(temp_2));
		memset(temp_3, 0, sizeof(temp_3));

		itoa(ip.ip[0],ip_string,10);
		itoa(ip.ip[1],temp_1,10);
		itoa(ip.ip[2],temp_2,10);
		itoa(ip.ip[3],temp_3,10);

		strcat(ip_string,".");
		strcat(ip_string,temp_1);
		strcat(ip_string,".");
		strcat(ip_string,temp_2);
		strcat(ip_string,".");
		strcat(ip_string,temp_3);

        xStatus = CellIoT_lib_socketDial(gsm.m.conn_val_id + 1, 0, pxAddress->usPort, ip_string, 0, 0, 1, 0, NULL, NULL, 1);



        /* Keep socket state - connected */
        if( SOCKETS_ERROR_NONE == xStatus )
        {
            pxContext->ulState |= nxpsecuresocketsSOCKET_CONNECTED_FLAG;
        }

        /* Negotiate TLS if requested. */
        if( ( SOCKETS_ERROR_NONE == xStatus ) && ( pdTRUE == pxContext->xRequireTLS ) )
        {
            xTLSParams.ulSize = sizeof( xTLSParams );
            xTLSParams.pcDestination = pxContext->pcDestination;
            xTLSParams.pcServerCertificate = pxContext->pcServerCertificate;
            xTLSParams.ulServerCertificateLength = pxContext->ulServerCertificateLength;
            xTLSParams.pvCallerContext = pxContext;
            xTLSParams.pxTimeoutNetworkRecv = prvTimeoutNetworkRecv;
            xTLSParams.pxNetworkRecv = prvNetworkRecv;
            xTLSParams.pxNetworkSend = prvNetworkSend;
            xTLSParams.ppcAlpnProtocols = ( const char ** ) pxContext->ppcAlpnProtocols;
            xTLSParams.ulAlpnProtocolsCount = pxContext->ulAlpnProtocolsCount;
            xStatus = TLS_Init( &pxContext->pvTLSContext, &xTLSParams );

            if( SOCKETS_ERROR_NONE == xStatus )
            {
                xStatus = TLS_Connect( pxContext->pvTLSContext );

                /* Report positive error codes as negative number */
                xStatus = xStatus > 0 ? SOCKETS_TLS_HANDSHAKE_ERROR : xStatus;
            }
        }
    }
    else
    {
        xStatus = SOCKETS_SOCKET_ERROR;
    }

    return xStatus;
}
/*-----------------------------------------------------------*/


/* Convert IP address in uint32_t to comma separated bytes. */
#define UINT32_IPADDR_TO_CSV_BYTES( a )                     \
    ( ( ( a ) >> 24 ) & 0xFF ), ( ( ( a ) >> 16 ) & 0xFF ), \
    ( ( ( a ) >> 8 ) & 0xFF ), ( ( a ) & 0xFF )

/*-----------------------------------------------------------*/

uint32_t SOCKETS_GetHostByName( const char * pcHostName )
{
    uint32_t ulAddr = 0;

    if( strlen( pcHostName ) <= ( size_t ) securesocketsMAX_DNS_NAME_LENGTH )
    {
#ifdef USE_WIFI
        WIFI_GetHostIP( ( char * ) pcHostName, ( uint8_t * ) &ulAddr );
#else
        /*{*/
			gsm_ip_t* ip = &gsm.m.conns[gsm.m.conn_val_id].remote_ip;
			//gsm_core_lock();
			CellIoT_lib_getHostIP(( char * ) pcHostName, NULL, NULL, 1);
			//gsm_core_unlock();
			ulAddr = ( ( ( ip->ip[0] ) << 24 ) & 0xFF000000 ) +
					 ( ( ( ip->ip[1] ) << 16 ) & 0x00FF0000 ) +
				     ( ( ( ip->ip[2] ) << 8 )  & 0x0000FF00 ) +
					 (   ( ip->ip[3] )         & 0x000000FF );

			/*ulAddr = ( ( ( ip->ip[0] ) << 24 ) & 0xFF000000 );
			ulAddr+= ( ( ( ip->ip[1] ) << 16 ) & 0x00FF0000 );
			ulAddr+= ( ( ( ip->ip[2] ) << 8 )  & 0x0000FF00 );
			ulAddr+= (   ( ip->ip[3] )         & 0x000000FF );*/
        /*}*/
#endif

	configPRINTF( ( "Looked up %s as %d.%d.%d.%d\r\n",
							pcHostName,
							UINT32_IPADDR_TO_CSV_BYTES( ulAddr ) ) );
    }
    else
    {
        configPRINTF( ( "Malformed URL, Host name: %s is too long.", pcHostName ) );
    }

    /* This api is to return the address in network order. WIFI_GetHostIP returns the host IP
     * in host order.
     */
    ulAddr = SOCKETS_htonl( ulAddr );

    return ulAddr;
}

/*-----------------------------------------------------------*/

int32_t SOCKETS_Recv( Socket_t xSocket,
                      void * pvBuffer,
                      size_t xBufferLength,
                      uint32_t ulFlags )
{
    int32_t lStatus = SOCKETS_ERROR_NONE;
    SSOCKETContextPtr_t pxContext = ( SSOCKETContextPtr_t ) xSocket;


    if( ( SOCKETS_INVALID_SOCKET != xSocket ) &&
        ( NULL != pvBuffer ) &&
        ( ( nxpsecuresocketsSOCKET_READ_CLOSED_FLAG & pxContext->xShutdownFlags ) == 0UL ) &&
        ( pxContext->ulState & ( nxpsecuresocketsSOCKET_CONNECTED_FLAG ) )
        )
    {
        pxContext->xRecvFlags = ( BaseType_t ) ulFlags;
        if( pdTRUE == pxContext->xRequireTLS )
        {
            /* Receive through TLS pipe, if negotiated. */
            lStatus = TLS_Recv( pxContext->pvTLSContext, pvBuffer, xBufferLength );
        }
        else
        {
            /* Receive unencrypted. */
            lStatus = prvNetworkRecv( pxContext, pvBuffer, xBufferLength );
        }
    }
    else
    {
        lStatus = SOCKETS_SOCKET_ERROR;
    }

    return lStatus;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_Send( Socket_t xSocket,
                      const void * pvBuffer,
                      size_t xDataLength,
                      uint32_t ulFlags )
{
    /* The WiFi module refuses to send data if it exceeds this threshold defined in
     * atheros_stack_offload.h */
#define IPV4_FRAGMENTATION_THRESHOLD (1664U+88U)
    uint32_t ulSendMaxLength = IPV4_FRAGMENTATION_THRESHOLD;
    int32_t lWritten = 0, lWrittenPerLoop = 0;
    SSOCKETContextPtr_t pxContext = ( SSOCKETContextPtr_t ) xSocket;

    if( ( SOCKETS_INVALID_SOCKET != pxContext->xSocket ) &&
        ( NULL != pvBuffer ) &&
        ( ( nxpsecuresocketsSOCKET_WRITE_CLOSED_FLAG & pxContext->xShutdownFlags ) == 0UL ) &&
        ( pxContext->ulState & ( nxpsecuresocketsSOCKET_CONNECTED_FLAG ) )
        )
    {
        pxContext->xSendFlags = ( BaseType_t ) ulFlags;

        if( pdTRUE == pxContext->xRequireTLS )
        {
            /* In case of TLS, reserve extra space for SSL meta data (header, maclen, ivlen, ... = 45B) */
            ulSendMaxLength = 1531;

            for(
                uint32_t ulToRemain = xDataLength, ulBufferPos = 0, ulToSend = 0;
                ulToRemain;
                ulBufferPos += ulToSend, ulToRemain -= ulToSend
                )
            {
                ulToSend = ulToRemain > ulSendMaxLength ? ulSendMaxLength : ulToRemain;
                /* Send through TLS pipe, if negotiated. */
                lWrittenPerLoop = TLS_Send( pxContext->pvTLSContext,
                                            &( ( unsigned char const * ) pvBuffer )[ ulBufferPos ],
                                            ulToSend );

                /* Error code < 0. */
                if( lWrittenPerLoop < 0 )
                {
                    /* Set the error code to be returned */
                    lWritten = lWrittenPerLoop;
                    break;
                }

                /* Number of lWritten bytes. */
                lWritten += lWrittenPerLoop;

                if( lWrittenPerLoop != ulToSend )
                {
                    break;
                }
            }
        }
        else
        {
            for(
                uint32_t ulToRemain = xDataLength, ulBufferPos = 0, ulToSend = 0;
                ulToRemain;
                ulBufferPos += ulToSend, ulToRemain -= ulToSend
                )
            {
                ulToSend = ulToRemain > ulSendMaxLength ? ulSendMaxLength : ulToRemain;
                lWrittenPerLoop = prvNetworkSend( pxContext,
                                                  &( ( unsigned char const * ) pvBuffer )[ ulBufferPos ],
                                                  ulToSend );

                /* Error code < 0. */
                if( lWrittenPerLoop < 0 )
                {
                    /* Set the error code to be returned */
                    lWritten = lWrittenPerLoop;
                    break;
                }

                /* Number of lWritten bytes. */
                lWritten += lWrittenPerLoop;

                if( lWrittenPerLoop != ulToSend )
                {
                    break;
                }
            }
        }
    }
    else
    {
        lWritten = SOCKETS_SOCKET_ERROR;
    }

    return lWritten;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_SetSockOpt( Socket_t xSocket,
                            int32_t lLevel,
                            int32_t lOptionName,
                            const void * pvOptionValue,
                            size_t xOptionLength )
{
    int32_t lStatus = SOCKETS_ERROR_NONE;
    TickType_t xTimeout;
    SSOCKETContextPtr_t pxContext = ( SSOCKETContextPtr_t ) xSocket;

    if( ( NULL != pxContext ) && ( SOCKETS_INVALID_SOCKET != pxContext->xSocket ) )
    {
        switch( lOptionName )
        {
            case SOCKETS_SO_SERVER_NAME_INDICATION:

                /* Secure socket option cannot be used on connected socket */
                if( pxContext->ulState & ( nxpsecuresocketsSOCKET_CONNECTED_FLAG ) )
                {
                    lStatus = SOCKETS_SOCKET_ERROR;
                    break;
                }

                /* Non-NULL destination string indicates that SNI extension should
                 * be used during TLS negotiation. */
                if( NULL == ( pxContext->pcDestination =
                                  ( char * ) pvPortMalloc( 1 + xOptionLength ) ) )
                {
                    lStatus = SOCKETS_ENOMEM;
                }
                else
                {
                    memcpy( pxContext->pcDestination, pvOptionValue, xOptionLength );
                    pxContext->pcDestination[ xOptionLength ] = '\0';
                }

                break;

            case SOCKETS_SO_TRUSTED_SERVER_CERTIFICATE:

                /* Secure socket option cannot be used on connected socket */
                if( pxContext->ulState & ( nxpsecuresocketsSOCKET_CONNECTED_FLAG ) )
                {
                    lStatus = SOCKETS_SOCKET_ERROR;
                    break;
                }

                /* Non-NULL server certificate field indicates that the default trust
                 * list should not be used. */
                if( NULL == ( pxContext->pcServerCertificate =
                                  ( char * ) pvPortMalloc( xOptionLength ) ) )
                {
                    lStatus = SOCKETS_ENOMEM;
                }
                else
                {
                    memcpy( pxContext->pcServerCertificate, pvOptionValue, xOptionLength );
                    pxContext->ulServerCertificateLength = xOptionLength;
                }

                break;

            case SOCKETS_SO_REQUIRE_TLS:

                /* Secure socket option cannot be used on connected socket */
                if( pxContext->ulState & ( nxpsecuresocketsSOCKET_CONNECTED_FLAG ) )
                {
                    lStatus = SOCKETS_SOCKET_ERROR;
                    break;
                }

                pxContext->xRequireTLS = pdTRUE;
                break;

            case SOCKETS_SO_NONBLOCK:

                if( pxContext->ulState & ( nxpsecuresocketsSOCKET_CONNECTED_FLAG ) )
                {
                    xTimeout = 0;
                    /* TODO: Investigate the NONBLOCK compile time config. */
                    pxContext->ulSendTimeout = 1;
                    pxContext->ulRecvTimeout = 1;
                }
                else
                {
                    lStatus = SOCKETS_SOCKET_ERROR;
                }

                break;

            case SOCKETS_SO_RCVTIMEO:
                xTimeout = *( ( const TickType_t * ) pvOptionValue ); /*lint !e9087 pvOptionValue passed should be of TickType_t. */

                if( xTimeout == 0U )
                {
                    pxContext->ulRecvTimeout = portMAX_DELAY;
                }
                else
                {
                    pxContext->ulRecvTimeout = xTimeout;
                }

                break;

            case SOCKETS_SO_SNDTIMEO:
                /* Comply with Berkeley standard - a 0 timeout is wait forever. */
                xTimeout = *( ( const TickType_t * ) pvOptionValue ); /*lint !e9087 pvOptionValue passed should be of TickType_t. */

                if( xTimeout == 0U )
                {
                    pxContext->ulSendTimeout = portMAX_DELAY;
                }
                else
                {
                    pxContext->ulSendTimeout = xTimeout;
                }

                break;

            case SOCKETS_SO_ALPN_PROTOCOLS:

                /* Secure socket option cannot be used on connected socket */
                if( pxContext->ulState & ( nxpsecuresocketsSOCKET_CONNECTED_FLAG ) )
                {
                    lStatus = SOCKETS_SOCKET_ERROR;
                    break;
                }

                /* Prepare list of supported protocols, must be NULL-terminated */
                pxContext->ppcAlpnProtocols = (char **)pvPortMalloc((xOptionLength + 1 )*sizeof(char*));
                if( pxContext->ppcAlpnProtocols == NULL )
                {
                    lStatus = SOCKETS_SOCKET_ERROR;
                    break;
                }

                int i;
                for( i = 0; i < xOptionLength; i++ )
                {
                    int len = strlen( ((char **)pvOptionValue)[i] );
                    pxContext->ppcAlpnProtocols[i] = (char *)pvPortMalloc(len + 1);
                    if( pxContext->ppcAlpnProtocols[i] == NULL )
                    {
                        /* Malloc failed - remove created list and set error return value */
                        while(i)
                        {
                            vPortFree( pxContext->ppcAlpnProtocols[i - 1] );
                            i--;
                        }
                        vPortFree( pxContext->ppcAlpnProtocols );
                        pxContext->ppcAlpnProtocols = NULL;
                        pxContext->ulAlpnProtocolsCount = 0;

                        lStatus = SOCKETS_SOCKET_ERROR;
                        break;
                    }
                    memcpy( pxContext->ppcAlpnProtocols[i], ((char **)pvOptionValue)[i], len );
                    pxContext->ppcAlpnProtocols[i][len] = '\0';
                }

                if( lStatus != SOCKETS_SOCKET_ERROR )
                {
                    pxContext->ppcAlpnProtocols[xOptionLength] = NULL;
                    pxContext->ulAlpnProtocolsCount = xOptionLength;
                }

                break;

            default:

                /* Nothing to do */
                break;
        }
    }
    else
    {
        lStatus = SOCKETS_SOCKET_ERROR;
    }

    return lStatus;
}
/*-----------------------------------------------------------*/

int32_t SOCKETS_Shutdown( Socket_t xSocket,
                          uint32_t ulHow )
{
    SSOCKETContextPtr_t pxSecureSocket = ( SSOCKETContextPtr_t ) xSocket;
    int32_t lRetVal = SOCKETS_SOCKET_ERROR;

    /* Ensure that a valid socket was passed. */
    if( ( SOCKETS_INVALID_SOCKET != pxSecureSocket->xSocket ) )
    {
        switch( ulHow )
        {
            case SOCKETS_SHUT_RD:
                /* Further receive calls on this socket should return error. */
                pxSecureSocket->xShutdownFlags |= nxpsecuresocketsSOCKET_READ_CLOSED_FLAG;

                /* Return success to the user. */
                lRetVal = SOCKETS_ERROR_NONE;
                break;

            case SOCKETS_SHUT_WR:
                /* Further send calls on this socket should return error. */
                pxSecureSocket->xShutdownFlags |= nxpsecuresocketsSOCKET_WRITE_CLOSED_FLAG;

                /* Return success to the user. */
                lRetVal = SOCKETS_ERROR_NONE;
                break;

            case SOCKETS_SHUT_RDWR:
                /* Further send or receive calls on this socket should return error. */
                pxSecureSocket->xShutdownFlags |= nxpsecuresocketsSOCKET_READ_CLOSED_FLAG;
                pxSecureSocket->xShutdownFlags |= nxpsecuresocketsSOCKET_WRITE_CLOSED_FLAG;

                /* Return success to the user. */
                lRetVal = SOCKETS_ERROR_NONE;
                break;

            default:
                /* An invalid value was passed for ulHow. */
                lRetVal = SOCKETS_EINVAL;
                break;
        }
    }
    else
    {
        return SOCKETS_EINVAL;
    }

    return lRetVal;
}
/*-----------------------------------------------------------*/

Socket_t SOCKETS_Socket( int32_t lDomain,
                         int32_t lType,
                         int32_t lProtocol )
{
    int32_t lStatus = SOCKETS_ERROR_NONE;
    int32_t xSocket = 0;
    SSOCKETContextPtr_t pxContext = NULL;

    /* Ensure that only supported values are supplied. */
    configASSERT( lDomain == SOCKETS_AF_INET );
    configASSERT( lType == SOCKETS_SOCK_STREAM );
    configASSERT( lProtocol == SOCKETS_IPPROTO_TCP );

    /* Allocate the internal context structure. */
    pxContext = pvPortMalloc( sizeof( SSOCKETContext_t ) );

    if( pxContext != NULL )
    {
        memset( pxContext, 0, sizeof( SSOCKETContext_t ) );

        /* Create the wrapped socket. */

       // Attribute a Socket number to simulate the qcom_socket API
        xSocket = 1;

        if( xSocket != -1 )
        {
            pxContext->xSocket = ( Socket_t ) xSocket;
            /* Set default timeouts. */
            pxContext->ulRecvTimeout = socketsconfigDEFAULT_RECV_TIMEOUT;
            pxContext->ulSendTimeout = socketsconfigDEFAULT_SEND_TIMEOUT;
        }
        else /* Driver could not allocate socket. */
        {
            lStatus = SOCKETS_SOCKET_ERROR;
            vPortFree( pxContext );
        }
    }
    else /* Malloc failed. */
    {
        lStatus = SOCKETS_ENOMEM;
    }

    if( lStatus != SOCKETS_ERROR_NONE )
    {
        pxContext = (SSOCKETContextPtr_t) SOCKETS_INVALID_SOCKET;
    }

    return (Socket_t) pxContext;
}
/*-----------------------------------------------------------*/

BaseType_t SOCKETS_Init( void )
{
    /* Empty initialization for NXP board. */
    return pdPASS;
}