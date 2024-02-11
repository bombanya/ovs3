#ifndef _WEIGHTS_H
#define _WEIGHTS_H

#include "axi_stream.hh"

SC_MODULE(Weights)
{
	enum FSM {
		PROG, WORK, INPUT_LAYER, NEXT_LAYER
	} state = PROG;

	int8_t weights[6000];
	uint64_t weights_cnt = 0;
	uint64_t weights_output_cnt;

	bool last_layer = false;

	uint8_t inputs_n;
	uint8_t neurons_n;

	uint8_t neurons_completed;
	uint8_t inputs_completed;

	sc_in<bool> clk_i;
	AXI_Stream_in input_handler;
	AXI_Stream_in net_params;
	AXI_Stream_out process_units;

	SC_HAS_PROCESS(Weights);

	Weights(sc_module_name nm, sc_clock &clk, AXI_Stream &input_handler, 
	AXI_Stream &net_params, AXI_Stream &process_units);

	void fsm();
	void prep_outputs_for_proc();
};

#endif
