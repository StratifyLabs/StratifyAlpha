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

#include <sys/lock.h>
#include <fcntl.h>
#include <errno.h>
#include <mcu/mcu.h>
#include <mcu/debug.h>
#include <mcu/periph.h>
#include <mcu/microchip/sst25vf.h>
#include <mcu/sys.h>
#include <mcu/uartfifo.h>
#include <mcu/usbfifo.h>
#include <mcu/fifo.h>
#include <mcu/sys.h>
#include <sos/link.h>
#include <sos/fs/sysfs.h>
#include <sos/fs/appfs.h>
#include <sos/fs/devfs.h>
#include <sos/fs/sffs.h>
#include <sos/sos.h>

#include "link_transport.h"

#define STFY_SYSTEM_CLOCK 120000000
#define STFY_SYSTEM_MEMORY_SIZE (8192*2)

static void board_event_handler(int event, void * args);

const mcu_board_config_t mcu_board_config = {
		.core_osc_freq = 12000000,
		.core_cpu_freq = STFY_SYSTEM_CLOCK,
		.core_periph_freq = STFY_SYSTEM_CLOCK,
		.usb_max_packet_zero = MCU_CORE_USB_MAX_PACKET_ZERO_VALUE,
		.debug_uart_pin_assignment[0] = {0, 2},
		.debug_uart_pin_assignment[1] = {0, 3},
		.usb_pin_assignment[0] = {0, 29},
		.usb_pin_assignment[1] = {0, 30},
		.usb_pin_assignment[2] = {0xff, 0xff},
		.usb_pin_assignment[3] = {0xff, 0xff},
		.o_flags = 0,
		.event_handler = board_event_handler,
		.led.port = 1, .led.pin = 18
};

#define SCHED_TASK_TOTAL 10

void board_event_handler(int event, void * args){
	switch(event){
	case MCU_BOARD_CONFIG_EVENT_PRIV_FATAL:
		//start the bootloader on a fatal event
		mcu_core_invokebootloader(0, 0);
		break;
	case MCU_BOARD_CONFIG_EVENT_START_LINK:
		mcu_debug("Start LED\n");
		sos_led_startup();
		break;
	case MCU_BOARD_CONFIG_EVENT_START_FILESYSTEM:
		mcu_debug("Started %ld apps\n", *((u32*)args));
		break;
	}
}


const sos_board_config_t sos_board_config = {
		.clk_usecond_tmr = 3,
		.task_total = SCHED_TASK_TOTAL,
		.clk_usec_mult = (uint32_t)(STFY_SYSTEM_CLOCK / 1000000),
		.clk_nsec_div = (uint32_t)((uint64_t)1024 * 1000000000 / STFY_SYSTEM_CLOCK),
		.stdin_dev = "/dev/stdio-in" ,
		.stdout_dev = "/dev/stdio-out",
		.stderr_dev = "/dev/stdio-out",
		.o_sys_flags = SYS_FLAGS_STDIO_FIFO,
		.sys_name = "Stratify Alpha",
		.sys_version = "1.3",
		.sys_id = "-KZKdVwMXIj6vTVsbX56",
		.sys_memory_size = STFY_SYSTEM_MEMORY_SIZE,
		.start = sos_default_thread,
		.start_args = &link_transport,
		.start_stack_size = STRATIFY_DEFAULT_START_STACK_SIZE,
		.socket_api = 0
		//.socket_api = &lwip_socket_api  //use this to include LWIP -- also remove SYMBOLS_IGNORE_SOCKET from symbols.c
};

volatile sched_task_t sos_sched_table[SCHED_TASK_TOTAL] MCU_SYS_MEM;
task_t sos_task_table[SCHED_TASK_TOTAL] MCU_SYS_MEM;

#define USER_ROOT 0
#define GROUP_ROOT 0

/* This is the state information for the sst25vf flash IC driver.
 *
 */
sst25vf_state_t sst25vf_state MCU_SYS_MEM;

/* This is the configuration specific structure for the sst25vf
 * flash IC driver.
 */
const sst25vf_cfg_t sst25vf_cfg = SST25VF_DEVICE_CFG(0, 6, -1, 0, -1, 0, 0, 8, 2*1024*1024, 10000000);

#define UART0_DEVFIFO_BUFFER_SIZE 128
char uart0_fifo_buffer[UART0_DEVFIFO_BUFFER_SIZE];
const uartfifo_cfg_t uart0_fifo_cfg = UARTFIFO_DEVICE_CFG(0,
		uart0_fifo_buffer,
		UART0_DEVFIFO_BUFFER_SIZE);
uartfifo_state_t uart0_fifo_state MCU_SYS_MEM;

#define UART1_DEVFIFO_BUFFER_SIZE 64
char uart1_fifo_buffer[UART1_DEVFIFO_BUFFER_SIZE];
const uartfifo_cfg_t uart1_fifo_cfg = UARTFIFO_DEVICE_CFG(1,
		uart1_fifo_buffer,
		UART1_DEVFIFO_BUFFER_SIZE);
uartfifo_state_t uart1_fifo_state MCU_SYS_MEM;

#define UART3_DEVFIFO_BUFFER_SIZE 64
char uart3_fifo_buffer[UART3_DEVFIFO_BUFFER_SIZE];
const uartfifo_cfg_t uart3_fifo_cfg = UARTFIFO_DEVICE_CFG(3,
		uart3_fifo_buffer,
		UART3_DEVFIFO_BUFFER_SIZE);
uartfifo_state_t uart3_fifo_state MCU_SYS_MEM;


#define STDIO_BUFFER_SIZE 128

char stdio_out_buffer[STDIO_BUFFER_SIZE];
char stdio_in_buffer[STDIO_BUFFER_SIZE];

fifo_cfg_t stdio_out_cfg = FIFO_DEVICE_CFG(stdio_out_buffer, STDIO_BUFFER_SIZE);
fifo_cfg_t stdio_in_cfg = FIFO_DEVICE_CFG(stdio_in_buffer, STDIO_BUFFER_SIZE);
fifo_state_t stdio_out_state = { .head = 0, .tail = 0, .rop = NULL, .rop_len = 0, .wop = NULL, .wop_len = 0 };
fifo_state_t stdio_in_state = {
		.head = 0, .tail = 0, .rop = NULL, .rop_len = 0, .wop = NULL, .wop_len = 0
};

#define MEM_DEV 0

/* This is the list of devices that will show up in the /dev folder
 * automatically.  By default, the peripheral devices for the MCU are available
 * plus some devices on the board.
 */
const devfs_device_t devices[] = {
		//mcu peripherals
		DEVFS_HANDLE("mem0", mcu_mem, 0, 0, 0, 0666, USER_ROOT, S_IFBLK),
		DEVFS_HANDLE("core", mcu_core, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("core0", mcu_core, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("adc0", mcu_adc, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("dac0", mcu_dac, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("eint0", mcu_eint, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("eint1", mcu_eint, 1, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("eint2", mcu_eint, 2, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("eint3", mcu_eint, 3, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pio0", mcu_pio, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pio1", mcu_pio, 1, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pio2", mcu_pio, 2, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pio3", mcu_pio, 3, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pio4", mcu_pio, 4, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("i2c0", mcu_i2c, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("i2c1", mcu_i2c, 1, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("i2c2", mcu_i2c, 2, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("pwm1", mcu_pwm, 1, 0, 0, 0666, USER_ROOT, S_IFBLK),
		DEVFS_HANDLE("qei0", mcu_qei, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("rtc", mcu_rtc, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("spi0", mcu_ssp, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("spi1", mcu_ssp, 1, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("spi2", mcu_ssp, 2, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("tmr0", mcu_tmr, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("tmr1", mcu_tmr, 1, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("tmr2", mcu_tmr, 2, 0, 0, 0666, USER_ROOT, S_IFCHR),
		//DEVFS_HANDLE("uart0", mcu_uart, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("uart0", uartfifo, 0, &uart0_fifo_cfg, &uart0_fifo_state, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("uart1", mcu_uart, 1, 0, 0, 0666, USER_ROOT, S_IFCHR),
		//DEVFS_HANDLE("uart1", uartfifo, 1, &uart1_fifo_cfg, &uart1_fifo_state, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("uart2", mcu_uart, 2, 0, 0, 0666, USER_ROOT, S_IFCHR),
		//DEVFS_HANDLE("uart3", uartfifo, 3, &uart3_fifo_cfg, &uart3_fifo_state, 0666, USER_ROOT, S_IFCHR),
		DEVFS_HANDLE("usb0", mcu_usb, 0, 0, 0, 0666, USER_ROOT, S_IFCHR),

		//user devices
		//SST25VF_SSP_DEVICE("disk0", 1, 0, 0, 6, 10000000, &sst25vf_cfg, &sst25vf_state, 0666, USER_ROOT, GROUP_ROOT),

		//FIFO buffers used for std in and std out
		DEVFS_HANDLE("stdio-out", fifo, 0, &stdio_out_cfg, &stdio_out_state, 0666, USER_ROOT, GROUP_ROOT),
		DEVFS_HANDLE("stdio-in", fifo, 0, &stdio_in_cfg, &stdio_in_state, 0666, USER_ROOT, GROUP_ROOT),

		//system devices
		DEVFS_HANDLE("link-phy-usb", usbfifo, 0, &sos_link_transport_usb_fifo_cfg, &sos_link_transport_usb_fifo_state, 0666, USER_ROOT, GROUP_ROOT),

		DEVFS_HANDLE("sys", sys, 0, 0, 0, 0666, USER_ROOT, GROUP_ROOT),


		DEVFS_TERMINATOR
};


//this is the data needed for the stratify flash file system (wear-aware file system)
extern const sffs_cfg_t sffs_cfg;
sffs_state_t sffs_state;
open_file_t sffs_open_file; // Cannot be in MCU_SYS_MEM because it is accessed in unpriv mode

const sffs_cfg_t sffs_cfg = {
		.open_file = &sffs_open_file,
		.devfs = &(sysfs_list[1]),
		.name = "disk0",
		.state = &sffs_state
};



#if 0
//This is the setup code if you want to use a FAT filesystem (like on an SD card)

fatfs_state_t fatfs_state;
open_file_t fatfs_open_file; // Cannot be in MCU_SYS_MEM because it is accessed in unpriv mode

const fatfs_cfg_t fatfs_cfg = {
		.open_file = &fatfs_open_file,
		.devfs = &(sysfs_list[1]),
		.name = "disk1",
		.state = &fatfs_state,
		.vol_id = 0
};
#endif

const sysfs_t const sysfs_list[] = {
		APPFS_MOUNT("/app", &(devices[MEM_DEV]), SYSFS_ALL_ACCESS), //the folder for ram/flash applications
		DEVFS_MOUNT("/dev", devices, SYSFS_READONLY_ACCESS), //the list of devices
		//SFFS_MOUNT("/home", &sffs_cfg, SYSFS_ALL_ACCESS), //the stratify file system on external RAM
		//FATFS("/home", &fatfs_cfg, SYSFS_ALL_ACCESS), //fat filesystem with external SD card
		SYSFS_MOUNT("/", sysfs_list, SYSFS_READONLY_ACCESS), //the root filesystem (must be last)
		SYSFS_TERMINATOR
};


