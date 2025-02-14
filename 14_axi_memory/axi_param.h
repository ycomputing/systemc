#ifndef __AXI_PARAM_H__
#define __AXI_PARAM_H__

// For detailed explanation, see AMBA AXI protocol spec
// https://developer.arm.com/documentation/ihi0022/latest/

#define ID_R_WIDTH	32
#define ADDR_WIDTH	64
#define DATA_WIDTH	128

#define ACCESS_READ 'R'
#define ACCESS_WRITE 'W'

#define DATA_WIDTH_BYTE	16 (DATA_WIDTH/8)

typedef unsigned char bus_access_t;
typedef sc_biguint<128> bus_data_t;

#define BUS_DATA_ZERO	0

#endif