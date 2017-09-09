CC := gcc
LDLIBS += -lusb-1.0
LDLIBS += -lpthread

all: lib-usb-hotplug-test

%lib-usb-hotplug-test: %.c
