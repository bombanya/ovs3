#include "input_handler.hh"

Input_handler::Input_handler(sc_module_name nm, sc_clock &clk, AXI_Stream &in, 
		AXI_Stream &net_params, AXI_Stream &weights, AXI_Stream &layer_inputs)
	:sc_module(nm), clk_i(clk), in(in), net_params(net_params), weights(weights), 
	layer_inputs(layer_inputs)
{
	this->in.tready.initialize(1);

	this->net_params.tlast.initialize(0);
	this->net_params.tvalid.initialize(0);

	this->weights.tlast.initialize(0);
	this->weights.tvalid.initialize(0);

	this->layer_inputs.tlast.initialize(0);
	this->layer_inputs.tvalid.initialize(0);

	SC_METHOD(proc_in);
	sensitive << clk_i.pos();
}

void Input_handler::proc_in()
{
	net_params.tvalid.write(0);
	net_params.tlast.write(0);

	weights.tvalid.write(0);
	weights.tlast.write(0);

	layer_inputs.tvalid.write(0);
	layer_inputs.tlast.write(0);

	in.tready.write(0);

	if (in.tvalid.read() && !buf_valid) {
		tdata_buf = in.tdata[0].read();
		tlast_buf = in.tlast.read();
		tuser_buf = in.tuser.read();
		buf_valid = true;
		fresh_buf = true;
	}

	if (buf_valid) {
		switch (state)
		{
		case PIC:
			if (tuser_buf) {
				state = NEURONS;
			} else {
				if (fresh_buf) layer_inputs.tvalid.write(1);
				else if (!(layer_inputs.tready.read() && layer_inputs.tvalid.read())) layer_inputs.tvalid.write(1);

				layer_inputs.tdata[0].write(tdata_buf);
				if (tlast_buf) {
					layer_inputs.tlast.write(1);
				}
				if (layer_inputs.tready.read()) {
					buf_valid = false;
				}
			}	
			break;
		case NEURONS:
			if (fresh_buf) net_params.tvalid.write(1);
			else if (!(net_params.tready.read() && net_params.tvalid.read())) net_params.tvalid.write(1);

			net_params.tdata[0].write(tdata_buf);
			if (tlast_buf) {
				net_params.tlast.write(1);
			}
			if (net_params.tready.read()) {
				if (tlast_buf) state = WEIGHTS;
				buf_valid = false;
			}
			break;
		case WEIGHTS:
			if (fresh_buf) weights.tvalid.write(1);
			else if (!(weights.tready.read() && weights.tvalid.read())) weights.tvalid.write(1);

			weights.tdata[0].write(tdata_buf);
			if (tlast_buf) {
				weights.tlast.write(1);
			}
			if (weights.tready.read()) {
				if (tlast_buf) state = PIC;
				buf_valid = false;
			}
			break;
		default:
			break;
		}
	}

	if (!buf_valid) {
		in.tready.write(1);
	}

	fresh_buf = false;
	
}
