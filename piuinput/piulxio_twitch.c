/*
  PIULXIO for Twitch server
  See LICENSE for details
*/

#define _GNU_SOURCE
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#include <inttypes.h>
#include <unistd.h>
#include <pthread.h>
#include <glob.h>
#include <libusb-1.0/libusb.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <uchar.h>

// Include external
#include "KeyHandlerTwitch.h"
#include "graphics.h"
#include "keyboards.h"
#include "joysticks.h"
#include "autoplay.h"
#include "piuinput_twitch.h"



#define PIULXIO_STR_MANUFACTURER	u"ANDAMIRO"
#define PIULXIO_STR_PRODUCT		u"PIU HID V1.00"
#define PIULXIO_CTRL_300  {0x0409, 0}

const short PIULXIO_VENDOR_ID  = 0x0d2f;
const short PIULXIO_PRODUCT_ID = 0x1020;
const short PIULXIO_ENDPOINT_OUT  = 0x02;
const short PIULXIO_ENDPOINT_IN = 0x01;
const short PIULXIO_ENDPOINT_PACKET_SIZE = 0x10;
const short PIULXIO_DESC_MAX_PACKET_SIZE = 0x08;

#define USB_DIR_OUT 0x40
#define USB_DIR_IN 0x80
const short PIULXIO_GET_DESCRIPTOR = 0x6;
const short PIULXIO_HID_SET_REPORT = 0x9;

int16_t string0[] = PIULXIO_CTRL_300;
char16_t string1[] = PIULXIO_STR_MANUFACTURER;
char16_t string2[] = PIULXIO_STR_PRODUCT;

uint8_t dataIn[16] = {0, };
uint8_t dataOut[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff};

const struct libusb_device_descriptor piulxio_dev_desc = {
    .bLength = 18,
    .bDescriptorType = LIBUSB_DT_DEVICE,
    .idVendor = PIULXIO_VENDOR_ID,
    .idProduct = PIULXIO_PRODUCT_ID,
    .bcdUSB = 0x110,
    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,
    .bMaxPacketSize0 = 0x8,
    .bcdDevice = 0x100,
    .iManufacturer = 1,
    .iProduct = 2,
    .iSerialNumber = 0,
    .bNumConfigurations = 1
};

const struct libusb_endpoint_descriptor piulxio_endp_desc[2] = {
    {
        .bLength = 7,
        .bDescriptorType = LIBUSB_DT_ENDPOINT,
        .bEndpointAddress = 0x80 | PIULXIO_ENDPOINT_IN,
        .bmAttributes = 0x03,
        .wMaxPacketSize = PIULXIO_DESC_MAX_PACKET_SIZE,
        .bInterval = 1,
        .bRefresh = 0,
        .bSynchAddress = 0,
        .extra = NULL,
        .extra_length = 0
    },
    {
        .bLength = 7,
        .bDescriptorType = LIBUSB_DT_ENDPOINT,
        .bEndpointAddress = PIULXIO_ENDPOINT_OUT,
        .bmAttributes = 0x03,
        .wMaxPacketSize = PIULXIO_DESC_MAX_PACKET_SIZE,
        .bInterval = 1,
        .bRefresh = 0,
        .bSynchAddress = 0,
        .extra = NULL,
        .extra_length = 0
    }
};

const struct libusb_interface_descriptor piulxio_int = {
    .bLength = 8,
    .bDescriptorType = LIBUSB_DT_INTERFACE,
    .bInterfaceNumber = 0,
    .bAlternateSetting = 0,
    .bNumEndpoints = 2,
    .bInterfaceClass = 0x03,
    .bInterfaceSubClass = 0x00,
    .bInterfaceProtocol = 0x00,
    .iInterface = 0,
    .endpoint = piulxio_endp_desc
};

const struct libusb_interface piulxio_interface = {
    .altsetting = &piulxio_int,
    .num_altsetting = 1
};

const struct libusb_config_descriptor piulxio_config_desc = {
    .bLength = 9,
    .bDescriptorType = LIBUSB_DT_CONFIG,
    .wTotalLength = 9+9+9+7+7,
    .bNumInterfaces = 1,
    .bConfigurationValue = 1,
    .iConfiguration = 0,
    .bmAttributes = 0xC0,
    .MaxPower = 50,
    .interface = &piulxio_interface,
    .extra = NULL,
    .extra_length = 0
};

static const uint8_t piulxio_hid_report_desc[] = {
	0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined 0xFF00)
	0x09, 0x01,        // Usage (0x01)
	0xA1, 0x01,        // Collection (Application)
	0x09, 0x02,        //   Usage (0x02)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0xFF,        //   Logical Maximum (-1)
	0x75, 0x08,        //   Report Size (8)
	0x95, 0x10,        //   Report Count (16)
	0x81, 0x02,        //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
	0x09, 0x03,        //   Usage (0x03)
	0x15, 0x00,        //   Logical Minimum (0)
	0x25, 0xFF,        //   Logical Maximum (-1)
	0x75, 0x08,        //   Report Size (8)
	0x95, 0x10,        //   Report Count (16)
	0x91, 0x02,        //   Output (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)
	0xC0,              // End Collection
};

static const uint8_t piulxio_hid_desc[] = {
	9,                                      // bLength
	0x21,                                   // bDescriptorType
	0x11, 0x01,                             // bcdHID
	0,                                      // bCountryCode
	1,                                      // bNumDescriptors
	0x22,                                   // bDescriptorType (0x22 = Report)
	sizeof(piulxio_hid_report_desc), 0,	// wDescriptorLength
};

struct libusb_context {
    int a0;
};

struct libusb_device {
    int pid;
    int vid;
};

struct libusb_device_handle {
    int a0;
};

int (*libusb_libusb_init)(libusb_context **ctx) = NULL;

// libusb_init
int LIBUSB_CALL libusb_init(libusb_context **ctx) {
    // And now, init the USB
    printf("CKDUR: libusb_init (1.0) %d\n", __LINE__);

    *ctx = malloc(sizeof(struct libusb_context));

    return 0;
}
// libusb_exit
void LIBUSB_CALL libusb_exit(libusb_context *ctx) {
    printf("CKDUR: libusb_exit (1.0) %d\n", __LINE__);
    if(ctx != NULL)
        free(ctx);
}

ssize_t LIBUSB_CALL libusb_get_device_list(libusb_context *ctx, libusb_device ***list) {
    printf("CKDUR: libusb_get_device_list (1.0) %d\n", __LINE__);
    *list = malloc(sizeof(struct libusb_device*)*2);
    (*list)[0] = malloc(sizeof(struct libusb_device));
    (*list)[0]->pid = PIULXIO_PRODUCT_ID;
    (*list)[0]->vid = PIULXIO_VENDOR_ID;
    (*list)[1] = NULL;
    return 1;
}
// libusb_free_device_list
void LIBUSB_CALL libusb_free_device_list(libusb_device **list, int unref_devices) {
    printf("CKDUR: libusb_free_device_list (1.0) %d\n", __LINE__);
    if(list && *list && unref_devices) {
        free(*list);
    }
    free(list);
}

// libusb_get_device_descriptor
int LIBUSB_CALL libusb_get_device_descriptor(libusb_device *dev, struct libusb_device_descriptor *desc) {
    printf("CKDUR: libusb_get_device_descriptor (1.0) %d\n", __LINE__);
    if(dev->pid == PIULXIO_PRODUCT_ID && dev->vid == PIULXIO_PRODUCT_ID) {
        memcpy(desc, &piulxio_dev_desc, sizeof(struct libusb_device_descriptor));
        return 0;
    }
    return -1;
}

void helper_fill_config_descriptor(struct libusb_config_descriptor *config, uint8_t config_index) {
    memset(config, 0, sizeof(struct libusb_config_descriptor));
    memcpy(config, &piulxio_config_desc, sizeof(struct libusb_config_descriptor));
    config->interface = &piulxio_interface;
}


// libusb_get_config_descriptor
int LIBUSB_CALL libusb_get_config_descriptor(libusb_device *dev, uint8_t config_index, struct libusb_config_descriptor **config) {
    printf("CKDUR: libusb_get_config_descriptor (1.0) %d\n", __LINE__);
    if(dev->pid == PIULXIO_PRODUCT_ID && dev->vid == PIULXIO_PRODUCT_ID) {
        *config = malloc(sizeof(struct libusb_config_descriptor));
        helper_fill_config_descriptor(*config, config_index);
        return 0;
    }
    return -1;
}
// libusb_get_active_config_descriptor
int LIBUSB_CALL libusb_get_active_config_descriptor(libusb_device *dev, struct libusb_config_descriptor **config) {
    printf("CKDUR: libusb_get_active_config_descriptor (1.0) %d\n", __LINE__);
    if(dev->pid == PIULXIO_PRODUCT_ID && dev->vid == PIULXIO_PRODUCT_ID) {
        *config = malloc(sizeof(struct libusb_config_descriptor));
        helper_fill_config_descriptor(*config, 0);
        return 0;
    }
    return -1;
}

// libusb_free_config_descriptor
void LIBUSB_CALL libusb_free_config_descriptor(struct libusb_config_descriptor *config) {
    printf("CKDUR: libusb_free_config_descriptor (1.0) %d\n", __LINE__);
    if(config != NULL) {
        free(config);
    }
}

// libusb_get_device_address
uint8_t LIBUSB_CALL libusb_get_device_address(libusb_device *dev) {
    printf("CKDUR: libusb_get_device_address (1.0) %d\n", __LINE__);
    return 0;
}
// libusb_get_bus_number
uint8_t LIBUSB_CALL libusb_get_bus_number(libusb_device *dev) {
    printf("CKDUR: libusb_get_bus_number (1.0) %d\n", __LINE__);
    return 0;
}

// libusb_open
int LIBUSB_CALL libusb_open(libusb_device *dev, libusb_device_handle **dev_handle) {
    printf("CKDUR: libusb_open (1.0) %d\n", __LINE__);
    if(dev->pid == PIULXIO_PRODUCT_ID && dev->vid == PIULXIO_PRODUCT_ID) {
        *dev_handle = malloc(sizeof(struct libusb_device_handle));
        (*dev_handle)->a0 = 0;
        return 0;
    }
    return -1;

}
// libusb_close
void LIBUSB_CALL libusb_close(libusb_device_handle *dev_handle) {
    printf("CKDUR: libusb_close (1.0) %d\n", __LINE__);
    if(dev_handle) {
        free(dev_handle);
    }
}

#define min(a, b) ((a)<(b)?(a):(b))

// libusb_control_transfer
int LIBUSB_CALL libusb_control_transfer(libusb_device_handle *dev_handle,
	uint8_t request_type, uint8_t bRequest, uint16_t wValue, uint16_t wIndex,
	unsigned char *data, uint16_t wLength, unsigned int timeout) {
    
    if(dev_handle->a0 == 0) {
        printf("reqtype %x %x %x %x %x\n", request_type, bRequest, wValue, wIndex, wLength);
        if(request_type == USB_DIR_IN && bRequest == PIULXIO_GET_DESCRIPTOR) {
            // From do_control_to_usb: can be 0x300, or param_2 | 0xff | 0x300
            if(wValue == 0x300) { // Get the control descriptor
                int ret = min(4, wLength);
                memcpy(data, string0, ret);
                return ret;
            }
            else if (wValue == 0x301) {
                int ret = min(sizeof(string1), wLength);
                memcpy(data, string1, ret);
                return ret;
            }
            else if (wValue == 0x302) {
                int ret = min(sizeof(string2), wLength);
                memcpy(data, string2, ret);
                return ret;
            }
            else if(wValue == 0x2200) {
                // HID report descriptor
                int ret = min(sizeof(piulxio_hid_report_desc), wLength);
                memcpy(data, piulxio_hid_report_desc, ret);
                return ret;
            }
            else if(wValue == 0x2100) {
                // HID desc
                int ret = min(sizeof(piulxio_hid_desc), wLength);
                memcpy(data, piulxio_hid_desc, ret);
                return ret;
            }
            else {
                printf("Unknown request: %x %x %x %x %x\n", request_type, bRequest, wValue, wIndex, wLength);
                return 0;
            }
        }
        else if (wIndex == 0 && request_type == 0xA1 && bRequest == PIULXIO_HID_SET_REPORT) {
            printf("push data %x %x %x %x %x\r\n", request_type, bRequest, wValue, wIndex, wLength);
            int ret = sizeof(dataOut);
            memcpy(data, dataOut, ret);
            return ret;
        }
        else if(wIndex == 0 && request_type == 0x21 && bRequest == PIULXIO_HID_SET_REPORT ) {
            printf("recv data %x %x %x %x %x\r\n", request_type, bRequest, wValue, wIndex, wLength);
            int ret = sizeof(dataIn);
            memcpy(dataIn, data, ret);
            return ret;
        }
    }
    return -1;
}

// libusb_interrupt_transfer
int LIBUSB_CALL libusb_interrupt_transfer(libusb_device_handle *dev_handle,
	unsigned char endpoint, unsigned char *data, int length,
	int *actual_length, unsigned int timeout) {
    

    if(dev_handle->a0 == 0) {
        printf("reqtype ep=%x len=%x\n", (int)endpoint, length);
        if(endpoint == PIULXIO_ENDPOINT_IN) {
            int ret = min(sizeof(dataIn), length);
            memcpy(dataIn, data, ret);
            *actual_length = ret;
            return ret;
        }
        else if(endpoint == PIULXIO_ENDPOINT_OUT) {
            int ret = min(sizeof(dataOut), length);
            memcpy(data, dataOut, ret);
            *actual_length = ret;
            return ret;
        }
        else {
            printf("Unknown endpoint %d\n", (int)endpoint);
            return -1;
        }
    }
    
    return -1;
}

// libusb_alloc_transfer
struct libusb_transfer * LIBUSB_CALL libusb_alloc_transfer(int iso_packets) {
    return NULL;
}

// libusb_submit_transfer
int LIBUSB_CALL libusb_submit_transfer(struct libusb_transfer *transfer) {
    return 0;
}

// libusb_cancel_transfer
int LIBUSB_CALL libusb_cancel_transfer(struct libusb_transfer *transfer) {
    return 0;
}

// libusb_free_transfer
void LIBUSB_CALL libusb_free_transfer(struct libusb_transfer *transfer) {
}

// libusb_handle_events
int LIBUSB_CALL libusb_handle_events(libusb_context *ctx){
    return 0;
}

// libusb_kernel_driver_active
int LIBUSB_CALL libusb_kernel_driver_active(libusb_device_handle *dev_handle,
	int interface_number) {

    return 0;
}
// libusb_detach_kernel_driver
int LIBUSB_CALL libusb_detach_kernel_driver(libusb_device_handle *dev_handle,
	int interface_number) {

    return 0;
}

// libusb_claim_interface
int LIBUSB_CALL libusb_claim_interface(libusb_device_handle *dev_handle,
	int interface_number) {

    return 0;
}
// libusb_release_interface
int LIBUSB_CALL libusb_release_interface(libusb_device_handle *dev_handle,
	int interface_number) {

    return 0;
}

