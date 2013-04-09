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
 *  Filename:      btif_test_tester.c
 *
 *  Description:   Bluetooth TESTER Test Interface
 *
 *
 ***********************************************************************************/

#define LOG_TAG "BTIF_TEST_TESTER"
#include <cutils/log.h>

#include <hardware/bluetooth.h>
#include <hardware/bt_test.h>
#include "../gki/common/gki.h"

#include <errno.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>
#include <unistd.h>

extern int hci_btsnoop_fd;

static int dump(unsigned int type, char *name)
{
    static int server_socket = 0;
    int fd;

    if (type == DUMP_TYPE_STOP) {
        if (server_socket) {
            close(server_socket);
            server_socket = 0;
        }

        if (hci_btsnoop_fd == -1)
            return 1;

        fd = hci_btsnoop_fd;
        hci_btsnoop_fd = -1;
        close(fd);

        return 0;
    } else if (type == DUMP_TYPE_FILE) {
        if (hci_btsnoop_fd != -1)
            return 1;
        fd = open(name, O_WRONLY | O_CREAT | O_TRUNC,
                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH);
        if (fd != 0) {
            write(fd, "btsnoop\0\0\0\0\1\0\0\x3\xea", 16);
            hci_btsnoop_fd = fd;
            return 0;
        }
    } else if (type == DUMP_TYPE_SOCKET) {
        int                 socket_remote;
        struct sockaddr_un  local;
        struct sockaddr     remote;
        socklen_t           len;

        if (hci_btsnoop_fd != -1)
            return 1;

        server_socket = socket(AF_UNIX, SOCK_STREAM , 0);

        local.sun_family = AF_UNIX;
        strcpy(local.sun_path, name);
        unlink(local.sun_path);
        len = strlen(local.sun_path) + sizeof(local.sun_family);

        if (bind(server_socket, (struct sockaddr *) &local, len) == -1) {
            printf("Socket fail (1: errno %i)\n", errno);
            return -2;
        }

        if (listen(server_socket, 5) == -1) {
            printf("Socket fail (2: errno %i)\n", errno);
            return -2;
        }

        len = sizeof(struct sockaddr_un);
        socket_remote = accept(server_socket, &remote, &len);

        write(socket_remote, "btsnoop\0\0\0\0\1\0\0\x3\xea", 16);
        hci_btsnoop_fd = socket_remote;
    }

    return -1;
}

const tester_test_interface_t tester_interface = {
    .dump  = &dump,
};

const tester_test_interface_t *get_test_tester_interface(void)
{
    return &tester_interface;
}
