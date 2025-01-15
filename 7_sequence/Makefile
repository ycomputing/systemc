SYSTEMC		:= /opt/systemc
CXX			:= g++
CFLAGS		:= -c -Wall -I/opt/systemc/include
CXXFLAGS	:= -std=c++17
LDFLAGS		:= -L/opt/systemc/lib-linux64
LIBS		:= -lsystemc -lm
EXE			:= project.exe

SRCS	= $(wildcard *.cpp)
OBJS	= $(SRCS:.cpp=.o)
DEPEND	= $(OBJS:%.o=%.d)

$(EXE): $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS) 2>&1 | c++filt
	@test -x $@

-include $(DEPEND)

.cpp.o:
	$(CXX) $(CFLAGS) $(CXXFLAGS) -MMD -c $< -o $@

view:	$(EXE)
	./$(EXE)
	gtkwave trace.vcd

clean:
	rm -f $(OBJS) $(EXE) $(DEPEND)
