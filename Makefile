# COMPILER

CXX= g++

# OUTPUT DIRECTORIES

BINDIR = bin
OBJDIR = obj

# FILES

OBJS = \
	$(OBJDIR)/Main.o \
	$(OBJDIR)/coloring_algorithm.o \
	$(OBJDIR)/dsatur.o \
	$(OBJDIR)/graph.o \
	$(OBJDIR)/global_functions.o \
	$(OBJDIR)/SanSegundoBound.o \
	$(OBJDIR)/HFBBound.o

# CPLEX VERSION (LIBS and INCLUDE files)

CPLEXPATH = $(CPLEX_PATH)

CPLEXDIR = $(CPLEXPATH)/cplex
CONCERTDIR = $(CPLEXPATH)/concert

# header directories
CPLEXINCDIR = $(CPLEXDIR)/include
CONCERTINCDIR = $(CONCERTDIR)/include

# library directories
CPLEXLIBDIR = $(CPLEXDIR)/lib/x86-64_linux/static_pic
CONCERTLIBDIR = $(CONCERTDIR)/lib/x86-64_linux/static_pic


# Nothing should be changed

LP_LIBS =  -L$(CPLEXLIBDIR) -L$(CONCERTLIBDIR) -lilocplex -lcplex -lconcert -lm -lpthread  -lm  -lpthread -ldl

#DBG= -O3
DBG= -g

INCDIR = -I. -I$(CPLEXINCDIR) -I$(CPLEXINCDIR)/ilcplex -I$(CONCERTINCDIR) -I$(CONCERTINCDIR)/ilconcert -I$(INCPATHS) -Isrc

#COMPILER FLAGS

CXXFLAGS =  $(DBG) $(INCDIR) 

LDLIBS = $(LP_LIBS)

# Colori per un output più gradevole
BLUE   := \033[1;34m
GREEN  := \033[1;32m
RESET  := \033[0m

.PHONY: all clean

all: $(BINDIR)/EWMCP_BOUNDS

$(BINDIR)/EWMCP_BOUNDS: $(OBJS)
	@mkdir -p $(BINDIR)
	@echo  "$(BLUE)Linking:   $(RESET) $@"
	@$(CXX) $(CXXFLAGS) -o $@ $(OBJS) $(LDLIBS) $(LDOPT)
	@echo  "$(GREEN)Build complete!$(RESET)"

$(OBJDIR)/%.o: src/%.cpp
	@mkdir -p $(OBJDIR)
	@echo  "$(BLUE)Compiling: $(RESET) $<"
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJS): Makefile

clean:
	@echo "Cleaning up..."
	@rm -rf $(OBJDIR) $(BINDIR)
