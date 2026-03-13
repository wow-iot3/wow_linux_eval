.SUFFIXES: .c .o

COMPILE = $(CC) -g -O1 -Wall -D_FILE_OFFSET_BITS=64 -g -DDEBUG_FLAG -fstack-protector
INC_PATH = -I/home/wow/open/wow-iot7/wow_base/inc/ 
LIB_PATH = -L/home/wow/open/wow-iot7/build/local/UBUNTU/lib -lwowbase  -lev  -L  -lrt -lc -ldl -lm  -pthread 

TARGET_PRGS = ipc_client ipc_server

ALL_PRGS = $(TARGET_PRGS)

all: $(ALL_PRGS)
.c:
	$(COMPILE) -o $@ $< $(LIB_PATH) $(INC_PATH)
.c.o:
	$(COMPILE) -c -o $@ $<  $(INC_PATH)

clean:
	rm -f $(ALL_PRGS)

