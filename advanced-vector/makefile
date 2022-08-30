CC  = g++
CFLAGS  = -c -O2 -std=c++17 -Wall -Wextra -pedantic
LDFLAGS = 
SOURCES = $(sort $(patsubst %.cpp,%.o,$(wildcard *.cpp)))
OBJECTS = $(SOURCES:.cpp=.o)
PRJNAME = a_vect

ifeq ($(OS),Windows_NT)
CMD_DELETE	=	del /F
EXESUFFIX	=	.exe
else
CMD_DELETE	=	rm -f
EXESUFFIX	=
endif

STRIP		=	strip

all: $(SOURCES) $(PRJNAME)$(EXESUFFIX)

# make executable file
$(PRJNAME)$(EXESUFFIX): $(OBJECTS)
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@
	$(STRIP) $@

# make one object file for each *.cpp file
.cpp.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	$(CMD_DELETE) $(OBJECTS)
	$(CMD_DELETE) $(PRJNAME)$(EXESUFFIX)
