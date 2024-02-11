#ifndef _PROCESS_UNITS_H
#define _PROCESS_UNITS_H

#include "axi_stream.hh"

SC_MODULE(Process_units)
{
	enum FSM {
		WORK, INPUT_LAYER, NEXT_LAYER
	} state = INPUT_LAYER;

	int16_t sums[256] = {0};

	uint8_t neurons_n;

	uint8_t neurons_completed = 0;

	sc_in<bool> clk_i;
	AXI_Stream_in layer_inputs_in;
	AXI_Stream_in net_params;
	AXI_Stream_in weights;
	AXI_Stream_out layer_inputs_out;
	AXI_Stream_out results;
	
	sc_in<bool> reset;

	SC_HAS_PROCESS(Process_units);

	Process_units(sc_module_name nm, sc_clock &clk, AXI_Stream &layer_inputs_in, 
	AXI_Stream &net_params, AXI_Stream &weights, AXI_Stream &layer_inputs_out, AXI_Stream &results, sc_signal<bool> &reset);

	void fsm();

	bool dbg_flag = false;
};

#endif
