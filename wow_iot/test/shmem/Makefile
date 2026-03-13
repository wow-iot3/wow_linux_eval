.SUFFIXES: .c .o

COMPILE = $(CC) -g -O1 -Wall -D_FILE_OFFSET_BITS=64 -g -DDEBUG_FLAG -fstack-protector
INC_PATH = -I/home/wow/open/wow-iot7/wow_iot/inc/ -I/home/wow/open/wow-iot7/wow_base/inc/ 
LIB_PATH = -L/home/wow/open/wow-iot7/build/local/UBUNTU/lib   -lwowiot -lwowhal -lwowbase  -lev  -L  -lrt -lc -ldl -lm  -pthread 

TARGET_PRGS = shmcache_set shmcache_get shmcache_delete shmcache_remove_all \
			  shmcache_stats shmcache_dump

ALL_PRGS = $(TARGET_PRGS)

all: $(ALL_PRGS)
.c:
	$(COMPILE) -o $@ $< $(LIB_PATH) $(INC_PATH)
.c.o:
	$(COMPILE) -c -o $@ $<  $(INC_PATH)

clean:
	rm -f $(ALL_PRGS)

