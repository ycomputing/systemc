SYSTEMC		:= /opt/systemc
CXX			:= g++
CFLAGS		:= -c -g -Wall -I/opt/systemc/include
CXXFLAGS	:= -std=c++17
LDFLAGS		:= -L/opt/systemc/lib-linux64
LIBS		:= -lsystemc -lm
EXE			:= project.exe

SRCS	= $(wildcard *.cpp)
OBJS	= $(SRCS:.cpp=.o)

$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) 2>&1 | c++filt
	@test -x $@

.cpp.o:
	$(CXX) $(CFLAGS) $(CXXFLAGS) -c $< -o $@

clean:
	rm $(OBJS) $(EXE)
