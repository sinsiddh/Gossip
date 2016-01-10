#**************************************************
# Single Author Info
# skumar23 Sudhendu Kumar
#
# Group Info
# mghegde Mahendra Hegde
# ssingh24 Siddharth Singh
# skumar23 Sudhendu Kumar
#**************************************************/

EXECUTABLE    := p4
CC            := gcc
INCLUDES      += -I. -I/ncsu/gcc346/include/c++/ -I/ncsu/gcc346/include/c++/3.4.6/backward 
LIBSTATIC     := -L/ncsu/gcc346/lib -L.
ARCHIVER_ARG  := rcs
DEBUG_FLAGS   := -v -g
COMPILE_FLAGS := -c
OBJECT_FLAGS  := -pthread -o
SOURCE        := p4.c neigh.c 

all :
	$(CC) $(TEST_FILE) $(SOURCE) $(OBJECT_FLAGS) $(EXECUTABLE)

clean:
	rm -f $(EXECUTABLE) lib$(STATICLIB).a *.o temp* list* endpoints

