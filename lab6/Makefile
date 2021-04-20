CXX = g++
CXXFLAGS = -Wall -g -O3 -I/usr/csshare/pkgs/glew-1.13.0/include/ -L/usr/csshare/pkgs/glew-1.13.0/lib64/ -Wl,-rpath=/usr/csshare/pkgs/glew-1.13.0/lib64/
LIBS  = -lGL -lGLU -lglut -lGLEW
SRCS = $(wildcard *.cpp)
OBJS := ${SRCS:.cpp=.o}

UNAME_S := $(shell uname -s)
ifeq ($(UNAME_S),Darwin)
	LIBS = -framework OpenGL -framework GLUT -lGLEW
	CXXFLAGS += -Wno-deprecated-declarations
endif

TARGET = textures

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(CPPFLAGS) -o $(TARGET) $(OBJS) $(LIBS)

clean:
	rm -f *.o *.d *~ *.bak $(TARGET)

# Generate a list of dependencies for each cpp file
%.d: $(SRCS)
	@ $(CXX) $(CPPFLAGS) -MM $*.cpp | sed -e 's@^\(.*\)\.o:@\1.d \1.o:@' > $@

# At the end of the makefile
# Include the list of dependancies generated for each object file
# unless make was called with target clean
ifneq "$(MAKECMDGOALS)" "clean"
-include ${SRCS:.cpp=.d}
endif
