#include "input_handler.hh"
#include "net_params.hh"
#include "weights.hh"
#include "layer_inputs.hh"
#include "process_units.hh"

#include <iostream>
#include <fstream>
#include <png++/png.hpp>

SC_MODULE(Init) 
{
	sc_in<bool> clk;
	sc_clock &clk_ref;
	AXI_Stream_out net_input;
	AXI_Stream_in net_output;

	SC_HAS_PROCESS(Init);
	
	Init(sc_module_name nm, sc_clock &clk, AXI_Stream &net_input, AXI_Stream &net_output);

	void mainThread();

	void read_pic_int(std::string path, int8_t* inputs);
};

Init::Init(sc_module_name nm, sc_clock &clk, AXI_Stream &net_input, AXI_Stream &net_output) :
	sc_module(nm), clk(clk), clk_ref(clk), net_input(net_input), net_output(net_output)
{
	this->net_input.tvalid.initialize(0);
	this->net_input.tuser.initialize(0);
	this->net_input.tlast.initialize(0);

	this->net_output.tready.initialize(1);

	SC_CTHREAD(mainThread, this->clk.pos());
}

void Init::read_pic_int(std::string path, int8_t* inputs) {
    png::image<png::gray_pixel> pic(path);
    for (uint32_t y = 0; y < 7; y++) {
        for (uint32_t x = 0; x < 7; x++) {
            inputs[y * 7 + x] = pic.get_pixel(x, y) > 0 ? 64 : 0;
        }
    }
}

void Init::mainThread() {

	int8_t layers[][5] = {{49, 20, 50, 50, 3}, {49, 10, 3}};
	uint8_t layers_n[] = {5, 3};
	std::string weight_files[] = {"weights_20_50_50_3.txt", "weights_10_3.txt"};

	int8_t weights[6000];
	uint64_t weights_n = 0;

	std::ifstream ifile;
	std::string weights_path;
	std::string file_input;

	for (uint8_t config = 0; config < 2; config++) {
		std::cout << "config " << (int)config << ": ";
		for (uint8_t i = 0; i < layers_n[config]; i++) std::cout << (int)layers[config][i] << " ";
		std::cout << "\n\n";

		weights_path = "/home/merlin/git/ovs1/" + weight_files[config];
		ifile.open(weights_path);
		weights_n = 0;

		while (std::getline(ifile, file_input)) {
			weights[weights_n] = std::stoi(file_input);
			weights_n++;
		}
		ifile.close();

		wait();

		uint8_t layers_passed = 0;
		while (layers_passed != layers_n[config]) {
			net_input.tvalid.write(1);
			net_input.tuser.write(1);
			net_input.tdata[0].write(layers[config][layers_passed]);
			net_input.tlast.write(layers_passed + 1 == layers_n[config]);
			wait();
			// std::cout << (int)layers_passed << std::endl;
			if (net_input.tready.read()) {
				layers_passed++;
			}
		}

		uint64_t weights_passed = 0;
		while (weights_passed != weights_n) {
			net_input.tvalid.write(1);
			net_input.tuser.write(1);
			net_input.tdata[0].write(weights[weights_passed]);
			net_input.tlast.write(weights_passed + 1 == weights_n);
			wait();
			if (net_input.tready.read()) {
				weights_passed++;
			}
		}

		std::string dirs[] = {"circle", "square", "triangle"};
		std::string path;
		int8_t pic[49];

		for (uint8_t i = 0; i < 3; i++) {
			for (uint8_t j = 6; j < 9; j++) {
				path = "/home/merlin/git/ovs1/" + dirs[i] + "/" + std::to_string(j) + ".png";
				read_pic_int(path, pic);

				uint64_t pic_passed = 0;
				while (pic_passed != 49) {
					net_input.tvalid.write(1);
					net_input.tuser.write(0);
					net_input.tdata[0].write(pic[pic_passed]);
					net_input.tlast.write(pic_passed + 1 == 49);
					wait();
					if (net_input.tready.read()) {
						pic_passed++;
					}
				}

				net_input.tvalid.write(0);
				double pic_loaded = clk_ref.time_stamp().to_double();

				while (!net_output.tlast.read()) {
					wait();
					if (net_output.tvalid.read()) {
						double cir = net_output.tdata[0].read() / 32.0;
						double squ = net_output.tdata[1].read() / 32.0;
						double tri = net_output.tdata[2].read() / 32.0;

						double inf_compl = clk_ref.time_stamp().to_double();
						printf("Ref: %s; Inf time: %f mcs; Neuro: cir - %f squ - %f tri - %f", 
						dirs[i].c_str(), (inf_compl - pic_loaded) / 1000000.0, cir, squ, tri);
						std::cout << std::endl;
					}
				}
			}
			std::cout << std::endl;
		}
		std::cout << "---\n\n";
	}

	

	sc_stop();
}

int sc_main(int argc, char* argv[])
{
	sc_clock clk("clk", sc_time(10, SC_NS));

	AXI_Stream handler_in(1);
	AXI_Stream input_handler_to_layer_inputs(1);
	AXI_Stream input_handler_to_net_params(1);
	AXI_Stream input_handler_to_weights(1);

	Input_handler input_handler("input_handler", clk, handler_in, 
			input_handler_to_net_params, input_handler_to_weights, input_handler_to_layer_inputs);

	AXI_Stream net_params_to_layer_inputs(1);
	AXI_Stream net_params_to_process_units(1);
	AXI_Stream net_params_to_weights(1);
	sc_signal<bool> process_units_reset;

	Net_params net_params("net_params", clk, input_handler_to_net_params, net_params_to_layer_inputs, 
			net_params_to_process_units, net_params_to_weights, process_units_reset);

	AXI_Stream weights_to_process_units(5);

	Weights weights("weights", clk, input_handler_to_weights, net_params_to_weights, weights_to_process_units);

	AXI_Stream layer_inputs_to_process_units(5);
	AXI_Stream process_units_to_layer_inputs(5);

	Layer_inputs layer_inputs("layer_inputs", clk, input_handler_to_layer_inputs, process_units_to_layer_inputs, 
		net_params_to_layer_inputs, layer_inputs_to_process_units);

	AXI_Stream results(5);

	Process_units process_units("process_units", clk, layer_inputs_to_process_units, net_params_to_process_units, 
			weights_to_process_units, process_units_to_layer_inputs, results, process_units_reset);

	Init initobj("init", clk, handler_in, results);

	sc_trace_file *wf = sc_create_vcd_trace_file("wave");
	sc_trace(wf, clk, "clk");
	sc_trace(wf, handler_in.tvalid, "tvalid");
	sc_trace(wf, handler_in.tready, "tready");
	sc_trace(wf, handler_in.tdata[0], "tdata");
	sc_trace(wf, handler_in.tlast, "tlast");

	std::cout << clk.time_stamp().to_string() << std::endl;

	sc_start();

	std::cout << clk.time_stamp().to_string() << std::endl;

	sc_close_vcd_trace_file(wf);

	return 0;
}

