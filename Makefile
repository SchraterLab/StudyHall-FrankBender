BUILD_DIR=build
CXX = g++
CXXFLAGS = -std=c++11 -g -fPIC

ROOT_DIR := $(shell git rev-parse --show-toplevel)
-include $(ROOT_DIR)/config/settings
INCLUDES = -Ibackend/cpp/include -I$(DEP_DIR)/include -Icurl
LIBDIRS = -L$(DEP_DIR)/lib 
LIBS = -lpthread -lssl -lcrypto -lz
EXE = ${BUILD_DIR}/bin/prizm
SHARED = ${BUILD_DIR}/lib/prizm.so

SOURCES = $(wildcard backend/cpp/src/*.cc) $(wildcard backend/cpp/src/*.cpp) $(wildcard backend/cpp/src/*/*.cc)

OBJFILES_CC = $(notdir $(SOURCES:.cc=.o))
OBJFILES_ALL = $(notdir $(OBJFILES_CC:.tpp=.o))
OBJFILES = $(notdir $(OBJFILES_ALL:.cpp=.o))
OBJDIR = $(BUILD_DIR)/obj
OBJECTS = $(addprefix $(OBJDIR)/, $(OBJFILES))
-include $(DEP_DIR)/env

# make-depend-cxx=$(CXX) -MM -MF $3 -MP -MT $2 $(CXXFLAGS) $(INCLUDES) $1
# -include $(addprefix $(OBJDIR)/,$(OBJFILES:.o=.d))

all: $(EXE)

tpool: thread_pool

lib: $(SHARED)

$(SHARED): $(BUILD_DIR) $(OBJECTS)
	$(CXX) $(CXXFLAGS) -shared $(INCLUDES) $(LIBDIRS) $(LIBS) $(OBJECTS) -o $@

$(EXE): $(BUILD_DIR) $(SHARED)
	$(CXX) $(CXXFLAGS) -Wl,-rpath,$(BUILD_DIR)/lib $(LIBDIRS) -L$(BUILD_DIR)/lib $(INCLUDES) $(SOURCES) $(LIBS) -o $@ $(FLAGS)

thread_pool: $(BUILD_DIR) $(SHARED)
	$(CXX) $(CXXFLAGS) -Wl,-rpath,$(BUILD_DIR)/lib $(LIBDIRS) -L$(BUILD_DIR)/lib $(INCLUDES) $(SOURCES) $(LIBS) -o $@ $(FLAGS)

$(OBJDIR)/%.o: backend/cpp/src/%.cc
# $(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: backend/cpp/src/%.cpp
# $(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(OBJDIR)/%.o: backend/cpp/src/*/%.cc
# $(call make-depend-cxx,$<,$@,$(subst .o,.d,$@))
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c $< -o $@

$(BUILD_DIR):
	mkdir -p ${BUILD_DIR}/obj
	mkdir -p ${BUILD_DIR}/lib
	mkdir -p ${BUILD_DIR}/bin

clean:
	rm -rf ${BUILD_DIR}