#ifndef _INPUT_HANDLER_H
#define _INPUT_HANDLER_H

#include "axi_stream.hh"

SC_MODULE(Input_handler) 
{
	enum FSM {
		NEURONS, WEIGHTS, PIC
	} state = PIC;

	sc_in<bool> clk_i;
	AXI_Stream_in in;
	AXI_Stream_out net_params;
	AXI_Stream_out weights;
	AXI_Stream_out layer_inputs;

	int8_t tdata_buf;
	bool tlast_buf;
	bool tuser_buf;
	bool buf_valid = false;
	bool fresh_buf = false;

	SC_HAS_PROCESS(Input_handler);

	Input_handler(sc_module_name nm, sc_clock &clk, AXI_Stream &in, 
		AXI_Stream &net_params, AXI_Stream &weights, AXI_Stream &layer_inputs);

	void proc_in();
};

#endif
