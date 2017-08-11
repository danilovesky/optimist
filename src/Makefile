CC = g++
CFLAGS = -m32 -g
LIBS = 
PREFIX=/usr/local/optimist
BIN_PATH=/usr/local/bin

LIB_OBJS = stg.o list.o param.o stg_procs.o element.o debug.o strutils.o

# OMD = OptiMist: Detect redundant places
OMD = om_detect
OMD_OBJS = $(LIB_OBJS) omd_main.o omd_params.o omd_procs.o omd_msgs.o

# OME = OptiMist: Expose outputs
OME = om_expose
OME_OBJS = $(LIB_OBJS) ome_main.o ome_params.o ome_procs.o ome_msgs.o

# OMT = OptiMist: Transform the net by removing redundant places
OMT = om_transform
OMT_OBJS = $(LIB_OBJS) omt_main.o omt_params.o omt_procs.o omt_msgs.o 

# OMG = OptiMist: generate a Graphical representation of the net
OMG = om_graph
OMG_OBJS = $(LIB_OBJS) omg_main.o omg_params.o omg_procs.o omg_msgs.o

# OMV = OptiMist: map the net into Verilog netlist
OMV = om_verilog
OMV_OBJS = $(LIB_OBJS) omv_main.o omv_params.o omv_procs.o omv_msgs.o 

# OML = OptiMist: create a Library of elements
OML = om_library
OML_OBJS = $(LIB_OBJS) oml_main.o oml_params.o oml_procs.o oml_msgs.o 

# OPTIMIST = script which connects all OptiMist tools in one command-line interface 
OPTIMIST = optimist

# readme.txt 
README = index.htm

# all binaries
BINS = $(OMD) $(OME) $(OMT) $(OMG) $(OMV) $(OML)

# al documentation
DOCS = $(README)

.PHONY = all clean

all: $(BINS)
	

install: all
	cp $(BINS) $(OPTIMIST) $(DOCS) $(PREFIX)
	ln -s $(PREFIX)/$(OMD) $(BIN_PATH)/$(OMD)
	ln -s $(PREFIX)/$(OME) $(BIN_PATH)/$(OME)
	ln -s $(PREFIX)/$(OMT) $(BIN_PATH)/$(OMT)
	ln -s $(PREFIX)/$(OMG) $(BIN_PATH)/$(OMG)
	ln -s $(PREFIX)/$(OMV) $(BIN_PATH)/$(OMV)
	ln -s $(PREFIX)/$(OML) $(BIN_PATH)/$(OML)
	ln -s $(PREFIX)/$(OPTIMIST) $(BIN_PATH)/$(OPTIMIST)

uninstall:
	rm $(BIN_PATH)/$(OMD)
	rm $(BIN_PATH)/$(OME)
	rm $(BIN_PATH)/$(OMT)
	rm $(BIN_PATH)/$(OMG)
	rm $(BIN_PATH)/$(OMV)
	rm $(BIN_PATH)/$(OML)
	rm $(BIN_PATH)/$(OPTIMIST)
	rm -rf $(PREFIX)

$(OMD): $(OMD_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

$(OME): $(OME_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

$(OMT): $(OMT_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

$(OMG): $(OMG_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

$(OMV): $(OMV_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

$(OML): $(OML_OBJS)
	$(CC) $(CFLAGS) $(LIBS) $^ -o $@

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

clean:
	rm -f $(BINS)
	rm -f *.o
	rm -f *.~*
	rm -f core
