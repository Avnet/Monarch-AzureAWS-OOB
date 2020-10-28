/*
 * Copyright 2019-2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MSFT_AZURE_C_
#define MSFT_AZURE_C_

/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* FreeRTOS includes. */
#include "FreeRTOS.h"
#include "task.h"
#include "msft_Azure_IoT.h"
#include "msft_Azure_IoT_clientcredential.h"
#include "iot_mqtt_agent.h"
#include "iot_mqtt_types.h"
#include "event_groups.h"
#include "board.h"
#include "iot_init.h"
#include "azure_default_root_certificates.h"

/* Maximum amount of time a Shadow function call may block. */
#define AzureTwinDemoTIMEOUT                    pdMS_TO_TICKS( 30000UL )

#define Device_Twin_SET_JSON 			\
    "{"                             	\
    	"\"Temperature_Sensor\": %i,"	\
		"\"melody\": %d"	\
	"}"

const char strTrue[] = "TRUE";
const char strFalse[] = "FALSE";
const char strReported[] = "\"reported\"";
const char strMelody[] = "\"melody\":";

typedef enum AZURE_TWIN_TASK_ST
{
	AZURE_TW_IDLE,
	AZURE_TW_SET_STATE,
	AZURE_TW_WAIT_SET_RESP,
	AZURE_TW_GET_STATE,
	AZURE_TW_WAIT_GET_RESP,
	AZURE_STATES_BNDRY
}Azure_TW_Task;

static MQTTAgentConnectParams_t xConnectParams;
const MQTTAgentConnectParams_t * pConnectParams = &xConnectParams;
MQTTAgentHandle_t xMQTTHandle;

static Azure_TW_Task enAzure_TW_Task;
static MQTTAgentSubscribeParams_t xSubscribeParams;
static MQTTAgentUnsubscribeParams_t xUnsubscribeParams;
static MQTTAgentPublishParams_t xPublishParameters;
static char Scratch_Buffer[100];
EventGroupHandle_t xCreatedEventGroup;

static int Temp_Coeff;
//static int Hot_Temp;
static int Room_Temp;
//extern uint16_t * melody;
//extern uint16_t melody1[];
//extern uint16_t melody2[];



MQTTBool_t Twin_Operations_CallBack(void * pvPublishCallbackContext,
        							const MQTTPublishData_t * const pxPublishData )
{
	char * ptrData;

	// User could control the response from Azure here .
	if( enAzure_TW_Task == AZURE_TW_WAIT_SET_RESP )
	{
		AZUREDEBUG_PRINTF( ( " TWIN SET Operation Done. \r\n" ) );
	}
	else if( enAzure_TW_Task == AZURE_TW_WAIT_GET_RESP  )
	{
		ptrData = strstr( ( char *)pxPublishData->pvData , strReported );

		if( ptrData == NULL )
			goto cleanup;

		uint32_t desired_block_size = strlen(pxPublishData->pvData) - strlen(ptrData );
		strncpy( Scratch_Buffer , pxPublishData->pvData , desired_block_size ); // only desired part of data

		ptrData = strstr( ( char *)Scratch_Buffer, strMelody );
		if( ptrData == NULL )
			goto cleanup;

		ptrData += sizeof( strMelody)-1;

		if( *ptrData == '1' )
		{
			AZUREDEBUG_PRINTF( ( clientcredentialAZURE_IOT_THING_NAME " Playing Melody 1 \r\n" ) );
//			melody = melody1;
		}
		else if( *ptrData == '2' )
		{
			AZUREDEBUG_PRINTF( ( clientcredentialAZURE_IOT_THING_NAME " Playing Melody 2 \r\n" ) );
//			melody = melody2;
		}

		cleanup:

		AZUREDEBUG_PRINTF( ( " TWIN GET Operation Done. \r\n" ) );
	}

	xEventGroupSetBits(xCreatedEventGroup, 0x01);
	MQTT_AGENT_ReturnBuffer(( MQTTAgentHandle_t) 2, pxPublishData->xBuffer);

	return eMQTTTrue;
}

void prvmcsft_Azure_TwinTask( void * pvParameters )
{
	MQTTAgentReturnCode_t xMQTTReturn;
	//UBaseType_t Broker_Num = 1 ;
	char cPayload[100];
	char cTopic[50];
	uint8_t Req_Id =1;
	int Temp;

    ( void ) pvParameters;

    /* Initialize common libraries required by demo. */
	if (IotSdk_Init() != true)
	{
		configPRINTF(("Failed to initialize the common library."));
		vTaskDelete(NULL);
	}

    if( MQTT_AGENT_Create( &xMQTTHandle ) != eMQTTAgentSuccess )
    {
        configPRINTF(("Failed to initialize the MQTT Handle, stopping demo.\r\n"));
        vTaskDelete(NULL);
    }

    MQTT_AGENT_Init();

    memset( &xConnectParams, 0x00, sizeof( xConnectParams ) );
    xConnectParams.pcURL = clientcredentialAZURE_MQTT_BROKER_ENDPOINT;
    xConnectParams.usPort = clientcredentialAZURE_MQTT_BROKER_PORT;

    xConnectParams.xFlags = mqttagentREQUIRE_TLS;
    xConnectParams.pcCertificate = (char *)AZURE_SERVER_ROOT_CERTIFICATE_PEM;
    xConnectParams.ulCertificateSize = sizeof(AZURE_SERVER_ROOT_CERTIFICATE_PEM);
    xConnectParams.pxCallback = NULL;
    xConnectParams.pvUserData = NULL;

    xConnectParams.pucClientId = (const uint8_t *)(clientcredentialAZURE_IOT_THING_NAME);
    xConnectParams.usClientIdLength = (uint16_t)strlen(clientcredentialAZURE_IOT_THING_NAME);
#if SSS_USE_FTR_FILE
    xConnectParams.cUserName = clientcredentialAZURE_IOT_MQTT_USERNAME;
    xConnectParams.uUsernamelength = ( uint16_t ) strlen(clientcredentialAZURE_IOT_MQTT_USERNAME);
    xConnectParams.p_password = NULL ;
    xConnectParams.passwordlength = 0;
#endif

    enAzure_TW_Task = AZURE_TW_IDLE;

    xCreatedEventGroup = xEventGroupCreate();

    while( 1 )
    {
    	switch( enAzure_TW_Task )
    	{
    		case AZURE_TW_IDLE:

    		    xMQTTReturn = MQTT_AGENT_Connect( xMQTTHandle,
    		    								  pConnectParams,
    											  AzureTwinDemoTIMEOUT);

    		    if( eMQTTAgentSuccess == xMQTTReturn )
    		    {
    		    	AZUREDEBUG_PRINTF( ("Connected to Azure IoT Hub Successfully\r\n") );
    		    }
    		    else
    		    {
    		    	AZUREDEBUG_PRINTF( ("Connection refused!! \r\n") );
    		    	vTaskDelete(NULL);
    		    }

    		    xSubscribeParams.pucTopic = (const uint8_t *)AZURE_OPERATION_RESPONSE_TOPIC;
    		    xSubscribeParams.pvPublishCallbackContext = NULL;
    		    xSubscribeParams.pxPublishCallback = Twin_Operations_CallBack;
    		    xSubscribeParams.usTopicLength = sizeof(AZURE_OPERATION_RESPONSE_TOPIC) - 1;
    		    xSubscribeParams.xQoS = eMQTTQoS0;

    		    xUnsubscribeParams.pucTopic = (const uint8_t *)AZURE_OPERATION_RESPONSE_TOPIC;
    		    xUnsubscribeParams.usTopicLength = sizeof(AZURE_OPERATION_RESPONSE_TOPIC) - 1;

    		    if( MQTT_AGENT_Subscribe(xMQTTHandle, &xSubscribeParams, pdMS_TO_TICKS(10000)) == eMQTTAgentSuccess)
    		    {
    		    	AZUREDEBUG_PRINTF( ("Successfully Subscribe to Operations Topic\r\n") );
    		    	enAzure_TW_Task = AZURE_TW_SET_STATE;
    		    }
                else
                {
                	AZUREDEBUG_PRINTF( ("Unsuccessfully Subscribe to Operations Topic\r\n"));
                	AZUREDEBUG_PRINTF( ("Disconnect\r\n"));
                	MQTT_AGENT_Disconnect(xMQTTHandle, AzureTwinDemoTIMEOUT);
                	enAzure_TW_Task = AZURE_TW_IDLE;
                }

    			break;

    		case AZURE_TW_SET_STATE:
//    			TEMPMON->TEMPSENSE0 = TEMPMON_TEMPSENSE0_SET_POWER_DOWN(0) | TEMPMON_TEMPSENSE0_SET_MEASURE_TEMP(1); // enable power start conversion
    			vTaskDelay(pdMS_TO_TICKS(2));
//    			Temp = TEMPMON->TEMPSENSE0;
    			Temp = (( Temp >> 8 ) & 0xfff );
    			Temp -= Room_Temp;
    			Temp *= (95-25);
    			Temp *= 100;
    			Temp = Temp / Temp_Coeff;
    			Temp += 2500;
                memset(&(xPublishParameters), 0x00, sizeof(xPublishParameters));
                sprintf(cPayload, Device_Twin_SET_JSON, Temp , 0 );
                sprintf(cTopic, AZURE_IOT_MQTT_TWIN_SET_TOPIC, Req_Id );
                Req_Id++;
                xPublishParameters.pucTopic = (const uint8_t *)cTopic;
                xPublishParameters.pvData = cPayload;
                xPublishParameters.usTopicLength = (uint16_t)strlen((const char *)xPublishParameters.pucTopic);
                xPublishParameters.ulDataLength = strlen(cPayload);
                xPublishParameters.xQoS = eMQTTQoS0;

                if( MQTT_AGENT_Publish(xMQTTHandle, &xPublishParameters, pdMS_TO_TICKS(30000)) == eMQTTAgentSuccess )
                {
                	AZUREDEBUG_PRINTF( ("Successfully Publish to SET Topic\r\n"));
                    enAzure_TW_Task = AZURE_TW_WAIT_SET_RESP;
                }
                else
                {
                	AZUREDEBUG_PRINTF( ("Unsuccessfully Publish to SET Topic\r\n"));
                	AZUREDEBUG_PRINTF( ("Disconnect\r\n"));
                	MQTT_AGENT_Disconnect(xMQTTHandle, AzureTwinDemoTIMEOUT);
                	enAzure_TW_Task = AZURE_TW_IDLE;
                }

    			break;

    		case AZURE_TW_WAIT_SET_RESP:

    			if( xEventGroupWaitBits(xCreatedEventGroup, 0x01, pdTRUE, pdFALSE, pdMS_TO_TICKS(30000)) == 0 )
    			{
    				enAzure_TW_Task = AZURE_TW_GET_STATE;
    			}
    			else
    			{
    				enAzure_TW_Task = AZURE_TW_GET_STATE;
    				vTaskDelay(pdMS_TO_TICKS(2000));
    			}

    			break;

    		case AZURE_TW_GET_STATE:
    			memset(&(xPublishParameters), 0x00, sizeof(xPublishParameters));
    			sprintf(cPayload, "{ }" );
                sprintf(cTopic, AZURE_IOT_MQTT_TWIN_GET_TOPIC, Req_Id );
                Req_Id++;
                xPublishParameters.pucTopic = (const uint8_t *)cTopic;
                xPublishParameters.pvData = cPayload;
                xPublishParameters.usTopicLength = (uint16_t)strlen((const char *)xPublishParameters.pucTopic);
                xPublishParameters.ulDataLength = 3;
                xPublishParameters.xQoS = eMQTTQoS0;
                if( MQTT_AGENT_Publish(xMQTTHandle, &xPublishParameters, pdMS_TO_TICKS(30000)) == eMQTTAgentSuccess )
                {
                	AZUREDEBUG_PRINTF( ("Successfully Publish to GET Topic\r\n"));
                    enAzure_TW_Task = AZURE_TW_WAIT_GET_RESP;
                }
                else
                {
                	AZUREDEBUG_PRINTF( ("Unsuccessfully Publish to GET Topic\r\n"));
                	AZUREDEBUG_PRINTF( ("Disconnect\r\n"));
                	MQTT_AGENT_Disconnect(xMQTTHandle, AzureTwinDemoTIMEOUT);
                	enAzure_TW_Task = AZURE_TW_IDLE;
                }

    			break;

    		case AZURE_TW_WAIT_GET_RESP:

    			if( xEventGroupWaitBits(xCreatedEventGroup, 0x01, pdTRUE, pdFALSE, pdMS_TO_TICKS(30000)) == 0 )
    			{
    				enAzure_TW_Task = AZURE_TW_SET_STATE;
    			}
    			else
    			{
    				enAzure_TW_Task = AZURE_TW_SET_STATE;
    				vTaskDelay(pdMS_TO_TICKS(2000));
    			}
    			break;

    		default:
    	    	AZUREDEBUG_PRINTF( ( "Invalid Application State!! \r\n" ) );
    	    	AZUREDEBUG_PRINTF( ( "Closing Azure Demo\r\n" ) );
    	    	vTaskDelete(NULL);
    			break;

    	}
    }

}

void vStartAzureLedDemoTask( void )
{
    ( void ) xTaskCreate( prvmcsft_Azure_TwinTask,
                          "Microsoft Azure Twin Task",
						  AzureTwin_DemoUPDATE_TASK_STACK_SIZE,
                          NULL,
                          tskIDLE_PRIORITY,
                          NULL );
}

#endif /* MSFT_AZURE_C_ */
