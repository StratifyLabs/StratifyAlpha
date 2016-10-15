/*
 * link_transport.c
 *
 *  Created on: May 23, 2016
 *      Author: tgil
 */

#include <fcntl.h>
#include <unistd.h>
#include <iface/dev/pio.h>

#include "link_transport.h"

static link_transport_phy_t link_transport_open(const char * name, int baudrate);

link_transport_driver_t link_transport = {
		.handle = -1,
		.open = link_transport_open,
		.read = stratify_link_transport_usb_read,
		.write = stratify_link_transport_usb_write,
		.close = stratify_link_transport_usb_close,
		.wait = stratify_link_transport_usb_wait,
		.flush = stratify_link_transport_usb_flush,
		.notify = stratify_link_transport_usb_notify,
		.timeout = 500
};

#define CONNECT_PORT "/dev/pio1"
#define CONNECT_PINMASK (1<<19)

link_transport_phy_t link_transport_open(const char * name, int baudrate){
	pio_attr_t attr;
	link_transport_phy_t fd;
	int fd_pio;

	//Deassert the Connect line and enable the output
	fd_pio = open(CONNECT_PORT, O_RDWR);
	if( fd_pio < 0 ){
		return -1;
	}
	attr.mask = (CONNECT_PINMASK);
	ioctl(fd_pio, I_PIO_CLRMASK, (void*)attr.mask);
	attr.mode = PIO_MODE_OUTPUT | PIO_MODE_DIRONLY;
	ioctl(fd_pio, I_PIO_SETATTR, &attr);

	fd = stratify_link_transport_usb_open(name, baudrate);

	ioctl(fd_pio, I_PIO_SETMASK, (void*)attr.mask);
	close(fd_pio);

	return fd;
}
