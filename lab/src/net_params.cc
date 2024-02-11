#include "net_params.hh"

Net_params::Net_params(sc_module_name nm, sc_clock &clk, AXI_Stream &input_handler, AXI_Stream &layer_inputs, 
		AXI_Stream &process_units, AXI_Stream &weights, sc_signal<bool> &process_units_reset) :
	sc_module(nm), clk_i(clk), input_handler(input_handler), layer_inputs(layer_inputs), 
	process_units(process_units), weights(weights), process_units_reset(process_units_reset)
{
	this->input_handler.tready.initialize(1);

	this->layer_inputs.tvalid.initialize(0);
	this->layer_inputs.tlast.initialize(0);

	this->process_units.tvalid.initialize(0);
	this->process_units.tlast.initialize(0);

	this->weights.tvalid.initialize(0);
	this->weights.tlast.initialize(0);

	this->process_units_reset.initialize(0);

	SC_METHOD(proc_in);
	sensitive << clk_i.pos();
}

void Net_params::proc_in() 
{
	input_handler.tready.write(0);

	layer_inputs.tvalid.write(0);
	layer_inputs.tlast.write(0);

	process_units.tvalid.write(0);
	process_units.tlast.write(0);

	weights.tvalid.write(0);
	weights.tlast.write(0);

	process_units_reset.write(0);

	if (input_handler.tvalid.read() && state != PROG) {
		input_handler.tready.write(1);
		process_units_reset.write(1);
		layers_n = 0;
		layer_inputs_cnt = 0;
		process_units_cnt = 0;
		weights_cnt = 0;
		state = PROG;
		return;
	}

	switch (state)
	{
	case PROG:
		if (input_handler.tvalid.read()) {
			neurons[layers_n] = input_handler.tdata[0].read();
			layers_n++;
			if (input_handler.tlast.read()) {
				state = WORK;
				layer_inputs.tvalid.write(1);
				layer_inputs.tdata[0].write(neurons[0]);

				process_units.tvalid.write(1);
				process_units.tdata[0].write(neurons[0]);

				weights.tvalid.write(1);
				weights.tdata[0].write(neurons[0]);
			} else {
				input_handler.tready.write(1);
			}
		} else input_handler.tready.write(1);
		break;
	case WORK:
		if (layer_inputs.tready.read()) {
			layer_inputs_cnt = (layer_inputs_cnt + 1) % layers_n;
		}
		layer_inputs.tvalid.write(1);
		layer_inputs.tdata[0].write(neurons[layer_inputs_cnt]);
		layer_inputs.tlast.write(layer_inputs_cnt + 1 == layers_n);
		

		if (process_units.tready.read()) {
			process_units_cnt = (process_units_cnt + 1) % layers_n;
		}
		process_units.tvalid.write(1);
		process_units.tdata[0].write(neurons[process_units_cnt]);
		process_units.tlast.write(process_units_cnt + 1 == layers_n);
		
		if (weights.tready.read()) {
			weights_cnt = (weights_cnt + 1) % layers_n;
		}
		weights.tvalid.write(1);
		weights.tdata[0].write(neurons[weights_cnt]);
		weights.tlast.write(weights_cnt + 1 == layers_n);
		break;
	default:
		break;
	}


}
