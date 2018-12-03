

HOST_SRCS = ./host/main.cpp ../common/ocl_util.cpp
HOST_OBJS = $(HOST_SRCS:%.cpp=%.o)

HOST_INCS = ../common

COMP_CONFIG = $(shell aocl compile-config) -I$(HOST_INCS)  -DFPGA_DEVICE
LINK_CONFIG = $(shell aocl link-config)


KERNEL_SRCS = ./device/HelloWorld_Kernel.cl
KERNEL_NAME = helloword
DEV_EXE = $(KERNEL_NAME).aocx


.PHONY: host
host: main.exe

.PHONY: fpga
fpga: $(DEV_EXE)


.PHONY: all
all: host fpga

%.o: %.cpp
	g++  -c $< -o $@ $(COMP_CONFIG)

main.exe: $(HOST_OBJS)
	g++  $(HOST_OBJS) -o $@ $(LINK_CONFIG)

$(DEV_EXE): $(KERNEL_SRCS) # dest: src
	aoc $(KERNEL_SRCS) -march=emulator

.PHONY: clean
clean:
	rm -rf *.xclbin *.rpt  *.json *.dir *.aocx *.aoco *.source *.exe xocc* sdaccel* ./device/*.ll ./host/*.o  *.log *.jou *.mon *.cl

