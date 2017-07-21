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
#include <sos/sos_link_transport_usb.h>

#include "link_transport.h"

static link_transport_phy_t link_transport_open(const char * name, int baudrate);

link_transport_driver_t link_transport = {
		.handle = -1,
		.open = link_transport_open,
		.read = boot_link_transport_usb_read,
		.write = boot_link_transport_usb_write,
		.close = boot_link_transport_usb_close,
		.wait = boot_link_transport_usb_wait,
		.flush = boot_link_transport_usb_flush,
		.timeout = 500
};

#define CONNECT_PORT 2
#define CONNECT_PINMASK (1<<9)

static usbd_control_t m_usb_control;

link_transport_phy_t link_transport_open(const char * name, int baudrate){
	pio_attr_t attr;
	link_transport_phy_t fd;
	//Deassert the Connect line and enable the output
	mcu_pio_setmask(CONNECT_PORT, (void*)(CONNECT_PINMASK));
	attr.o_pinmask = (CONNECT_PINMASK);
	attr.o_flags = PIO_FLAG_SET_OUTPUT | PIO_FLAG_IS_DIRONLY;
	mcu_pio_setattr(CONNECT_PORT, &attr);

	//initialize the USB
	memset(&m_usb_control, 0, sizeof(m_usb_control));
	m_usb_control.constants = &sos_link_transport_usb_constants;
	fd = boot_link_transport_usb_open(name, &m_usb_control);

	//Asset the connect line
	mcu_pio_clrmask(CONNECT_PORT, (void*)(CONNECT_PINMASK));
	return fd;
}
