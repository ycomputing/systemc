#ifndef __AXI_PARAM_H__
#define __AXI_PARAM_H__

// For detailed explanation, see AMBA AXI protocol spec
// https://developer.arm.com/documentation/ihi0022/latest/

#define ID_R_WIDTH	32
#define ADDR_WIDTH	64
#define DATA_WIDTH	128

#define BUS_ACCESS_READ 'R'
#define BUS_ACCESS_WRITE 'W'

#define DATA_WIDTH_BYTE	16 (DATA_WIDTH/8)

typedef unsigned char bus_access_t;
typedef sc_dt::sc_bigint<128> bus_data_t;

#define BUS_DATA_ZERO	0

#define CHANNEL_AW		"AW"
#define CHANNEL_W		"W"
#define CHANNEL_B		"B"
#define CHANNEL_AR		"AR"
#define CHANNEL_R		"R"

#define CHANNEL_ACCEPT		"ACCEPT"
#define CHANNEL_BUSY		"BUSY"
#define CHANNEL_COMPLETE	"COMPLETE"
#define CHANNEL_ERROR		"ERROR"
#define CHANNEL_IDLE		"IDLE"
#define CHANNEL_INITIATE	"INITIATE"
#define CHANNEL_NOT_READY	"NOTREADY"
#define CHANNEL_READ		"READ"
#define CHANNEL_READY		"READY"
#define CHANNEL_RESPONSE	"RESPONSE"
#define CHANNEL_UNKNOWN		"UNKNOWN"
#define CHANNEL_WAIT		"WAIT"
#define CHANNEL_WRITE		"WRITE"

uint64_t address_from_hex_string(const std::string& str);
std::string address_to_hex_string(uint64_t address);
bus_data_t bus_data_from_hex_string(const std::string& str);
std::string bus_data_to_hex_string(bus_data_t data);
#endif