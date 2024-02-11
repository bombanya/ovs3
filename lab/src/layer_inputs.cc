#include "layer_inputs.hh"

Layer_inputs::Layer_inputs(sc_module_name nm, sc_clock &clk, AXI_Stream &input_handler, 
		AXI_Stream &process_units_in, AXI_Stream &net_params, AXI_Stream &process_units_out) :
		sc_module(nm), clk_i(clk), input_handler(input_handler), process_units_in(process_units_in),
		net_params(net_params), process_units_out(process_units_out)
{
	this->input_handler.tready.initialize(1);

	this->net_params.tready.initialize(0);

	this->process_units_out.tvalid.initialize(0);
	this->process_units_out.tlast.initialize(0);
	this->process_units_out.tuser.initialize(0);

	this->process_units_in.tready.initialize(1);

	SC_METHOD(fsm)
	sensitive << clk_i.pos();
}

void Layer_inputs::fsm() {
	input_handler.tready.write(0);
	net_params.tready.write(0);
	
	switch (state)
	{
	case PROG:
		input_handler.tready.write(1);
		if (input_handler.tvalid.read()) {
			inputs[inputs_n] = input_handler.tdata[0].read();
			valid_inputs[inputs_n] = true;
			inputs_n++;
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
		} else {
			net_params.tready.write(1);
		}
		break;
	case WORK:
		if (process_units_out.tready.read()) {
			if (inputs_completed == inputs_n) {
				process_units_out.tvalid.write(0);
				if (last_layer) {
					inputs_n = 0;
					state = PROG;
				}
				else {
					state = NEXT_LAYER;
					net_params.tready.write(1);
				}
			} else {
				prep_outputs_for_proc();
			}
		}
		break;
	default:
		break;
	}

	if (process_units_in.tvalid.read()) {
		for (uint8_t i = 0; i < process_units_in.tdata_width; i++) {
			if (process_units_in.tkeep[i].read()) {
				inputs[process_in_cnt] = process_units_in.tdata[i].read();
				valid_inputs[process_in_cnt] = true;
				process_in_cnt++;
				// std::cout << (int)process_in_cnt << std::endl;
			}
		}
		if (process_units_in.tlast.read()) {
			process_in_cnt = 0;
		}
	}
}

void Layer_inputs::prep_outputs_for_proc() {
	if (neurons_completed == 0) {
		if (!valid_inputs[inputs_completed]) {
			process_units_out.tvalid.write(0);
		} else {
			valid_inputs[inputs_completed] = false;
			input_buf = inputs[inputs_completed];
			process_units_out.tvalid.write(1);
		}
	} else process_units_out.tvalid.write(1);

	if (inputs_completed + 1 == inputs_n) {
		if (last_layer) process_units_out.tlast.write(1);
		else process_units_out.tlast.write(0);
		process_units_out.tuser.write(1);
	} else process_units_out.tuser.write(0);

	for (uint8_t i = 0; i < process_units_out.tdata_width; i++) {
		if (neurons_completed < neurons_n) {
			process_units_out.tdata[i].write(input_buf);
			process_units_out.tkeep[i].write(1);
			neurons_completed++;
		} else {
			process_units_out.tkeep[i].write(0);
		}
	}
	if (neurons_completed == neurons_n) {
		inputs_completed++;
		neurons_completed = 0;
	}
}
