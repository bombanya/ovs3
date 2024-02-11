#include "axi_stream.hh"

AXI_Stream::AXI_Stream(uint8_t tdata_width) : tdata_width(tdata_width){
	tdata = new sc_signal<int8_t>[tdata_width];
	tkeep = new sc_signal<bool>[tdata_width];
}

AXI_Stream::~AXI_Stream() {
	delete [] tdata;
	delete [] tkeep;
}

AXI_Stream_in::AXI_Stream_in(AXI_Stream& axi_stream) : 
	tvalid(axi_stream.tvalid), tready(axi_stream.tready), 
	tlast(axi_stream.tlast), tuser(axi_stream.tuser), 
	tdata_width(axi_stream.tdata_width)
 {
	tdata = new sc_in<int8_t>[tdata_width];
	tkeep = new sc_in<bool>[tdata_width];
	for (uint8_t i = 0; i < tdata_width; ++i) {
		tdata[i](axi_stream.tdata[i]);
		tkeep[i](axi_stream.tkeep[i]);
	}
}

AXI_Stream_in::~AXI_Stream_in() {
	delete [] tdata;
	delete [] tkeep;
}

AXI_Stream_out::AXI_Stream_out(AXI_Stream& axi_stream) : 
	tvalid(axi_stream.tvalid), tready(axi_stream.tready), 
	tlast(axi_stream.tlast), tuser(axi_stream.tuser), 
	tdata_width(axi_stream.tdata_width)
 {
	tdata = new sc_out<int8_t>[tdata_width];
	tkeep = new sc_out<bool>[tdata_width];
	for (uint8_t i = 0; i < tdata_width; ++i) {
		tdata[i](axi_stream.tdata[i]);
		tkeep[i](axi_stream.tkeep[i]);
	}
}

AXI_Stream_out::~AXI_Stream_out() {
	delete [] tdata;
	delete [] tkeep;
}
