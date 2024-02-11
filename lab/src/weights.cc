#include "weights.hh"

Weights::Weights(sc_module_name nm, sc_clock &clk, AXI_Stream &input_handler, 
		AXI_Stream &net_params, AXI_Stream &process_units) :
		sc_module(nm), clk_i(clk), input_handler(input_handler),
		net_params(net_params), process_units(process_units)
{
	this->input_handler.tready.initialize(0);

	this->net_params.tready.initialize(0);

	this->process_units.tvalid.initialize(0);
	this->process_units.tlast.initialize(0);
	this->process_units.tuser.initialize(0);


	SC_METHOD(fsm)
	sensitive << clk_i.pos();
}

void Weights::fsm() {
	input_handler.tready.write(0);
	net_params.tready.write(0);
	process_units.tvalid.write(0);

	if (input_handler.tvalid.read() && state != PROG) {
		state = PROG;
		input_handler.tready.write(1);
		weights_cnt = 0;
		return;
	}
	
	switch (state)
	{
	case PROG:
		input_handler.tready.write(1);
		if (input_handler.tvalid.read()) {
			weights[weights_cnt] = input_handler.tdata[0].read();
			weights_cnt++;
			if (input_handler.tlast.read()) {
				state = INPUT_LAYER;
				net_params.tready.write(1);
			}
		}
		break;
	case INPUT_LAYER:
		net_params.tready.write(1);
		if (net_params.tvalid.read()) {
			inputs_n = net_params.tdata[0].read();
			neurons_n = inputs_n;
			state = NEXT_LAYER;
			weights_output_cnt = 0;
		}
		break;
	case NEXT_LAYER:
		if (net_params.tvalid.read()) {
			last_layer = net_params.tlast.read();
			inputs_n = neurons_n;
			neurons_n = net_params.tdata[0].read();
			state = WORK;
			neurons_completed = 0;
			inputs_completed = 0;
			prep_outputs_for_proc();
			process_units.tvalid.write(1);
		} else {
			net_params.tready.write(1);
		}
		break;
	case WORK:
		if (process_units.tready.read()) {
			if (inputs_completed == inputs_n) {
				net_params.tready.write(1);
				if (last_layer) {
					state = INPUT_LAYER;
				}
				else {
					state = NEXT_LAYER;
				}
			} else {
				process_units.tvalid.write(1);
				prep_outputs_for_proc();
			}
		} else process_units.tvalid.write(1);
		break;
	default:
		break;
	}
}

void Weights::prep_outputs_for_proc() {
	if (inputs_completed + 1 == inputs_n) {
		if (last_layer) {
			process_units.tlast.write(1);
		}
		else process_units.tlast.write(0);
		process_units.tuser.write(1);
	} else process_units.tuser.write(0);

	for (uint8_t i = 0; i < process_units.tdata_width; i++) {
		if (neurons_completed < neurons_n) {
			process_units.tdata[i].write(weights[weights_output_cnt]);
			process_units.tkeep[i].write(1);
			neurons_completed++;
			weights_output_cnt++;
		} else {
			process_units.tkeep[i].write(0);
		}
	}
	if (neurons_completed == neurons_n) {
		inputs_completed++;
		neurons_completed = 0;
		// std::cout << "input completed: " << (int)inputs_completed << "; weights_output_cnt: " << weights_output_cnt << std::endl;
	}
}
