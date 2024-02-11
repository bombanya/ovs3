#ifndef _AXI_STREAM_H
#define _AXI_STREAM_H

#include "systemc.h"

struct AXI_Stream
{
	sc_signal<bool> tvalid;
	sc_signal<bool> tready;
	sc_signal<int8_t> *tdata;
	sc_signal<bool> *tkeep;
	sc_signal<bool> tlast;
	sc_signal<bool> tuser;

	uint8_t tdata_width;

	AXI_Stream(uint8_t tdata_width);
	~AXI_Stream();
};

struct AXI_Stream_in
{
	sc_in<bool> tvalid;
	sc_out<bool> tready;
	sc_in<int8_t> *tdata;
	sc_in<bool> *tkeep;
	sc_in<bool> tlast;
	sc_in<bool> tuser;

	uint8_t tdata_width;

	AXI_Stream_in(AXI_Stream& axi_stream);
	~AXI_Stream_in();
};

struct AXI_Stream_out
{
	sc_out<bool> tvalid;
	sc_in<bool> tready;
	sc_out<int8_t> *tdata;
	sc_out<bool> *tkeep;
	sc_out<bool> tlast;
	sc_out<bool> tuser;

	uint8_t tdata_width;

	AXI_Stream_out(AXI_Stream& axi_stream);
	~AXI_Stream_out();
};

#endif
