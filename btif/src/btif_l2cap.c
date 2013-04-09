/*
 * Copyright (C) 2013 Tieto Corporation
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>
#include <stdbool.h>
#include <hardware/bluetooth.h>
#include <hardware/bt_test.h>
#include <hardware/bt_test_l2cap.h>

#define LOG_TAG "BTIF_L2CAP_TEST"
#include "bta_jv_api.h"
#include "btif_api.h"
#include "bt_utils.h"
#include "../gki/common/gki.h"
#include "l2c_api.h"

static btl2cap_callback_function_t cback;

static bool interface_ready(void)
{
    return cback == NULL ? FALSE : TRUE;
}

static void conn_ind_cb(BD_ADDR addr, UINT16 cid, UINT16 psm, UINT8 id)
{
    bt_l2cap_callback_t callback;
    bt_l2cap_conn_ind_t data;

    if (!interface_ready())
        return;

    data.addr = (bt_bdaddr_t *)addr;
    data.cid  = cid;
    data.psm  = psm;
    data.id   = id;

    callback.type = CONN_IND_CB;
    callback.len  = sizeof(bt_l2cap_conn_ind_t);
    callback.val  = &data;

    cback(&callback);
}

static void conn_cfm_cb(UINT16 cid, UINT16 result)
{
    bt_l2cap_callback_t callback;
    bt_l2cap_int_t      data;

    if (!interface_ready())
        return;

    data.cid    = cid;
    data.result = result;

    callback.type = CONN_CFM_CB;
    callback.len  = sizeof(bt_l2cap_int_t);
    callback.val  = &data;

    cback(&callback);
}

static void conn_pnd_cb(UINT16 cid)
{
    bt_l2cap_callback_t callback;

    if (!interface_ready())
        return;

    callback.type = CONN_PND_CB;
    callback.len  = sizeof(uint16_t);
    callback.val  = &cid;

    cback(&callback);
}

static void conf_ind_cb(UINT16 cid, tL2CAP_CFG_INFO *config)
{
    bt_l2cap_callback_t callback;
    bt_l2cap_cfg_t      data;

    if (!interface_ready())
        return;

    data.cid = cid;
    data.cfg = (tl2cap_cfg_info_t *)config;

    callback.type = CONF_IND_CB;
    callback.len  = sizeof(bt_l2cap_cfg_t);
    callback.val  = &data;

    cback(&callback);
}

static void conf_cfm_cb(UINT16 cid, tL2CAP_CFG_INFO *config)
{
    bt_l2cap_callback_t callback;
    bt_l2cap_cfg_t      data;

    if (!interface_ready())
        return;

    data.cid = cid;
    data.cfg = (tl2cap_cfg_info_t *)config;

    callback.type = CONF_CFM_CB;
    callback.len  = sizeof(bt_l2cap_cfg_t);
    callback.val  = &data;

    cback(&callback);
}

static void disc_ind_cb(UINT16 cid, BOOLEAN result)
{
    bt_l2cap_callback_t callback;
    bt_l2cap_bool_cb_t  data;

    if (!interface_ready())
        return;

    data.cid    = cid;
    data.result = result;

    callback.type = DISC_IND_CB;
    callback.len  = sizeof(bt_l2cap_bool_cb_t);
    callback.val  = &data;

    cback(&callback);
}

static void disc_cfm_cb(UINT16 cid, UINT16 result)
{
    bt_l2cap_callback_t callback;
    bt_l2cap_int_t      data;

    if (!interface_ready())
        return;

    data.cid    = cid;
    data.result = result;

    callback.type = DISC_CFM_CB;
    callback.len  = sizeof(bt_l2cap_int_t);
    callback.val  = &data;

    cback(&callback);
}

static void qos_viol_ind_cb(BD_ADDR addr)
{
    bt_l2cap_callback_t callback;

    if (!interface_ready())
        return;

    callback.type = QOS_VIOL_IND_CB;
    callback.len  = sizeof(bt_bdaddr_t);
    callback.val  = addr;

    cback(&callback);
}

static void data_ind_cb(UINT16 cid, BT_HDR *buf)
{
    bt_l2cap_callback_t callback;
    bt_l2cap_data_t     data;

    if (!interface_ready())
        return;

    data.cid = cid;
    data.buf = (bt_hdr_t *)buf;

    callback.type = DATA_IND_CB;
    callback.len  = sizeof(bt_l2cap_data_t);
    callback.val  = &data;

    cback(&callback);
}

static void cong_stat_cb(UINT16 cid, BOOLEAN status)
{
    bt_l2cap_callback_t callback;
    bt_l2cap_bool_cb_t  data;

    if (!interface_ready())
        return;

    data.cid    = cid;
    data.result = status;

    callback.type = CONG_STAT_CB;
    callback.len  = sizeof(bt_l2cap_bool_cb_t);
    callback.val  = &data;

    cback(&callback);
}

static void tx_complete_cb(UINT16 cid, UINT16 sdu)
{
    bt_l2cap_callback_t callback;
    bt_l2cap_int_t      data;

    if (!interface_ready())
        return;

    data.cid    = cid;
    data.result = sdu;

    callback.type = TX_COMPLETE_CB;
    callback.len  = sizeof(bt_l2cap_int_t);
    callback.val  = &data;

    cback(&callback);
}

static tL2CAP_APPL_INFO l2cap_cbacks = {
    .pL2CA_ConnectInd_Cb       = &conn_ind_cb,
    .pL2CA_ConnectCfm_Cb       = &conn_cfm_cb,
    .pL2CA_ConnectPnd_Cb       = &conn_pnd_cb,
    .pL2CA_ConfigInd_Cb        = &conf_ind_cb,
    .pL2CA_ConfigCfm_Cb        = &conf_cfm_cb,
    .pL2CA_DisconnectInd_Cb    = &disc_ind_cb,
    .pL2CA_DisconnectCfm_Cb    = &disc_cfm_cb,
    .pL2CA_QoSViolationInd_Cb  = &qos_viol_ind_cb,
    .pL2CA_DataInd_Cb          = &data_ind_cb,
    .pL2CA_CongestionStatus_Cb = &cong_stat_cb,
    .pL2CA_TxComplete_Cb       = &tx_complete_cb,
};

uint16_t register_psm(int psm, btl2cap_callback_function_t callback)
{
    int ret;
    cback = callback;

    BTM_SetSecurityLevel(TRUE, "", 0, BTM_SEC_NONE, psm, 0, 0);
    BTM_SetSecurityLevel(FALSE, "", 0, BTM_SEC_NONE, psm, 0, 0);

    ret = L2CA_Register(psm, &l2cap_cbacks);

    if (ret == 0) {
        cback == NULL;
        return 0;
    } else {
        return ret;
    }
}

void deregister_psm(int psm)
{
    L2CA_Deregister(psm);
}

uint16_t allocate_psm(void)
{
    if (!interface_ready())
        return 0;

    return L2CA_AllocatePSM();
}

uint16_t connect_req(uint16_t psm, bt_bdaddr_t *addr)
{
    if (!interface_ready())
        return 0;

    return L2CA_ConnectReq(psm, (UINT8 *)addr);
}

bool connect_rsp(bt_bdaddr_t *addr, uint8_t id, uint16_t lcid,
        uint16_t result, uint16_t status)
{
    if (!interface_ready())
        return false;

    return L2CA_ConnectRsp((UINT8 *)addr, id, lcid, result, status);
}

bool config_req(uint16_t cid, tl2cap_cfg_info_t *config)
{
    if (!interface_ready())
        return false;

    return L2CA_ConfigReq(cid, (tL2CAP_CFG_INFO *)config);
}

bool config_rsp(uint16_t cid, tl2cap_cfg_info_t *config)
{
    if (!interface_ready())
        return false;

    return L2CA_ConfigRsp(cid, (tL2CAP_CFG_INFO *)config);
}

bool disconnect_req(uint16_t cid)
{
    if (!interface_ready())
        return false;

    return L2CA_DisconnectReq(cid);
}

bool disconnect_rsp(uint16_t cid)
{
    if (!interface_ready())
        return false;

    return L2CA_DisconnectRsp(cid);
}

uint8_t write_data(uint16_t cid, uint32_t length, uint8_t *buf)
{
    BT_HDR *p_msg;
    UINT8 *p_data;
    uint32_t i;

    if (!interface_ready())
        return 0;

    if ((p_msg = (BT_HDR *) GKI_getpoolbuf(HCI_ACL_POOL_ID)) == NULL)
        return -1;

    p_msg->len = length;
    p_msg->offset = L2CAP_MIN_OFFSET;
    p_data = (UINT8 *)(p_msg + 1) + p_msg->offset;

    for (i = 0; i < length; i += 1)
        UINT8_TO_BE_STREAM(p_data, buf[i]);

    return L2CA_DataWrite(cid, p_msg);
}

bool send_test_s_frame(uint16_t cid, uint8_t sup_type, uint8_t back_track)
{
    if (!interface_ready())
        return false;

    return L2CA_SendTestSFrame(cid, sup_type, back_track);
}

bool ping(uint8_t *addr, echo_response response)
{
    if (response == NULL)
        return false;

    return L2CA_Ping(addr, response);
}

static const btl2cap_interface_t l2cap_interface = {
    .register_psm = &register_psm,
    .deregister_psm = &deregister_psm,
    .allocate_psm = &allocate_psm,
    .connect_req = &connect_req,
    .connect_rsp = &connect_rsp,
    .config_req = &config_req,
    .config_rsp = &config_rsp,
    .disconnect_req = &disconnect_req,
    .disconnect_rsp = &disconnect_rsp,
    .write_data = &write_data,
    .send_test_s_frame = &send_test_s_frame,
    .ping = &ping,
};

const btl2cap_interface_t *get_test_l2cap_interface()
{
    return &l2cap_interface;
}
