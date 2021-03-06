/*

Copyright 2011-2016 Tyler Gilbert

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

	 http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.

*/



#include <fcntl.h>
#include <unistd.h>
#include <mcu/pio.h>
#include <mcu/boot_debug.h>

#include "link_transport.h"
#include "board_config.h"

static link_transport_phy_t link_transport_open(const char * name, const void * options);

link_transport_driver_t link_transport = {
	.handle = -1,
	.open = link_transport_open,
	.read = boot_link_transport_usb_read,
	.write = boot_link_transport_usb_write,
	.close = boot_link_transport_usb_close,
	.wait = boot_link_transport_usb_wait,
	.flush = boot_link_transport_usb_flush,
	.o_flags = 0, //no checksums -- checksums already happen over USB
	.transport_read = link1_transport_slaveread,
	.transport_write = link1_transport_slavewrite,
	.timeout = 500
};

static usbd_control_t m_usb_control;

link_transport_phy_t link_transport_open(const char * name, const void * options){
	MCU_UNUSED_ARGUMENT(options);
	link_transport_phy_t fd;
	usb_attr_t usb_attr;

	//initialize the USB
	memset(&(usb_attr.pin_assignment), 0xff, sizeof(usb_pin_assignment_t));
	usb_attr.o_flags = USB_FLAG_SET_DEVICE;
	usb_attr.pin_assignment.dp.port = 0;
	usb_attr.pin_assignment.dp.pin = 29;
	usb_attr.pin_assignment.dm.port = 0;
	usb_attr.pin_assignment.dm.pin = 30;
	usb_attr.freq = mcu_board_config.core_osc_freq;
	fd = boot_link_transport_usb_open(name,
												 &m_usb_control,
												 &sos_link_transport_usb_vcp_constants,
												 &usb_attr,
												 mcu_pin(USB_UP_PORT,USB_UP_PIN),
												 USB_UP_PIN_ACTIVE_HIGH); //active high up pin

	return fd;
}
