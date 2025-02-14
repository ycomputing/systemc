#include <systemc>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>

using namespace sc_core;
using namespace sc_dt;

#include "axi_subordinate.h"

void AXI_SUBORDINATE::on_reset()
{
	event_queue.cancel_all();
	memory.clear();

	AWREADY.write(0);
	WREADY.write(0);
	BVALID.write(0);
	BID.write(0);
	BRESP.write(0);
	ARREADY.write(0);
	RVALID.write(0);
	RID.write(0);
	RDATA.write(BUS_DATA_ZERO);
	RLAST.write(0);

}

void AXI_SUBORDINATE::on_clock()
{
}
