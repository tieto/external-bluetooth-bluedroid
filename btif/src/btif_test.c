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

#include <hardware/bluetooth.h>
#include <hardware/bt_test.h>

#define LOG_TAG "BTIF_TEST"

extern btl2cap_interface_t     *get_test_l2cap_interface();
extern sdp_test_interface_t    *get_test_sdp_interface();
extern rfcomm_test_interface_t *get_test_rfcomm_interface();
extern tester_test_interface_t *get_test_tester_interface();

static const void *get_interface(const int i)
{
    if (i == TEST_L2CAP)
        return get_test_l2cap_interface();
    else if (i == TEST_SDP)
        return get_test_sdp_interface();
    else if (i == TEST_RFCOMM)
        return get_test_rfcomm_interface();
    else if (i == TEST_TESTER)
        return get_test_tester_interface();
    else
        return NULL;
}

static test_interface_t test_interface = {
    get_interface
};

const test_interface_t *btif_test_get_interface(){
    return &test_interface;
}
