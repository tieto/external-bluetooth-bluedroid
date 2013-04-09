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
 *  Filename:      btif_test_rfcomm.c
 *
 *  Description:   Bluetooth RFCOMM Test Interface
 *
 *
 ***********************************************************************************/

#define LOG_TAG "BTIF_TEST_RFCOMM"
#include <cutils/log.h>

#include <hardware/bluetooth.h>
#include <hardware/bt_test.h>
#include "../gki/common/gki.h"



const rfcomm_test_interface_t rfcomm_interface = {

};

const rfcomm_test_interface_t *get_test_rfcomm_interface()
{
    return &rfcomm_interface;
}
