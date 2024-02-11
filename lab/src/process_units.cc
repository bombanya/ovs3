#include "process_units.hh"

Process_units::Process_units(sc_module_name nm, sc_clock &clk, AXI_Stream &layer_inputs_in, 
	AXI_Stream &net_params, AXI_Stream &weights, AXI_Stream &layer_inputs_out, AXI_Stream &results, sc_signal<bool> &reset) :
		sc_module(nm), clk_i(clk), layer_inputs_in(layer_inputs_in), net_params(net_params),
		weights(weights), layer_inputs_out(layer_inputs_out), results(results), reset(reset)
{
	this->layer_inputs_in.tready.initialize(0);

	this->net_params.tready.initialize(1);

	this->weights.tready.initialize(0);

	this->layer_inputs_out.tvalid.initialize(0);
	this->layer_inputs_out.tlast.initialize(0);

	this->results.tvalid.initialize(0);
	this->results.tlast.initialize(0);

	SC_METHOD(fsm)
	sensitive << clk_i.pos();
}

void Process_units::fsm() {
	net_params.tready.write(0);

	layer_inputs_out.tvalid.write(0);
	layer_inputs_out.tlast.write(0);

	results.tvalid.write(0);
	results.tlast.write(0);

	if (reset.read()) {
		net_params.tready.write(1);
		state = INPUT_LAYER;
		dbg_flag = true;
		return;
	}

	switch (state)
	{
	case INPUT_LAYER:
		net_params.tready.write(1);
		if (net_params.tvalid.read()) {
			state = NEXT_LAYER;
		}
		break;
	case NEXT_LAYER:
		if (net_params.tvalid.read()) {
			neurons_n = net_params.tdata[0].read();
			state = WORK;
		} else {
			net_params.tready.write(1);
		}
		break;
	case WORK:
		// if (dbg_flag) std::cout << layer_inputs_in.tvalid.read() << " " <<  weights.tvalid.read() << std::endl;
		if (layer_inputs_in.tvalid.read() && weights.tvalid.read() && layer_inputs_in.tready.read()) {
			// layer_inputs_in.tready.write(0);
			// weights.tready.write(0);
			if (layer_inputs_in.tuser.read()) {
				if (layer_inputs_in.tlast.read()) {
					results.tvalid.write(1);
				} else {
					layer_inputs_out.tvalid.write(1);
				}
			}
			for (uint8_t i = 0; i < layer_inputs_in.tdata_width; i++) {
				if (layer_inputs_in.tkeep[i].read()) {
					sums[neurons_completed] += layer_inputs_in.tdata[i].read() * weights.tdata[i].read();
					if (layer_inputs_in.tuser.read()) {
						if (layer_inputs_in.tlast.read()) {
							results.tkeep[i].write(1);
							results.tdata[i].write((int)(1.0 / (1.0 + exp(-(double)sums[neurons_completed] / 1024.0)) * 32.0));
						} else {
							// if (dbg_flag) std::cout << "writeback\n";
							layer_inputs_out.tkeep[i].write(1);
							layer_inputs_out.tdata[i].write((int)(1.0 / (1.0 + exp(-(double)sums[neurons_completed] / 1024.0)) * 32.0));
						}
						sums[neurons_completed] = 0;
					}
					neurons_completed++;
					// if (dbg_flag) std::cout << (int)layer_inputs_in.tdata[i].read() << " " << (int)weights.tdata[i].read() << " " << (int)neurons_completed << std::endl;
				}
			}
			if (neurons_completed == neurons_n) {
				// if (dbg_flag) sc_stop();
				// dbg_flag = true;
				neurons_completed = 0;
				if (layer_inputs_in.tuser.read()) {
					layer_inputs_in.tready.write(0);
					weights.tready.write(0);

					net_params.tready.write(1);
					if (layer_inputs_in.tlast.read()) {
						results.tlast.write(1);
						net_params.tready.write(1);
						state = INPUT_LAYER;
					} else {
						layer_inputs_out.tlast.write(1);
						net_params.tready.write(1);
						state = NEXT_LAYER;
					}
				}
				// std::cout << state << std::endl;
			}
		} else if (layer_inputs_in.tvalid.read() && weights.tvalid.read()) {
			layer_inputs_in.tready.write(1);
			weights.tready.write(1);
		}
		break;
	default:
		break;
	}
}
