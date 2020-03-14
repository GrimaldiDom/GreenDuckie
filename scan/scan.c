//
//  Intel Edison Playground
//  Copyright (c) 2015 Damian Kołakowski. All rights reserved.
//

#include <stdlib.h>
#include <errno.h>
#include <curses.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <sys/time.h>               // For timestamping

#define PORT 55123
#define DATA_SIZE 40
#define IP_RECV "192.168.1.99"

struct hci_request ble_hci_request(uint16_t ocf, int clen, void * status, void * cparam)
{
    struct hci_request rq;
    memset(&rq, 0, sizeof(rq));
    rq.ogf = OGF_LE_CTL;
    rq.ocf = ocf;
    rq.cparam = cparam;
    rq.clen = clen;
    rq.rparam = status;
    rq.rlen = 1;
    return rq;
}

void getDevice( int* device )
{
    // Get HCI device.
    *device = hci_open_dev(hci_get_route(NULL));
    if ( *device < 0 ) { 
        perror("Failed to open HCI device.");
        exit(0); 
    }
}
void setScanParamsCp( le_set_scan_parameters_cp* scan_params_cp )
{
    // Set BLE scan parameters.
    scan_params_cp->type                = 0x00; 
    scan_params_cp->interval            = htobs(0x0010);
    scan_params_cp->window              = htobs(0x0010);
    scan_params_cp->own_bdaddr_type     = 0x00; // Public Device Address (default).
    scan_params_cp->filter              = 0x00; // Accept all.
}

int main()
{
    // ************** INIT SOCKET CODE *************
    int sock = 0, valread;
    struct sockaddr_in serv_addr;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    if(inet_pton(AF_INET, IP_RECV, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    int ret, status;
    int device;
    getDevice( &device );

    le_set_scan_parameters_cp scan_params_cp;
    memset(&scan_params_cp, 0, sizeof(scan_params_cp));
    setScanParamsCp( &scan_params_cp );

    struct hci_request scan_params_rq = ble_hci_request(OCF_LE_SET_SCAN_PARAMETERS, LE_SET_SCAN_PARAMETERS_CP_SIZE, &status, &scan_params_cp);
    
    ret = hci_send_req(device, &scan_params_rq, 0);
    if ( ret < 0 ) {
        hci_close_dev(device);
        perror("Failed to set scan parameters data.");
        return 0;
    }

    // Set BLE events report mask.

    le_set_event_mask_cp event_mask_cp;
    memset(&event_mask_cp, 0, sizeof(le_set_event_mask_cp));
    int i = 0;
    for ( i = 0 ; i < 8 ; i++ ) event_mask_cp.mask[i] = 0xFF;

    struct hci_request set_mask_rq = ble_hci_request(OCF_LE_SET_EVENT_MASK, LE_SET_EVENT_MASK_CP_SIZE, &status, &event_mask_cp);
    ret = hci_send_req(device, &set_mask_rq, 0);
    if ( ret < 0 ) {
        hci_close_dev(device);
        perror("Failed to set event mask.");
        return 0;
    }

    // Enable scanning.

    le_set_scan_enable_cp scan_cp;
    memset(&scan_cp, 0, sizeof(scan_cp));
    scan_cp.enable      = 0x01; // Enable flag.
    scan_cp.filter_dup  = 0x00; // Filtering disabled.

    struct hci_request enable_adv_rq = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &scan_cp);

    ret = hci_send_req(device, &enable_adv_rq, 0);
    if ( ret < 0 ) {
        hci_close_dev(device);
        perror("Failed to enable scan.");
        return 0;
    }

    // Get Results.

    struct hci_filter nf;
    hci_filter_clear(&nf);
    hci_filter_set_ptype(HCI_EVENT_PKT, &nf);
    hci_filter_set_event(EVT_LE_META_EVENT, &nf);
    if ( setsockopt(device, SOL_HCI, HCI_FILTER, &nf, sizeof(nf)) < 0 ) {
        hci_close_dev(device);
        perror("Could not set socket options\n");
        return 0;
    }

    printf("Scanning....\n");

    uint8_t buf[HCI_MAX_EVENT_SIZE];
    evt_le_meta_event * meta_event;
    le_advertising_info * info;
    int len;
    char data_to_send[DATA_SIZE];
    char hostname[2] = "A\0";

    struct timeval tv; // For timestamp
    while ( 1 ) {
        gettimeofday(&tv,NULL);

        len = read(device, buf, sizeof(buf));
        if ( len >= HCI_EVENT_HDR_SIZE ) {
            meta_event = (evt_le_meta_event*)(buf+HCI_EVENT_HDR_SIZE+1);
            if ( meta_event->subevent == EVT_LE_ADVERTISING_REPORT ) {
                uint8_t reports_count = meta_event->data[0];
                void * offset = meta_event->data + 1;
                while ( reports_count-- ) {
                    memset( data_to_send, 0, DATA_SIZE ); // Zero out data_to send

                    info = (le_advertising_info *)offset;
                    char addr[18];

                    char sec[12];
                    sprintf( sec, "%ld", tv.tv_sec );

                    char us[10];
                    sprintf( us, "%04ld", tv.tv_usec );

                    char power[4];
                    sprintf( power, "%d", (char)info->data[info->length] );

                    ba2str(&(info->bdaddr), addr);

                    sprintf( data_to_send, "%1.1s/%10.10s.%4.4s/%17.17s/%3.3s", hostname, sec, us, addr, power );
                    printf("%s\n", data_to_send)
                    offset = info->data + info->length + 2;
                    
                    send(sock, data_to_send , DATA_SIZE, 0 );
                }
            }
        }
    }

    // Disable scanning.

    memset(&scan_cp, 0, sizeof(scan_cp));
    scan_cp.enable = 0x00;  // Disable flag.

    struct hci_request disable_adv_rq = ble_hci_request(OCF_LE_SET_SCAN_ENABLE, LE_SET_SCAN_ENABLE_CP_SIZE, &status, &scan_cp);
    ret = hci_send_req(device, &disable_adv_rq, 0);
    if ( ret < 0 ) {
        hci_close_dev(device);
        perror("Failed to disable scan.");
        return 0;
    }

    hci_close_dev(device);
    
    return 0;
}
