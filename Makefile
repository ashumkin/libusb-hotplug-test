CC := gcc
LDLIBS += -lusb-1.0

all: lib-usb-hotplug-test

%lib-usb-hotplug-test: %.c
