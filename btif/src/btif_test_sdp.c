/******************************************************************************
 *
 *  Copyright (C) 2013 Tieto Corporation
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at:
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 ******************************************************************************/


/************************************************************************************
 *
 *  Filename:      btif_test_sdp.c
 *
 *  Description:   Bluetooth SDP Test Interface
 *
 *
 ***********************************************************************************/

#define LOG_TAG "BTIF_TEST_SDP"
#include <cutils/log.h>

#include <hardware/bluetooth.h>
#include <hardware/bt_test.h>
#include "../gki/common/gki.h"
#include "../stack/sdp/sdpint.h"
#include "bta_api.h"

#include "btif_sock_sdp.h"

extern int32_t add_rfc_sdp_rec(const char* name, const uint8_t *uuid, int scn);

static tCONN_CB* _connect(UINT8 *p_bd_addr)
{
    tCONN_CB *p_ccb;
    UINT16    cid;

    /* Allocate a new CCB. Return if none available. */
    if ((p_ccb = sdpu_allocate_ccb()) == NULL)
    {
        SDP_TRACE_WARNING0 ("SDP - no spare CCB for orig");
        return (NULL);
    }

    SDP_TRACE_EVENT0 ("SDP - Originate started");

    /* We are the originator of this connection */
//    p_ccb->con_flags |= SDP_FLAGS_IS_ORIG;

    /* Save the BD Address and Channel ID. */
    memcpy (&p_ccb->device_address[0], p_bd_addr, sizeof (BD_ADDR));

    /* Transition to the next appropriate state, waiting for connection confirm. */
    p_ccb->con_state = SDP_STATE_CONN_SETUP;

// btla-specific ++
#ifndef ANDROID_APP_INCLUDED  /* Skip for Android: Do not need to set
                               * out_service for sdp, since sdp does not use sec.
                               * Prevents over-writing service_rec of a
                               * connection already in progress */
    BTM_SetOutService(p_bd_addr, BTM_SEC_SERVICE_SDP_SERVER, 0);
#endif
// btla-specific --

    cid = L2CA_ConnectReq (SDP_PSM, p_bd_addr);

    /* Check if L2CAP started the connection process */
    if (cid != 0)
    {
        p_ccb->connection_id = cid;
        return (p_ccb);
    }
    else
    {
        SDP_TRACE_WARNING0 ("SDP - Connection failed");
        sdpu_release_ccb (p_ccb);
        return (NULL);
    }
}

static uint32_t get_number_of_records(void)
{
    return (uint32_t) sdp_cb.server_db.num_records;
}

static uint32_t get_max_number_of_records(void)
{
    return SDP_MAX_RECORDS;
}

static uint32_t create_record(void)
{
    return SDP_CreateRecord();
}

static uint32_t delete_record(uint32_t handle)
{
    return SDP_DeleteRecord(handle);
}

static uint32_t connect(uint8_t bd_addr[6])
{
#if SDP_CLIENT_ENABLED == TRUE
    tCONN_CB     *p_ccb;
    uint32_t      idx = 0;

    p_ccb = _connect(bd_addr);
    if (!p_ccb)
        return idx;

    idx = (uint32_t)(p_ccb - sdp_cb.ccb);

    return (uint32_t)(idx + 1);
#endif
    return 0;
}

static uint32_t disconnect(uint32_t handle)
{
#if SDP_CLIENT_ENABLED == TRUE
    tCONN_CB *p_ccb = NULL;

    if (handle > 0 && handle <= SDP_MAX_CONNECTIONS)
    {
        p_ccb = &sdp_cb.ccb[handle - 1];
        sdp_disconnect (p_ccb, SDP_SUCCESS);
        return TRUE;
    }
#endif
    return FALSE;
}

static uint32_t send(uint32_t handle, uint32_t length, uint8_t *data)
{
#if SDP_CLIENT_ENABLED == TRUE
    BT_HDR    *p_msg;
    UINT8     *p_data;
    UINT32     i;
    tCONN_CB  *p_ccb = NULL;

    if ((p_msg = (BT_HDR *) GKI_getpoolbuf(SDP_POOL_ID)) == NULL)
    {
        return FALSE;
    }

    p_msg->len = length;
    p_msg->offset = L2CAP_MIN_OFFSET;

    p_data = (UINT8 *)(p_msg + 1) + p_msg->offset;
    for (i = 0; i < length; i += 1)
        UINT8_TO_BE_STREAM(p_data, data[i]);

    if (p_msg && (handle > 0) && (handle <= SDP_MAX_CONNECTIONS) )
    {
        p_ccb = &sdp_cb.ccb[handle - 1];
        if (p_ccb->con_state == SDP_STATE_CONNECTED)
        {
            L2CA_DataWrite (p_ccb->connection_id, p_msg);
            return TRUE;
        }
    }
#endif
    return FALSE;
}

static void sdp_get_remote_records_callback(UINT16 status)
{
    printf("sdp_get_remote_records_callback status: %d", status);

    return;
}

static uint8_t *get_remote_records(uint8_t bd_addr[6], uint8_t type,
                      union btt_msg_cmd_sdp_print_remote_records_data data)
{
    struct sdp_test_records *records;
    unsigned int attribute_offset;
    unsigned int value_offset;
    unsigned int data_size;
    tSDP_UUID uuid_list;
    BOOLEAN   result = TRUE;
    tSDP_DISCOVERY_DB  *db;

    value_offset = 0;
    data_size = 0;

    /*TODO implement get_number_of_remote_records() to replace '0' */
    attribute_offset = sizeof(struct sdp_test_records) +
                       0 * sizeof(struct sdp_test_record);
    value_offset += attribute_offset;
    records = malloc(value_offset + data_size);

    records->size = value_offset + data_size;
    records->records_num = 0;

    return (uint8_t *) records;
}

static uint8_t *get_records()
{
    struct sdp_test_records          *records;
    struct sdp_test_record_attribute *attribute;
    unsigned int value_offset;
    unsigned int attribute_offset;
    unsigned int data_size;
    unsigned int i_record;
    unsigned int i_attribute;

    value_offset = 0;
    data_size = 0;
    for (i_record = 0; i_record < get_number_of_records(); i_record += 1) {
        value_offset += sdp_cb.server_db.record[i_record].num_attributes *
                        sizeof(struct sdp_test_record_attribute);
        for (i_attribute = 0;
             i_attribute < sdp_cb.server_db.record[i_record].num_attributes;
             i_attribute+=1) {
            data_size +=
                sdp_cb.server_db.record[i_record].attribute[i_attribute].len;
        }
    }

    attribute_offset = sizeof(struct sdp_test_records) +
                       get_number_of_records() * sizeof(struct sdp_test_record);
    value_offset += attribute_offset;
    records = malloc(value_offset + data_size);

    records->size = value_offset + data_size;
    records->records_num = get_number_of_records();

    for (i_record = 0; i_record < records->records_num; i_record += 1) {
        records->record[i_record].attributes_num =
                              sdp_cb.server_db.record[i_record].num_attributes;
        records->record[i_record].attribute_offset = attribute_offset;

        for (i_attribute = 0;
             i_attribute < records->record[i_record].attributes_num;
             i_attribute+=1) {
            attribute = (struct sdp_test_record_attribute *)((char *)records +
                                    records->record[i_record].attribute_offset);
            attribute[i_attribute].id   =
                    sdp_cb.server_db.record[i_record].attribute[i_attribute].id;
            attribute[i_attribute].type =
                    sdp_cb.server_db.record[i_record].attribute[i_attribute].type;
            attribute[i_attribute].size =
                    sdp_cb.server_db.record[i_record].attribute[i_attribute].len;
            attribute[i_attribute].value_offset = value_offset;

            memcpy(((char *)records + value_offset),
                   sdp_cb.server_db.record[i_record].attribute[i_attribute].value_ptr,
                   attribute[i_attribute].size);
            value_offset += attribute[i_attribute].size;
        }

        attribute_offset += records->record[i_record].attributes_num
                            * sizeof(struct sdp_test_record_attribute);
    }

    return (uint8_t *)records;
}

static uint32_t add_attribute(uint32_t handle, uint16_t attribute_id,
        uint8_t attribute_type, uint32_t attribute_length,
        uint8_t *attribute_value)
{
    return SDP_AddAttribute(handle, attribute_id, attribute_type,
            attribute_length, attribute_value);
}

static uint32_t delete_attribute(uint32_t handle, uint16_t attribute_id)
{
    return SDP_DeleteAttribute(handle, attribute_id);
}

static int32_t add_rfcomm_record(const char* name, const uint8_t *uuid, int scn)
{
    return add_rfc_sdp_rec(name, uuid, scn);
}

static uint8_t trace_level(uint8_t level)
{
    return SDP_SetTraceLevel(level);
}

const sdp_test_interface_t sdp_interface = {
    .get_number_of_records     = &get_number_of_records,
    .get_max_number_of_records = &get_max_number_of_records,
    .create_record             = &create_record,
    .delete_record             = &delete_record,
    .add_attribute             = &add_attribute,
    .delete_attribute          = &delete_attribute,
    .get_records               = &get_records,
    .get_remote_records        = &get_remote_records,
    .add_rfcomm_record         = &add_rfcomm_record,
    .trace_level               = &trace_level,
    .connect                   = &connect,
    .disconnect                = &disconnect,
    .send                      = &send,
};

const sdp_test_interface_t *get_test_sdp_interface(void)
{
    return &sdp_interface;
}
