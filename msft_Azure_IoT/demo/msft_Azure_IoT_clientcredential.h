/*
 * Copyright 2020 NXP
 * All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */


#ifndef MSFT_AZURE_IOT_CLIENTCREDENTIAL_H_
#define MSFT_AZURE_IOT_CLIENTCREDENTIAL_H_
/*
 * AZURE_IOT_BROKER = 1 --> Azure IoT Hub
 * AZURE_IOT_BROKER = 2 --> Azure IoT Central
 */
#define AZURE_IOT_BROKER 2

/*
 * @brief MQTT Broker endpoint.
 *
 * @todo Set this to the fully-qualified DNS name of your MQTT broker.
 */
#define clientcredentialAZURE_MQTT_BROKER_ENDPOINT 	     ""


#if (AZURE_IOT_BROKER == 2)
/*
 * @brief Host name.
 *
 * @todo Set this to the Scope ID of your IoT Thing.
 */
#define clientcredentialAZURE_IOT_SCOPE_ID               ""

#endif
/*
 * @brief Host name.
 *
 * @todo Set this to the unique name of your IoT Thing.
 */
#define clientcredentialAZURE_IOT_DEVICE_ID              ""

/*
 * @brief UniStream MQTT username.
 *
 * @todo The UniStream MQTT username for Azure IoT Hub will be {iothubhostname}/{device_id}/?api-version=2018-06-30,
 * where {iothubhostname} is the full CName of the IoT hub and {device_id} is the ClientID.
 *
 * @todo The MQTT username forAzure IoT Central will be {scope_id}/registrations/{device_id}/api-version=2019-03-31,
 * where {scope_id} is the Scope ID of the Azure IoT Central Dashboard and {device_id} is the ClientID.
 *
 */


#if (AZURE_IOT_BROKER == 1)

#define clientcredentialAZURE_IOT_MQTT_USERNAME clientcredentialAZURE_MQTT_BROKER_ENDPOINT "/" clientcredentialAZURE_IOT_DEVICE_ID "/?api-version=2018-06-30"

#elif (AZURE_IOT_BROKER == 2)

#define clientcredentialAZURE_IOT_MQTT_USERNAME clientcredentialAZURE_IOT_SCOPE_ID "/registrations/" clientcredentialAZURE_IOT_DEVICE_ID "/api-version=2019-03-31"

#endif
/*
 * @brief Port number the MQTT broker is using.
 */
#define clientcredentialAZURE_MQTT_BROKER_PORT             8883

/*
 * ThingSpace Credentials
 *
 * To generate THINGSPACE_KEY_SECRET:
 *   1 - Navigate to https://thingspace.verizon.com/
 *   2 - Sign-in to manage your API Keys
 *   3 - From the account icon (upper right hand corner), select API
 *   4 - From the left side of the page select API-->API Key management
 *   5 - Copy your Key and Secret
 *   6 - Navigate to https://www.base64encode.org/
 *   7 - Input your key and secret in the following format <key>:<secret>
 *   8 - Press the >Encode< button
 *   9 - Copy the base64 encoded string below
 *
 * To create API credentials (different from ThingSpace/Verizon account):
 *   1 - Navigate to https://globalm2m.verizonwireless.com/manage/apiuser
 *   2 - Enter a user name (can be the same as the ThingSpace login)
 *   3 - Enter a password (twice) and confirm
 *   4 - Enter user/password below
 *
 * To find account and device numbers:
 *   1 - Navigate to https://globalm2m.verizonwireless.com/manage/devices
 *   2 - Click the device you are testing
 *   3 - The IMEI and MDN are in the first box (Device identity)
 *   4 - The Service plan and billing section has the account number
 *   5 - Copy these numbers into the appropriate defines below
 */
#define THINGSPACE_KEY_SECRET    ""
#define THINGSPACE_USER          ""
#define THINGSPACE_PASSWORD      ""
#define THINGSPACE_ACCOUNT_NUM   ""
#define THINGSPACE_DEVICE_IMEI   ""
#define THINGSPACE_DEVICE_MDN    ""


#endif /* MSFT_AZURE_IOT_CLIENTCREDENTIAL_H_ */
