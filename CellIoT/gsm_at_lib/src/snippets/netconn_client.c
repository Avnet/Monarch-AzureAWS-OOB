#include "netconn_client.h"
#include "gsm.h"
#include "gsm_network_api.h"

#if GSM_CFG_NETCONN

/**
 * \brief           Host and port settings
 */
#define NETCONN_HOST        "example.com"
#define NETCONN_PORT        80

/**
 * \brief           Request header to send on successful connection
 */
static const char
request_header[] = ""
"GET / HTTP/1.1\r\n"
"Host: " NETCONN_HOST "\r\n"
"Connection: close\r\n"
"\r\n";

/**
 * \brief           Netconn client thread implementation
 * \param[in]       arg: User argument
 */
void
netconn_client_thread(void const* arg) {
    gsmr_t res;
    gsm_pbuf_p pbuf;
    gsm_netconn_p client;
    gsm_sys_sem_t* sem = (void *)arg;

    /* Request attach to network */
    while (gsm_network_request_attach() != gsmOK) {
        gsm_delay(1000);
    }

    /*
     * First create a new instance of netconn
     * connection and initialize system message boxes
     * to accept received packet buffers
     */
    client = gsm_netconn_new(GSM_NETCONN_TYPE_TCP);
    if (client != NULL) {
        /*
         * Connect to external server as client
         * with custom NETCONN_CONN_HOST and CONN_PORT values
         *
         * Function will block thread until we are successfully connected (or not) to server
         */
        res = gsm_netconn_connect(client, NETCONN_HOST, NETCONN_PORT);
        if (res == gsmOK) {                     /* Are we successfully connected? */
            printf("Connected to " NETCONN_HOST "\r\n");
            res = gsm_netconn_write(client, request_header, sizeof(request_header) - 1);    /* Send data to server */
            if (res == gsmOK) {
                res = gsm_netconn_flush(client);/* Flush data to output */
            }
            if (res == gsmOK) {                 /* Were data sent? */
                printf("Data were successfully sent to server\r\n");

                /*
                 * Since we sent HTTP request,
                 * we are expecting some data from server
                 * or at least forced connection close from remote side
                 */
                do {
                    /*
                     * Receive single packet of data
                     *
                     * Function will block thread until new packet
                     * is ready to be read from remote side
                     *
                     * After function returns, don't forgot the check value.
                     * Returned status will give you info in case connection
                     * was closed too early from remote side
                     */
                    res = gsm_netconn_receive(client, &pbuf);
                    if (res == gsmCLOSED) {     /* Was the connection closed? This can be checked by return status of receive function */
                        printf("Connection closed by remote side...\r\n");
                        break;
                    } else if (res == gsmTIMEOUT) {
                        printf("Netconn timeout while receiving data. You may try multiple readings before deciding to close manually\r\n");
                    }

                    if (res == gsmOK && pbuf != NULL) { /* Make sure we have valid packet buffer */
                        /*
                         * At this point read and manipulate
                         * with received buffer and check if you expect more data
                         *
                         * After you are done using it, it is important
                         * you free the memory otherwise memory leaks will appear
                         */
                        printf("Received new data packet of %d bytes\r\n", (int)gsm_pbuf_length(pbuf, 1));
                        gsm_pbuf_free(pbuf);    /* Free the memory after usage */
                        pbuf = NULL;
                    }
                } while (1);
            } else {
                printf("Error writing data to remote host!\r\n");
            }

            /*
             * Check if connection was closed by remote server
             * and in case it wasn't, close it manually
             */
            if (res != gsmCLOSED) {
                gsm_netconn_close(client);
            }
        } else {
            printf("Cannot connect to remote host %s:%d!\r\n", NETCONN_HOST, NETCONN_PORT);
        }
        gsm_netconn_delete(client);             /* Delete netconn structure */
    }
    gsm_network_request_detach();               /* Detach from network */

    if (gsm_sys_sem_isvalid(sem)) {
        gsm_sys_sem_release(sem);
    }
    gsm_sys_thread_terminate(NULL);             /* Terminate current thread */
}

#endif /* GSM_CFG_NETCONN */
