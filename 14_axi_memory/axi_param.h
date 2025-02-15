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

typedef sc_dt::sc_bigint<128> bus_data_t;

#define BUS_DATA_ZERO	0

// name of channels

#define CHANNEL_NAME_AW			"AW"
#define CHANNEL_NAME_W			"W"
#define CHANNEL_NAME_B			"B"
#define CHANNEL_NAME_AR			"AR"
#define CHANNEL_NAME_R			"R"
#define CHANNEL_NAME_READER		"reader"
#define CHANNEL_NAME_WRITER		"writer"
#define CHANNEL_NAME_MANAGER	"manager"

// name of channel states

#define CHANNEL_ACCEPT		"ACCEPT"
#define CHANNEL_BUSY		"BUSY"
#define CHANNEL_COMPLETE	"COMPLETE"
#define CHANNEL_ENQUEUE		"ENQUEUE"
#define CHANNEL_ERROR		"ERROR"
#define CHANNEL_HOLD		"HOLD"
#define CHANNEL_IDLE		"IDLE"
#define CHANNEL_INITIATE	"INITIATE"
#define CHANNEL_NO_MATCH	"NOMATCH"
#define CHANNEL_NOT_READY	"NOTREADY"
#define CHANNEL_OKAY		"OKAY"
#define CHANNEL_READ		"READ"
#define CHANNEL_READY		"READY"
#define CHANNEL_RESPONSE	"RESPONSE"
#define CHANNEL_UNKNOWN		"UNKNOWN"
#define CHANNEL_WAIT		"WAIT"
#define CHANNEL_WRITE		"WRITE"

// unit of latency is in clock cycles

#define LATENCY_READY_AW	10
#define LATENCY_READY_W		20
#define LATENCY_READY_B		3
#define LATENCY_READY_AR	10
#define LATENCY_READY_R		20


// conversion utility functions

uint64_t address_from_hex_string(const std::string& str);
std::string address_to_hex_string(uint64_t address);
bus_data_t bus_data_from_hex_string(const std::string& str);
std::string bus_data_to_hex_string(bus_data_t data);

#endif