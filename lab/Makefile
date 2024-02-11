SYSTEMC_HOME    = /home/merlin/git/ovs3/systemc-2.3.3
TARGET_ARCH     = linux64

SYSTEMC_INC_DIR = $(SYSTEMC_HOME)/include
SYSTEMC_LIB_DIR = $(SYSTEMC_HOME)/lib-$(TARGET_ARCH)

FLAGS           = -g -Wall -pedantic -Wno-long-long \
                 -DSC_INCLUDE_DYNAMIC_PROCESSES -fpermissive \
                 -I$(SYSTEMC_INC_DIR)
LDFLAGS         = -L$(SYSTEMC_LIB_DIR) -lsystemc -lm

SRCS = src/axi_stream.cc src/input_handler.cc src/layer_inputs.cc src/net_params.cc src/process_units.cc src/weights.cc src/main.cc
OBJS = $(SRCS:.cpp=.o)
	
main:
	g++ -o model $(LDFLAGS) $(FLAGS) $(SRCS) -lpng

