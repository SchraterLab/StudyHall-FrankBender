BUILD_DIR=build
CXX = g++
CXXFLAGS = -std=c++11 -g -fPIC

INCLUDES = -Iprizm/cpp/include
LIBS = -L.
EXE = ${BUILD_DIR}/bin/prizm
SHARED = ${BUILD_DIR}/lib/prizm.so

SOURCES = $(wildcard prizm/cpp/src/*.cc) $(wildcard prizm/cpp/src/*.cpp)

OBJFILES_CC = $(notdir $(SOURCES:.cc=.o))
OBJFILES_ALL = $(notdir $(OBJFILES_CC:.tpp=.o))
OBJFILES = $(notdir $(OBJFILES_ALL:.cpp=.o))
OBJDIR = $(BUILD_DIR)/obj
OBJECTS = $(addprefix $(OBJDIR)/, $(OBJFILES))

test: $(EXE)

lib: $(SHARED)

$(EXE): $(BUILD_DIR) $(SHARED)
	$(CXX) $(CXXFLAGS) -Wl,-rpath,$(BUILD_DIR)/lib $(INCLUDES) prizm/cpp/src/printcolor.cc prizm/cpp/src/main.cc $(LIBS) -o $@

$(SHARED): $(BUILD_DIR) $(OBJECTS)
	$(CXX) $(CXXFLAGS) -shared $(INCLUDES) $(LIBS) $(OBJECTS) -o $@

$(OBJDIR)/%.o: prizm/cpp/src/%.cc
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: prizm/cpp/src/%.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p ${BUILD_DIR}/obj
	mkdir -p ${BUILD_DIR}/lib
	mkdir -p ${BUILD_DIR}/bin

clean:
	rm -rf ${BUILD_DIR}