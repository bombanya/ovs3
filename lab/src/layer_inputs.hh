#ifndef _LAYER_INPUTS_H
#define _LAYER_INPUTS_H

#include "axi_stream.hh"

SC_MODULE(Layer_inputs)
{
	enum FSM {
		PROG, WORK, INPUT_LAYER, NEXT_LAYER
	} state = PROG;

	int8_t inputs[256];
	bool valid_inputs[256];
	int8_t input_buf;

	bool last_layer = false;

	uint8_t inputs_n = 0;
	uint8_t neurons_n;

	uint8_t neurons_completed;
	uint8_t inputs_completed;

	uint8_t process_in_cnt = 0;

	sc_in<bool> clk_i;
	AXI_Stream_in input_handler;
	AXI_Stream_in process_units_in;
	AXI_Stream_in net_params;
	AXI_Stream_out process_units_out;

	SC_HAS_PROCESS(Layer_inputs);

	Layer_inputs(sc_module_name nm, sc_clock &clk, AXI_Stream &input_handler, 
	AXI_Stream &process_units_in, AXI_Stream &net_params, AXI_Stream &process_units_out);

	void fsm();
	void prep_outputs_for_proc();
	
};

#endif
