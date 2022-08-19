#include makevars

GA_INC_DIR= galib
GA_LIB_DIR= galib/ga

GA_UTILS = utils
UTFLAGS = -L$(GA_UTILS)

#CXXFLAGS = -std=c++11 -I$(GA_INC_DIR) -g -I. -lm
CXXFLAGS = -std=c++11 -I$(GA_INC_DIR) -O3 -I. -lm
LNFLAGS = -L$(GA_LIB_DIR)
LDFLAGS = -lga -lboost_iostreams -lboost_system -lboost_filesystem

EXEC = scheduler
SOURCES = model.cpp schedule.cpp main.cpp scheduler.cpp metascheduler.cpp context.cpp Event.cpp MsGraph.cpp MsgNode.cpp Calendar.cpp Graph.cpp YenTopKShortestPathsAlg.cpp DijkstraShortestPathAlg.cpp
OBJECTS = $(SOURCES: .cpp=.o)

all: lib $(EXEC)

$(EXEC): $(OBJECTS) 
	$(CXX) $(CXXFLAGS) $(OBJECTS) -o $(EXEC) $(LNFLAGS) $(LDFLAGS) $(UTFLAGS)

lib:
	cd galib; $(MAKE) lib

clean:
	rm -f scheduler
	rm -f *.o