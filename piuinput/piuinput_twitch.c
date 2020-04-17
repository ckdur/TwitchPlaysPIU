/*
  PIUINPUT for Twitch server
  CKDUR(R). All lefts reserved
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
#include <usb.h>

// Include external
#include "KeyHandlerTwitch.h"
#include "graphics.h"
#include "keyboards.h"
#include "joysticks.h"
#include "autoplay.h"
#include "piuinput_twitch.h"

#define USB_DIR_OUT 0x40
#define USB_DIR_IN 0x80

#define USB_TYPE_STANDARD (0x00 << 5)
#define USB_TYPE_CLASS (0x01 << 5)
#define USB_TYPE_VENDOR (0x02 << 5)
#define USB_TYPE_RESERVED (0x03 << 5)

#define USB_RECIP_DEVICE 0x00
#define USB_RECIP_INTERFACE 0x01
#define USB_RECIP_ENDPOINT 0x02
#define USB_RECIP_OTHER 0x03

const short PIUIO_VENDOR_ID	= 0x0547;
const short PIUIO_PRODUCT_ID = 0x1002;

const short PIUIOBUTTON_VENDOR_ID	= 0x0D2F;
const short PIUIOBUTTON_PRODUCT_ID = 0x1010;

/* proprietary (arbitrary?) request PIUIO requires to handle I/O */
const short PIUIO_CTL_REQ = 0xAE;

/* timeout value for read/writes, in microseconds (so, 10 ms) */
const int REQ_TIMEOUT = 10000;

// CKDur objects

/* USB BUS OBJECT*/
struct usb_bus *bus;

/** Hook declaration */
void (*libusb_usb_init)( void ) = NULL;
int (*libusb_usb_find_busses)( void ) = NULL;
int (*libusb_usb_find_devices)( void ) = NULL;
usb_dev_handle *(*libusb_usb_open)( struct usb_device * ) = NULL;
/** End: Hook declaration */


int g_init = 0;

void usb_init(void)
{
  // And now, init the USB
  printf("CKDUR: usb_init %d\n", __LINE__);
	
  if( !libusb_usb_init )
    libusb_usb_init = dlsym(RTLD_NEXT, "usb_init");
  libusb_usb_init();
	
	// Prevents to be initialized twice
  if(g_init) return;
  g_init = 1;
  
  // Init libraries
  init_keyboards();
  init_joysticks();
  KeyHandler_Twitch_Init();
  init_graphics();
}

struct usb_device* pec;
char bytes_f[4];
char bytes_fb[2];
char bytes_l[4];
int usb_control_msg(usb_dev_handle *dev, int requesttype, int request,
	int value, int index, char *bytes, int size, int timeout)
{
  pec = (struct usb_device*)dev;

  if(pec->descriptor.idVendor == PIUIO_VENDOR_ID && pec->descriptor.idProduct == PIUIO_PRODUCT_ID &&
    (requesttype & USB_DIR_IN) && (requesttype & USB_TYPE_VENDOR) && (request & PIUIO_CTL_REQ))
  {
    poll_keyboards();
    poll_joysticks();
    KeyHandler_Twitch_Poll();
    update_autoplay();
    bytes_f[0] = bytes[0] = bytes_j[0] & bytes_p[0] & bytes_t[0];
    bytes_f[1] = bytes[1] = bytes_j[1] & bytes_p[1] & bytes_t[1];
    bytes_f[2] = bytes[2] = bytes_j[2] & bytes_p[2] & bytes_t[2];
    bytes_f[3] = bytes[3] = bytes_j[3] & bytes_p[3] & bytes_t[3];
    update_graphics();
    return 8;
  }
  else if(pec->descriptor.idVendor == PIUIO_VENDOR_ID && pec->descriptor.idProduct == PIUIO_PRODUCT_ID &&
         (requesttype & USB_DIR_OUT) && (requesttype & USB_TYPE_VENDOR) && (request & PIUIO_CTL_REQ))
  {
    KeyHandler_Twitch_UpdateLights(bytes);
    bytes_l[0] = bytes[0];
    bytes_l[1] = bytes[1];
    bytes_l[2] = bytes[2];
    bytes_l[3] = bytes[3];
    return 8;
  }
  else if(pec->descriptor.idVendor == PIUIOBUTTON_VENDOR_ID && pec->descriptor.idProduct == PIUIOBUTTON_PRODUCT_ID)
  {
    bytes_fb[0] = bytes[0] = bytes_jb[0] & bytes_pb[0] & bytes_tb[0];
    bytes_fb[1] = bytes[1] = bytes_jb[1] & bytes_pb[1] & bytes_tb[1];
    return 0;
  }
  else {
    printf("Unknown request: %x, %x, %x:%x\n", (int)requesttype, (int)request, pec->descriptor.idVendor, pec->descriptor.idProduct);
  }
  return 8;
}

int usb_claim_interface(usb_dev_handle *dev, int interface)
{
    // Nothing
    return 0;
}

typedef unsigned int uint;

int g_found_busses = 0;
int usb_find_busses(void)
{
  // Create the usb bus object
  printf("CKDUR: usb_find_busses\n");
  if(g_found_busses) return 1;
  if(!g_found_busses) bus = (struct usb_bus*)malloc(sizeof(struct usb_bus));
  memset(bus, 0, sizeof(struct usb_bus));
  bus->next=NULL;
  bus->prev=NULL;

  usb_busses = bus;
  g_found_busses = 1;

  return 1;
}

int g_usb_device1=0;
int g_usb_device2=0;
int g_found_devs=0;
struct usb_device* g_dev = NULL;
struct usb_device* g_dev2 = NULL; // For button
int usb_find_devices(void)
{
  printf("CKDUR: usb_find_devices\n");
  if(g_found_devs) return 1;
  if(!g_usb_device1)
  {
    g_dev = (struct usb_device*)malloc(sizeof(struct usb_device));
    memset(g_dev, 0, sizeof(struct usb_device));
    g_dev->bus = bus;    // Same bus... LOL
    // A new configfor PIUIO
    g_dev->config = (struct usb_config_descriptor*)malloc(sizeof(struct usb_config_descriptor));
    memset(g_dev->config, 0, sizeof(struct usb_config_descriptor));
    g_dev->config->interface = (struct usb_interface*)malloc(sizeof(struct usb_interface));
    memset(g_dev->config->interface, 0, sizeof(struct usb_interface));
    g_dev->config->interface->altsetting = (struct usb_interface_descriptor*)malloc(sizeof(struct usb_interface_descriptor));
    memset(g_dev->config->interface->altsetting, 0, sizeof(struct usb_interface_descriptor));
    g_dev->config->interface->altsetting->endpoint = (struct usb_endpoint_descriptor*)malloc(sizeof(struct usb_endpoint_descriptor));
    memset(g_dev->config->interface->altsetting->endpoint, 0, sizeof(struct usb_endpoint_descriptor));
    // Set the bitches for PIUIO
    g_dev->descriptor.idVendor = PIUIO_VENDOR_ID;
    g_dev->descriptor.idProduct = PIUIO_PRODUCT_ID;
    g_usb_device1=1;
  }
  if(!g_usb_device2)
  {
    g_dev2 = (struct usb_device*)malloc(sizeof(struct usb_device));
    memset(g_dev2, 0, sizeof(struct usb_device));
    g_dev2->bus = bus;    // Same bus... LOL
    // A new config for PIUIOBUTTON
    g_dev2->config = (struct usb_config_descriptor*)malloc(sizeof(struct usb_config_descriptor));
    memset(g_dev2->config, 0, sizeof(struct usb_config_descriptor));
    g_dev2->config->interface = (struct usb_interface*)malloc(sizeof(struct usb_interface));
    memset(g_dev2->config->interface, 0, sizeof(struct usb_interface));
    g_dev2->config->interface->altsetting = (struct usb_interface_descriptor*)malloc(sizeof(struct usb_interface_descriptor));
    memset(g_dev2->config->interface->altsetting, 0, sizeof(struct usb_interface_descriptor));
    g_dev2->config->interface->altsetting->endpoint = (struct usb_endpoint_descriptor*)malloc(sizeof(struct usb_endpoint_descriptor));
    memset(g_dev2->config->interface->altsetting->endpoint, 0, sizeof(struct usb_endpoint_descriptor));
    // Set the bitches for PIUIOBUTTON
    g_dev2->descriptor.idVendor = PIUIOBUTTON_VENDOR_ID;
    g_dev2->descriptor.idProduct = PIUIOBUTTON_PRODUCT_ID;
    g_usb_device2=1;
  }
  g_dev->next = g_dev2;
  g_dev2->prev = g_dev;

  // Finally, ret
  bus->devices = g_dev;
  bus->root_dev = g_dev;
  printf("The address are: %x %x\n", (uint)bus->devices, (uint)g_dev2);
  g_found_devs = 1;
	return 1;
}

usb_dev_handle *usb_open(struct usb_device *dev)
{
  void* p = (void*)dev;
  usb_dev_handle* pd = (usb_dev_handle*)p;
  return pd;
}

int usb_reset(usb_dev_handle *dev)
{
  return 0;
}

int usb_set_altinterface(usb_dev_handle *dev, int alternate)
{
  return 0;
}

int usb_set_configuration(usb_dev_handle *dev, int configuration)
{
  return 0;
}

struct usb_bus *usb_get_busses(void)
{
	return usb_busses;
}


