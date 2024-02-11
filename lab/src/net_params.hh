#ifndef _NET_PARAMS_H
#define _NET_PARAMS_H

#include "axi_stream.hh"

SC_MODULE(Net_params)
{
	enum FSM {
		PROG, WORK
	} state = PROG;

	uint8_t neurons[64];
	uint8_t layers_n = 0;


	sc_in<bool> clk_i;
	AXI_Stream_in input_handler;

	AXI_Stream_out layer_inputs;
	uint8_t layer_inputs_cnt = 0;

	AXI_Stream_out process_units;
	uint8_t process_units_cnt = 0;

	AXI_Stream_out weights;
	uint8_t weights_cnt = 0;

	sc_out<bool> process_units_reset;

	SC_HAS_PROCESS(Net_params);

	Net_params(sc_module_name nm, sc_clock &clk, AXI_Stream &input_handler, AXI_Stream &layer_inputs, 
		AXI_Stream &process_units, AXI_Stream &weights, sc_signal<bool> &process_units_reset);

	void proc_in();
};

#endif
