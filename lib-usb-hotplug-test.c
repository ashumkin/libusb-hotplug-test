// vim: set shiftwidth=2 tabstop=2 expandtab:
#include <stdio.h>
#include <stdlib.h>
// #include <libusb.h>
#include "libusb-1.0/libusb.h"
#include <unistd.h>
#include <signal.h>

static int interrupted = 0;

int hotplug_callback(struct libusb_context *ctx, struct libusb_device *dev,
                     libusb_hotplug_event event, void *user_data) {
  static libusb_device_handle *handle = NULL;
  struct libusb_device_descriptor desc;
  int rc;
  (void)libusb_get_device_descriptor(dev, &desc);
  printf("Size: %d; class: %d; subclass: %d; VID: %x; PID: %x\n", \
      desc.bLength, desc.bDeviceClass, desc.bDeviceSubClass, \
      desc.idVendor, desc.idProduct);
  if (LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED == event) {
    printf("USB device plugged\n");
    rc = libusb_open(dev, &handle);
    if (LIBUSB_SUCCESS == rc) {
      printf("USB device opened\n");
      char buff[256];
      int err = libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, buff, sizeof(buff));
      if (err > 0) {
        printf("Serial is: %s\n", buff);
      } else {
        printf("Couldn't get serial string (error: %d)\n", err);
      }
    } else {
      printf("Could not open USB device\n");
    }
  } else if (LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT == event) {
    printf("USB device unplugged\n");
    if (handle) {
      libusb_close(handle);
      handle = NULL;
    }
  } else {
    printf("Unhandled event %d\n", event);
  }
  return 0;
}

void signal_handler(int signal)
{
  printf("Received signal %d\n", signal);
  interrupted = 1;
}

int main (void) {
  libusb_hotplug_callback_handle handle;
  int rc;
  signal(SIGINT, signal_handler);
  libusb_init(NULL);
  printf("LibUSB version: %d.%d.%d.%d\n", libusb_get_version()->major, libusb_get_version()->minor, libusb_get_version()->micro, libusb_get_version()->nano);
  rc = libusb_hotplug_register_callback(NULL, LIBUSB_HOTPLUG_EVENT_DEVICE_ARRIVED |
                                        LIBUSB_HOTPLUG_EVENT_DEVICE_LEFT, LIBUSB_HOTPLUG_ENUMERATE, LIBUSB_HOTPLUG_MATCH_ANY, LIBUSB_HOTPLUG_MATCH_ANY,
                                        LIBUSB_HOTPLUG_MATCH_ANY, hotplug_callback, NULL,
                                        &handle);
  if (LIBUSB_SUCCESS != rc) {
    printf("Error creating a hotplug callback\n");
    libusb_exit(NULL);
    return EXIT_FAILURE;
  }
  while (! interrupted) {
    libusb_handle_events_completed(NULL, NULL);
    usleep(1000);
  }
  printf("Closing handles\n");
  libusb_hotplug_deregister_callback(NULL, handle);
  libusb_exit(NULL);
  return 0;
}

