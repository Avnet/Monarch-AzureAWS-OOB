/**
 * \file            gsm_parser.c
 * \brief           Parse incoming data from AT port
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
#include "gsm_parser.h"
#include "gsm_mem.h"
#include "gsm_utils.h"
#include "gsm_int.h"
#include "CellIoT_lib.h"
#include "math.h"

/**
 * \brief           Parse number from string
 * \note            Input string pointer is changed and number is skipped
 * \param[in]       Pointer to pointer to string to parse
 * \return          Parsed number
 */
int32_t
gsmi_parse_number(const char** str) {
    int32_t val = 0;
    uint8_t minus = 0;
    const char* p = *str;                       /*  */

    if (*p == '"') {                            /* Skip leading quotes */
        p++;
    }
    if (*p == ',') {                            /* Skip leading comma */
        p++;
    }
    if (*p == '"') {                            /* Skip leading quotes */
        p++;
    }
    if (*p == '/') {                            /* Skip '/' character, used in datetime */
        p++;
    }
    if (*p == ':') {                            /* Skip ':' character, used in datetime */
        p++;
    }
    if (*p == '+') {                            /* Skip '+' character, used in datetime */
        p++;
    }
    if (*p == '-') {                            /* Check negative number */
        minus = 1;
        p++;
    }
    while (GSM_CHARISNUM(*p)) {                 /* Parse until character is valid number */
        val = val * 10 + GSM_CHARTONUM(*p);
        p++;
    }
    if (*p == '"') {                            /* Skip trailling quotes */
        p++;
    }
    *str = p;                                   /* Save new pointer with new offset */

    return minus ? -val : val;
}

/**
 * \brief           Parse number from string as hex
 * \note            Input string pointer is changed and number is skipped
 * \param[in]       Pointer to pointer to string to parse
 * \return          Parsed number
 */
uint32_t
gsmi_parse_hexnumber(const char** str) {
    int32_t val = 0;
    const char* p = *str;                       /*  */

    if (*p == '"') {                            /* Skip leading quotes */
        p++;
    }
    if (*p == ',') {                            /* Skip leading comma */
        p++;
    }
    if (*p == '"') {                            /* Skip leading quotes */
        p++;
    }
    while (GSM_CHARISHEXNUM(*p)) {              /* Parse until character is valid number */
        val = val * 16 + GSM_CHARHEXTONUM(*p);
        p++;
    }
    if (*p == ',') {                            /* Go to next entry if possible */
        p++;
    }
    *str = p;                                   /* Save new pointer with new offset */
    return val;
}

/**
 * \brief           Parse input string as string part of AT command
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[in]       dst: Destination pointer.
 *                      Set to `NULL` in case you want to skip string in source
 * \param[in]       dst_len: Length of distance buffer,
 *                      including memory for `NULL` termination
 * \param[in]       trim: Set to `1` to process entire string,
 *                      even if no memory anymore
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gsmi_parse_string(const char** src, char* dst, size_t dst_len, uint8_t trim) {
    const char* p = *src;
    size_t i;

    if (*p == ',') {
        p++;
    }
    if (*p == '"') {
        p++;
    }
    i = 0;
    if (dst_len > 0) {
        dst_len--;
    }
    while (*p) {
        if (*p == '"' && (p[1] == ',' || p[1] == '\r' || p[1] == '\n')) {
            p++;
            break;
        }
        if (dst != NULL) {
            if (i < dst_len) {
                *dst++ = *p;
                i++;
            } else if (!trim) {
                break;
            }
        }
        p++;
    }
    if (dst != NULL) {
        *dst = 0;
    }
    *src = p;
    return 1;
}

/**
 * \brief           Parse the input to get to the next param position
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gsmi_get_next_param_pos(const char** src) {
    const char* p = *src;

    while (*p != ',') {
        p++;
    }

    p++;
    *src = p;
    return 1;
}

/**
 * \brief           Check current string position and trim to the next entry
 * \param[in]       src: Pointer to pointer to input string
 */
void
gsmi_check_and_trim(const char** src) {
    const char* t = *src;
    if (*t != '"' && *t != '\r' && *t != ',') { /* Check if trim required */
        gsmi_parse_string(src, NULL, 0, 1);     /* Trim to the end */
    }
}

/**
 * \brief           Parse string as IP address
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[in]       dst: Destination pointer
 * \return          `1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_ip(const char** src, gsm_ip_t* ip) {
    const char* p = *src;

    if (*p == ',') {
        p++;
    }
    if (*p == '"') {
        p++;
    }
    if (GSM_CHARISNUM(*p)) {
        ip->ip[0] = gsmi_parse_number(&p); p++;
        ip->ip[1] = gsmi_parse_number(&p); p++;
        ip->ip[2] = gsmi_parse_number(&p); p++;
        ip->ip[3] = gsmi_parse_number(&p);
    }
    if (*p == '"') {
        p++;
    }

    *src = p;                                   /* Set new pointer */
    return 1;
}

/**
 * \brief           Parse string as MAC address
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[in]       dst: Destination pointer
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_mac(const char** src, gsm_mac_t* mac) {
    const char* p = *src;

    if (*p == '"') {
        p++;
    }
    mac->mac[0] = gsmi_parse_hexnumber(&p); p++;
    mac->mac[1] = gsmi_parse_hexnumber(&p); p++;
    mac->mac[2] = gsmi_parse_hexnumber(&p); p++;
    mac->mac[3] = gsmi_parse_hexnumber(&p); p++;
    mac->mac[4] = gsmi_parse_hexnumber(&p); p++;
    mac->mac[5] = gsmi_parse_hexnumber(&p);
    if (*p == '"') {
        p++;
    }
    if (*p == ',') {
        p++;
    }
    *src = p;
    return 1;
}

/**
 * \brief           Parse memory string, ex. "SM", "ME", "MT", etc
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \return          Parsed memory
 */
gsm_mem_t
gsmi_parse_memory(const char** src) {
    size_t i, sl;
    gsm_mem_t mem = GSM_MEM_UNKNOWN;
    const char* s = *src;

    if (*s == ',') {
        s++;
    }
    if (*s == '"') {
        s++;
    }

    /* Scan all memories available for modem */
    for (i = 0; i < gsm_dev_mem_map_size; i++) {
        sl = strlen(gsm_dev_mem_map[i].mem_str);
        if (!strncmp(s, gsm_dev_mem_map[i].mem_str, sl)) {
            mem = gsm_dev_mem_map[i].mem;
            s += sl;
            break;
        }
    }

    if (mem == GSM_MEM_UNKNOWN) {
        gsmi_parse_string(&s, NULL, 0, 1);      /* Skip string */
    }
    if (*s == '"') {
        s++;
    }
    *src = s;
    return mem;
}


/**
 * \brief           Parse a string of memories in format "M1","M2","M3","M4",...
 * \param[in,out]   src: Pointer to pointer to string to parse from
 * \param[out]      mem_dst: Output result with memory list as bit field
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_memories_string(const char** src, uint32_t* mem_dst) {
    const char* str = *src;
    gsm_mem_t mem;

    *mem_dst = 0;
    if (*str == ',') {
        str++;
    }
    if (*str == '(') {
        str++;
    }
    do {
        mem = gsmi_parse_memory(&str);          /* Parse memory string */
        *mem_dst |= GSM_U32(1 << GSM_U32(mem)); /* Set as bit field */
    } while (*str && *str != ')');
    if (*str == ')') {
        str++;
    }
    *src = str;
    return 1;
}

/**
 * \brief           Parse received +CREG message
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_creg(const char* str, uint8_t skip_first) {
    if (*str == '+') {
        str += 7;
    }

    if (skip_first) {
        gsmi_parse_number(&str);
    }
    gsm.m.network.status = (gsm_network_reg_status_t)gsmi_parse_number(&str);

    /*
     * In case we are connected to network,
     * scan for current network info
     */
    if (gsm.m.network.status == GSM_NETWORK_REG_STATUS_CONNECTED ||
        gsm.m.network.status == GSM_NETWORK_REG_STATUS_CONNECTED_ROAMING) {
        /* Try to get operator */
        /* Notify user in case we are not able to add new command to queue */
        gsm_operator_get(&gsm.m.network.curr_operator, NULL, NULL, 0);
#if GSM_CFG_NETWORK
    } else if (gsm_network_is_attached()) {
        gsm_network_check_status(NULL, NULL, 0);    /* Do the update */
#endif /* GSM_CFG_NETWORK */
    }

    /* Send callback event */
    gsmi_send_cb(GSM_EVT_NETWORK_REG_CHANGED);

    return 1;
}

/**
 * \brief           Parse received +CSQ signal value
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_csq(const char* str) {
    int16_t rssi;
    if (*str == '+') {
        str += 6;
    }

    rssi = gsmi_parse_number(&str);
    if (rssi < 32) {
        rssi = -(113 - (rssi * 2));
    } else {
        rssi = 0;
    }
    gsm.m.rssi = rssi;                          /* Save RSSI to global variable */
    if (gsm.msg->cmd_def == GSM_CMD_CSQ_GET &&
        gsm.msg->msg.csq.rssi != NULL) {
        *gsm.msg->msg.csq.rssi = rssi;          /* Save to user variable */
    }

    /* Report CSQ status */
    gsm.evt.evt.rssi.rssi = rssi;
    gsmi_send_cb(GSM_EVT_SIGNAL_STRENGTH);      /* RSSI event type */

    return 1;
}

/**
 * \brief           Parse received +CPIN status value
 * \param[in]       str: Input string
 * \param[in]       send_evt: Send event about new CPIN status
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_cpin(const char* str, uint8_t send_evt) {
    gsm_sim_state_t state;
    if (*str == '+') {
        str += 7;
    }
    if (!strncmp(str, "READY", 5)) {
        state = GSM_SIM_STATE_READY;
    } else if (!strncmp(str, "NOT READY", 9)) {
        state = GSM_SIM_STATE_NOT_READY;
    } else if (!strncmp(str, "NOT INSERTED", 14)) {
        state = GSM_SIM_STATE_NOT_INSERTED;
    } else if (!strncmp(str, "SIM PIN", 7)) {
        state = GSM_SIM_STATE_PIN;
    } else if (!strncmp(str, "SIM PUK", 7)) {
        state = GSM_SIM_STATE_PUK;
    } else {
        state = GSM_SIM_STATE_NOT_READY;
    }

    /* React only on change */
    if (state != gsm.m.sim.state) {
        gsm.m.sim.state = state;
        /*
         * In case SIM is ready,
         * start with basic info about SIM
         */
        if (gsm.m.sim.state == GSM_SIM_STATE_READY) {
            gsmi_get_sim_info(0);
        }

        if (send_evt) {
            gsm.evt.evt.cpin.state = gsm.m.sim.state;
            gsmi_send_cb(GSM_EVT_SIM_STATE_CHANGED);
        }
    }
    return 1;
}

/**
 * \brief           Parse +COPS string from COPS? command
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_cops(const char* str) {
    if (*str == '+') {
        str += 7;
    }

    gsm.m.network.curr_operator.mode = (gsm_operator_mode_t)gsmi_parse_number(&str);
    if (*str != '\r') {
        gsm.m.network.curr_operator.format = (gsm_operator_format_t)gsmi_parse_number(&str);
        if (*str != '\r') {
            switch (gsm.m.network.curr_operator.format) {
                case GSM_OPERATOR_FORMAT_LONG_NAME:
                    gsmi_parse_string(&str, gsm.m.network.curr_operator.data.long_name, sizeof(gsm.m.network.curr_operator.data.long_name), 1);
                    break;
                case GSM_OPERATOR_FORMAT_SHORT_NAME:
                    gsmi_parse_string(&str, gsm.m.network.curr_operator.data.short_name, sizeof(gsm.m.network.curr_operator.data.short_name), 1);
                    break;
                case GSM_OPERATOR_FORMAT_NUMBER:
                    gsm.m.network.curr_operator.data.num = GSM_U32(gsmi_parse_number(&str));
                    break;
                default: break;
            }
        }
    } else {
        gsm.m.network.curr_operator.format = GSM_OPERATOR_FORMAT_INVALID;
    }

    if (CMD_IS_DEF(GSM_CMD_COPS_GET) &&
        gsm.msg->msg.cops_get.curr != NULL) {   /* Check and copy to user variable */
        GSM_MEMCPY(gsm.msg->msg.cops_get.curr, &gsm.m.network.curr_operator, sizeof(*gsm.msg->msg.cops_get.curr));
    }
    return 1;
}

/**
 * \brief           Parse +COPS received statement byte by byte
 * \note            Command must be active and message set to use this function
 * \param[in]       ch: New character to parse
 * \param[in]       reset: Flag to reset state machine
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_cops_scan(uint8_t ch, uint8_t reset) {
    static union {
        struct {
            uint8_t bo:1;                       /*!< Bracket open flag (Bracket Open) */
            uint8_t ccd:1;                      /*!< 2 consecutive commas detected in a row (Comma Comma Detected) */
            uint8_t tn:2;                       /*!< Term number in response, 2 bits for 4 diff values */
            uint8_t tp;                         /*!< Current term character position */
            uint8_t ch_prev;                    /*!< Previous character */
        } f;
    } u;

    if (reset) {                                /* Check for reset status */
        GSM_MEMSET(&u, 0x00, sizeof(u));       	/* Reset everything */
        u.f.ch_prev = 0;
        return 1;
    }

    if (u.f.ch_prev == 0) {                     /* Check if this is first character */
        if (ch == ' ') {                        /* Skip leading spaces */
            return 1;
        } else if (ch == ',') {                 /* If first character is comma, no operators available */
            u.f.ccd = 1;                        /* Fake double commas in a row */
        }
    }

    if (u.f.ccd ||                              /* Ignore data after 2 commas in a row */
        gsm.msg->msg.cops_scan.opsi >= gsm.msg->msg.cops_scan.opsl) {   /* or if array is full */
        return 1;
    }

    if (u.f.bo) {                               /* Bracket already open */
        if (ch == ')') {                        /* Close bracket check */
            u.f.bo = 0;                         /* Clear bracket open flag */
            u.f.tn = 0;                         /* Go to next term */
            u.f.tp = 0;                         /* Go to beginning of next term */
            gsm.msg->msg.cops_scan.opsi++;      /* Increase index */
            if (gsm.msg->msg.cops_scan.opf != NULL) {
                *gsm.msg->msg.cops_scan.opf = gsm.msg->msg.cops_scan.opsi;
            }
        } else if (ch == ',') {
            u.f.tn++;                           /* Go to next term */
            u.f.tp = 0;                         /* Go to beginning of next term */
        } else if (ch != '"') {                 /* We have valid data */
            size_t i = gsm.msg->msg.cops_scan.opsi;
            switch (u.f.tn) {
                case 0: {                       /* Parse status info */
                    gsm.msg->msg.cops_scan.ops[i].stat = (gsm_operator_status_t)(10 * (size_t)gsm.msg->msg.cops_scan.ops[i].stat + (ch - '0'));
                    break;
                }
                case 1: {                       /*!< Parse long name */
                    if (u.f.tp < sizeof(gsm.msg->msg.cops_scan.ops[i].long_name) - 1) {
                        gsm.msg->msg.cops_scan.ops[i].long_name[u.f.tp++] = ch;
                        gsm.msg->msg.cops_scan.ops[i].long_name[u.f.tp] = 0;
                    }
                    break;
                }
                case 2: {                       /*!< Parse short name */
                    if (u.f.tp < sizeof(gsm.msg->msg.cops_scan.ops[i].short_name) - 1) {
                        gsm.msg->msg.cops_scan.ops[i].short_name[u.f.tp++] = ch;
                        gsm.msg->msg.cops_scan.ops[i].short_name[u.f.tp] = 0;
                    }
                    break;
                }
                case 3: {                       /*!< Parse number */
                    gsm.msg->msg.cops_scan.ops[i].num = (10 * gsm.msg->msg.cops_scan.ops[i].num) + (ch - '0');
                    break;
                }
                default: break;
            }
        }
    } else {
        if (ch == '(') {                        /* Check for opening bracket */
            u.f.bo = 1;
        } else if (ch == ',' && u.f.ch_prev == ',') {
            u.f.ccd = 1;                        /* 2 commas in a row */
        }
    }
    u.f.ch_prev = ch;
    return 1;
}

/**
 * \brief           Parse datetime in format dd/mm/yy,hh:mm:ss
 * \param[in]       src: Pointer to pointer to input string
 * \param[out]      dt: Date time structure
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_datetime(const char** src, gsm_datetime_t* dt) {
    dt->date = gsmi_parse_number(src);
    dt->month = gsmi_parse_number(src);
    dt->year = GSM_U16(2000) + gsmi_parse_number(src);
    dt->hours = gsmi_parse_number(src);
    dt->minutes = gsmi_parse_number(src);
    dt->seconds = gsmi_parse_number(src);

    gsmi_check_and_trim(src);                   /* Trim text to the end */
    return 1;
}

#if GSM_CFG_CALL || __DOXYGEN__

/**
 * \brief           Parse received +CLCC with call status info
 * \param[in]       str: Input string
 * \param[in]       send_evt: Send event about new CPIN status
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_clcc(const char* str, uint8_t send_evt) {
    if (*str == '+') {
        str += 7;
    }

    gsm.m.call.id = gsmi_parse_number(&str);
    gsm.m.call.dir = (gsm_call_dir_t)gsmi_parse_number(&str);
    gsm.m.call.state = (gsm_call_state_t)gsmi_parse_number(&str);
    gsm.m.call.type = (gsm_call_type_t)gsmi_parse_number(&str);
    gsm.m.call.is_multipart = (gsm_call_type_t)gsmi_parse_number(&str);
    gsmi_parse_string(&str, gsm.m.call.number, sizeof(gsm.m.call.number), 1);
    gsm.m.call.addr_type = gsmi_parse_number(&str);
    gsmi_parse_string(&str, gsm.m.call.name, sizeof(gsm.m.call.name), 1);

    if (send_evt) {
        gsm.evt.evt.call_changed.call = &gsm.m.call;
        gsmi_send_cb(GSM_EVT_CALL_CHANGED);
    }
    return 1;
}

#endif /* GSM_CFG_CALL || __DOXYGEN__ */

#if GSM_CFG_SMS || __DOXYGEN__

/**
 * \brief           Parse string and check for type of SMS state
 * \param[in]       src: Pointer to pointer to string to parse
 * \param[out]      stat: Output status variable
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_sms_status(const char** src, gsm_sms_status_t* stat) {
    gsm_sms_status_t s;
    char t[11];

    gsmi_parse_string(src, t, sizeof(t), 1);    /* Parse string and advance */
    if (!strcmp(t, "REC UNREAD")) {
        s = GSM_SMS_STATUS_UNREAD;
    } else if (!strcmp(t, "REC READ")) {
        s = GSM_SMS_STATUS_READ;
    } else if (!strcmp(t, "STO UNSENT")) {
        s = GSM_SMS_STATUS_UNSENT;
    } else if (!strcmp(t, "REC SENT")) {
        s = GSM_SMS_STATUS_SENT;
    } else {
        s = GSM_SMS_STATUS_ALL;                 /* Error! */
    }
    if (s != GSM_SMS_STATUS_ALL) {
        *stat = s;
        return 1;
    }
    return 0;
}

/**
 * \brief           Parse received +CMGS with last sent SMS memory info
 * \param[in]       str: Input string
 * \param[in]       num: Parsed number in memory
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gsmi_parse_cmgs(const char* str, size_t* num) {
    if (*str == '+') {
        str += 7;
    }

    if (num != NULL) {
        *num = (size_t)gsmi_parse_number(&str);
    }
    return 1;
}

/**
 * \brief           Parse +CMGR statement
 * \todo            Parse date and time from SMS entry
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_cmgr(const char* str) {
    gsm_sms_entry_t* e;
    if (*str == '+') {
        str += 7;
    }

    e = gsm.msg->msg.sms_read.entry;
    e->length = 0;
    gsmi_parse_sms_status(&str, &e->status);
    gsmi_parse_string(&str, e->number, sizeof(e->number), 1);
    gsmi_parse_string(&str, e->name, sizeof(e->name), 1);
    gsmi_parse_datetime(&str, &e->datetime);

    return 1;
}

/**
 * \brief           Parse +CMGL statement
 * \todo            Parse date and time from SMS entry
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_cmgl(const char* str) {
    gsm_sms_entry_t* e;

    if (!CMD_IS_DEF(GSM_CMD_CMGL) ||
        gsm.msg->msg.sms_list.ei >= gsm.msg->msg.sms_list.etr) {
        return 0;
    }

    if (*str == '+') {
        str += 7;
    }

    e = &gsm.msg->msg.sms_list.entries[gsm.msg->msg.sms_list.ei];
    e->length = 0;
    e->mem = gsm.msg->msg.sms_list.mem;         /* Manually set memory */
    e->pos = GSM_SZ(gsmi_parse_number(&str));   /* Scan position */
    gsmi_parse_sms_status(&str, &e->status);
    gsmi_parse_string(&str, e->number, sizeof(e->number), 1);
    gsmi_parse_string(&str, e->name, sizeof(e->name), 1);
    gsmi_parse_datetime(&str, &e->datetime);

    return 1;
}

/**
 * \brief           Parse received +CMTI with received SMS info
 * \param[in]       str: Input string
 * \param[in]       send_evt: Send event about new CPIN status
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_cmti(const char* str, uint8_t send_evt) {
    if (*str == '+') {
        str += 7;
    }

    gsm.evt.evt.sms_recv.mem = gsmi_parse_memory(&str);   /* Parse memory string */
    gsm.evt.evt.sms_recv.pos = gsmi_parse_number(&str);   /* Parse number */

    if (send_evt) {
        gsmi_send_cb(GSM_EVT_SMS_RECV);
    }
    return 1;
}

/**
 * \brief           Parse +CPMS statement
 * \param[in]       str: Input string
 * \param[in]       opt: Expected input: 0 = CPMS_OPT, 1 = CPMS_GET, 2 = CPMS_SET
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_cpms(const char* str, uint8_t opt) {
    uint8_t i;
    if (*str == '+') {
        str += 7;
    }
    switch (opt) {                              /* Check expected input string */
        case 0: {                               /* Get list of CPMS options: +CPMS: (("","","",..),("....")("...")) */
            for (i = 0; i < 3; i++) {           /* 3 different memories for "operation","receive","sent" */
                if (!gsmi_parse_memories_string(&str, &gsm.m.sms.mem[i].mem_available)) {
                    return 0;
                }
            }
            break;
        }
        case 1: {                               /* Received statement of current info: +CPMS: "ME",10,20,"SE",2,20,"... */
            for (i = 0; i < 3; i++) {           /* 3 memories expected */
                gsm.m.sms.mem[i].current = gsmi_parse_memory(&str); /* Parse memory string and save it as current */
                gsm.m.sms.mem[i].used = gsmi_parse_number(&str);/* Get used memory size */
                gsm.m.sms.mem[i].total = gsmi_parse_number(&str);   /* Get total memory size */
            }
            break;
        }
        case 2: {                               /* Received statement of set info: +CPMS: 10,20,2,20 */
            for (i = 0; i < 3; i++) {           /* 3 memories expected */
                gsm.m.sms.mem[i].used = gsmi_parse_number(&str);/* Get used memory size */
                gsm.m.sms.mem[i].total = gsmi_parse_number(&str);   /* Get total memory size */
            }
            break;
        }
        default: break;
    }
    return 1;
}

#endif /* GSM_CFG_SMS || __DOXYGEN__ */

#if GSM_CFG_PHONEBOOK || __DOXYGEN__

/**
 * \brief           Parse +CPBS statement
 * \param[in]       str: Input string
 * \param[in]       opt: Expected input: 0 = CPBS_OPT, 1 = CPBS_GET, 2 = CPBS_SET
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_cpbs(const char* str, uint8_t opt) {
    if (*str == '+') {
        str += 7;
    }
    switch (opt) {                              /* Check expected input string */
        case 0: {                               /* Get list of CPBS options: ("M1","M2","M3",...) */
            return gsmi_parse_memories_string(&str, &gsm.m.pb.mem.mem_available);
        }
        case 1: {                               /* Received statement of current info: +CPBS: "ME",10,20 */
            gsm.m.pb.mem.current = gsmi_parse_memory(&str); /* Parse memory string and save it as current */
            gsm.m.pb.mem.used = gsmi_parse_number(&str);/* Get used memory size */
            gsm.m.pb.mem.total = gsmi_parse_number(&str);   /* Get total memory size */
            break;
        }
        case 2: {                               /* Received statement of set info: +CPBS: 10,20 */
            gsm.m.pb.mem.used = gsmi_parse_number(&str);/* Get used memory size */
            gsm.m.pb.mem.total = gsmi_parse_number(&str);   /* Get total memory size */
            break;
        }
    }
    return 1;
}

/**
 * \brief           Parse +CPBR statement
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_cpbr(const char* str) {
    gsm_pb_entry_t* e;

    if (!CMD_IS_DEF(GSM_CMD_CPBR) ||
        gsm.msg->msg.pb_list.ei >= gsm.msg->msg.pb_list.etr) {
        return 0;
    }

    if (*str == '+') {
        str += 7;
    }

    e = &gsm.msg->msg.pb_list.entries[gsm.msg->msg.pb_list.ei];
    e->pos = GSM_SZ(gsmi_parse_number(&str));
    gsmi_parse_string(&str, e->name, sizeof(e->name), 1);
    e->type = (gsm_number_type_t)gsmi_parse_number(&str);
    gsmi_parse_string(&str, e->number, sizeof(e->number), 1);

    gsm.msg->msg.pb_list.ei++;
    if (gsm.msg->msg.pb_list.er != NULL) {
        *gsm.msg->msg.pb_list.er = gsm.msg->msg.pb_list.ei;
    }
    return 1;
}

/**
 * \brief           Parse +CPBF statement
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_cpbf(const char* str) {
    gsm_pb_entry_t* e;

    if (!CMD_IS_DEF(GSM_CMD_CPBF) ||
        gsm.msg->msg.pb_search.ei >= gsm.msg->msg.pb_search.etr) {
        return 0;
    }

    if (*str == '+') {
        str += 7;
    }

    e = &gsm.msg->msg.pb_search.entries[gsm.msg->msg.pb_search.ei];
    e->pos = GSM_SZ(gsmi_parse_number(&str));
    gsmi_parse_string(&str, e->name, sizeof(e->name), 1);
    e->type = (gsm_number_type_t)gsmi_parse_number(&str);
    gsmi_parse_string(&str, e->number, sizeof(e->number), 1);

    gsm.msg->msg.pb_search.ei++;
    if (gsm.msg->msg.pb_search.er != NULL) {
        *gsm.msg->msg.pb_search.er = gsm.msg->msg.pb_search.ei;
    }
    return 1;
}

#endif /* GSM_CFG_PHONEBOOK || __DOXYGEN__ */

#if GSM_CFG_CONN

/**
 * \brief           Parse connection info line from CIPSTATUS command
 * \param[in]       str: Input string
 * \param[in]       is_conn_line: Set to `1` for connection, `0` for general status
 * \param[out]      continueScan: Pointer to output variable holding continue processing state
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gsmi_parse_cipstatus_conn(const char* str, uint8_t is_conn_line, uint8_t* continueScan) {
    uint8_t num;
    gsm_conn_t* conn;
    char s_tmp[16];
    uint8_t tmp_pdp_state;

    *continueScan = 1;
    if (is_conn_line && (*str == 'C' || *str == 'S')) {
        str += 3;
    } else {
        /* Check if PDP context is deactivated or not */
        tmp_pdp_state = 1;
        if (!strncmp(&str[7], "IP INITIAL", 10)) {
            *continueScan = 0;                  /* Stop command execution at this point (no OK,ERROR received after this line) */
            tmp_pdp_state = 0;
        } else if (!strncmp(&str[7], "PDP DEACT", 9)) {
            /* Deactivated */
            tmp_pdp_state = 0;
        }

        /* Check if we have to update status for application */
        if (gsm.m.network.is_attached != tmp_pdp_state) {
            gsm.m.network.is_attached = tmp_pdp_state;

            /* Notify upper layer */
            gsmi_send_cb(gsm.m.network.is_attached ? GSM_EVT_NETWORK_ATTACHED : GSM_EVT_NETWORK_DETACHED);
        }

        return 1;
    }

    /* Parse connection line */
    num = GSM_U8(gsmi_parse_number(&str));
    conn = &gsm.m.conns[num];

    conn->status.f.bearer = GSM_U8(gsmi_parse_number(&str));
    gsmi_parse_string(&str, s_tmp, sizeof(s_tmp), 1);   /* Parse TCP/UPD */
    if (strlen(s_tmp)) {
        if (!strcmp(s_tmp, "TCP")) {
            conn->type = GSM_CONN_TYPE_TCP;
        } else if (!strcmp(s_tmp, "UDP")) {
            conn->type = GSM_CONN_TYPE_UDP;
        }
    }
    gsmi_parse_ip(&str, &conn->remote_ip);
    conn->remote_port = gsmi_parse_number(&str);

    /* Get connection status */
    gsmi_parse_string(&str, s_tmp, sizeof(s_tmp), 1);

    /* TODO: Implement all connection states */
    if (!strcmp(s_tmp, "INITIAL")) {

    } else if (!strcmp(s_tmp, "CONNECTING")) {

    } else if (!strcmp(s_tmp, "CONNECTED")) {

    } else if (!strcmp(s_tmp, "REMOTE CLOSING")) {

    } else if (!strcmp(s_tmp, "CLOSING")) {

    } else if (!strcmp(s_tmp, "CLOSED")) {      /* Connection closed */
        if (conn->status.f.active) {            /* Check if connection is not */
            gsmi_conn_closed_process(conn->num, 0); /* Process closed event */
        }
    }

    /* Save last parsed connection */
    gsm.m.active_conns_cur_parse_num = num;

    return 1;
}

/**
 * \brief           Parse IPD or RECEIVE statements
 * \param[in]       str: Input string
 * \return          `1` on success, `0` otherwise
 */
uint8_t
gsmi_parse_ipd(const char* str) {
    uint8_t conn;
    size_t len;
    gsm_conn_p c;

    if (*str == '+') {
        str++;
        if (*str == 'R') {
            str += 8;                           /* Advance for RECEIVE */
        } else {
            str += 4;                           /* Advance for IPD */
        }
    }

    conn = gsmi_parse_number(&str);             /* Parse number for connection number */
    len = gsmi_parse_number(&str);              /* Parse number for number of bytes to read */

    c = conn < GSM_CFG_MAX_CONNS ? &gsm.m.conns[conn] : NULL;   /* Get connection handle */
    if (c == NULL) {                            /* Invalid connection number */
        return 0;
    }

    gsm.m.ipd.read = 1;                         /* Start reading network data */
    gsm.m.ipd.tot_len = len;                    /* Total number of bytes in this received packet */
    gsm.m.ipd.rem_len = len;                    /* Number of remaining bytes to read */
    gsm.m.ipd.conn = c;                         /* Pointer to connection we have data for */

    return 1;
}

#endif /* GSM_CFG_CONN */

#if GSM_SEQUANS_SPECIFIC_CMD
/**
 * \brief           Parse +SQNDNSLKUP statement
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_sqndnslkup(const char* str) {

	if(gsm.m.conn_val_id > GSM_CFG_MAX_CONNS)
	{
		return 0;
	}

	gsm_ip_t* ip = &gsm.m.conns[gsm.m.conn_val_id].remote_ip;   /* Get IP data structure from connection handle */

    if (*str == '+') {
        str += 13;
    }

    gsmi_get_next_param_pos(&str);
    gsmi_parse_ip(&str, ip);

    return 1;
}

extern void
gsmi_receive_raw(const char* str , uint8_t connid , uint32_t rx_size );
/**
 * \brief           Parse +SNQSRING
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_rcvdata_update(const char* str)
{
	uint32_t conn_id, read_count;
	char * ptx = (char *) str;

	if(gsm.m.conn_val_id > GSM_CFG_MAX_CONNS)
	{
		return 0;
	}

	/* Jump directly to the number by skipping '+SQNSRING: ' */
	ptx += 11;

	conn_id = ( uint32_t ) gsmi_parse_number( (const char**) &ptx);
	read_count = ( uint32_t ) gsmi_parse_number( (const char**) &ptx);

	/* store pending data info */
	if( (read_count << 1) > (AT_BUFFER_SIZE << 1) )
	{
		uint32_t bp = read_count;
		uint32_t cid = conn_id;

		while(bp > (AT_BUFFER_SIZE >> 1) )
		{
			gsm_ring_list_insert_elem(gsm.m.ring_list, (AT_BUFFER_SIZE >> 1), cid, -1);
			bp -= AT_BUFFER_SIZE >> 1;
		}
		gsm_ring_list_insert_elem(gsm.m.ring_list, bp, cid, -1);
	}
	else
	{
		gsm_ring_list_insert_elem(gsm.m.ring_list, read_count, conn_id, -1);
	}


    return 1;
}

/**
 * \brief           Parse +SQNSRECV
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint8_t
gsmi_parse_rcvdata_ntf(const char* str, uint8_t *conn_id, uint32_t *bytes_pending)
{
#define SQNSRECV_JUMP	11U
	uint32_t ret = 0;
	char * ptx = (char *) str;

	if(gsm.m.conn_val_id > GSM_CFG_MAX_CONNS)
	{
		return 0;
	}

	/* Jump directly to the number by skipping '+SQNSRECV: ' */
	ptx += SQNSRECV_JUMP;

	*conn_id = ( uint32_t ) gsmi_parse_number( (const char**) &ptx);
	*bytes_pending = ( uint32_t ) gsmi_parse_number( (const char**) &ptx);

	ret = 1;

	gsm_ring_list_delete_elem(gsm.m.ring_list, 0);
	gsm.m.ring_list->is_at_sqnsrecv_ongoing = 0;

	return ret;
}


/**
 * \brief           Parse +SQNSRING
 * \param[in]       str: Input string
 * \return          1 on success, 0 otherwise
 */
uint32_t
gsmi_handle_recv_string(const char * str, uint8_t ring_recv )
{
//	uint8_t conn_id;
	uint32_t read_count;

	char * ptr = strstr("+SQNSRING:", str );

	if( ptr != NULL )
	{
		ptr += 11;

		/*conn_id = */( uint32_t ) gsmi_parse_number( (const char**) &ptr);
		read_count = ( uint32_t ) gsmi_parse_number( (const char**) &ptr);
		/* read_count = total nb of bytes from the URC to the last character of the received data
		 * e.g +SQNSRING:1,10,0A0B0CFE123456789ABC
		 */
		read_count = ( ptr - str + 1 ) +  read_count * 2;
		ring_recv = 1;
	}
	else
	{
		read_count = 0;
		ring_recv = 0;
	}


	return read_count;
}

/**
 * \brief           Send AT+SQNSRECV when +SQNSRING has been received
 * \return          1 if Success, 0 otherwise
 */
uint32_t
gsmi_send_sqnsrecv(void)
{
	uint32_t status = 0;
	st_RingElem *element = gsm.m.ring_list->first_ring;

	if(sRXData[element->connid - 1][u8_nextFreeBufferPool].BytesPending == 0)
	{
		AT_PORT_SEND_BEGIN_AT();
		AT_PORT_SEND_CONST_STR("+SQNSRECV=");
		gsmi_send_number(GSM_U32(element->connid), 0, 0);
		gsmi_send_number(GSM_U32(element->BytesPending), 0, 1);
		AT_PORT_SEND_END_AT();

		/* TODO: Need to implement a timeout
		 * Here, a timer should start
		 */
		gsm.m.ring_list->is_at_sqnsrecv_ongoing = 1;
		status = 1;
	}

	return status;
}


#endif /* GSM_SEQUANS_SPECIFIC_CMD */
