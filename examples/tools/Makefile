# Flags to use regardless of compiler
CFLAGS_all := -Wall -Wno-unused-function -std=c++11 -I../../ -I./
#CFLAGS_all := -Wall -Wno-unused-variable -Wno-unused-function -Wno-extra-semi -std=c++11 -I../../ -I./

# Emscripten compiler information
CXX_web := emcc
CXX_native := g++

#OFLAGS_native := -g -pedantic
OFLAGS_native := -O3 -DNDEBUG

OFLAGS_web := -g4 -pedantic -Wno-dollar-in-identifier-extension -s TOTAL_MEMORY=67108864 -s ASSERTIONS=2 -s DEMANGLE_SUPPORT=1 # -s SAFE_HEAP=1
#OFLAGS_web := -Os -DNDEBUG -s TOTAL_MEMORY=67108864


CFLAGS_native := $(CFLAGS_all) $(OFLAGS_native)

CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s NO_EXIT_RUNTIME=1
#CFLAGS_web := $(CFLAGS_all) $(OFLAGS_web) --js-library ../../emtools/library_emp.js -s EXPORTED_FUNCTIONS="['_main', '_empCppCallback']" -s DISABLE_EXCEPTION_CATCHING=1 -s NO_EXIT_RUNTIME=1


TARGETS := array DFA lexer_utils macros meta NFA Random reflection RegEx signal vector


TOOL_DEPEND := ../../tools/assert.h

default: native

native: $(TARGETS)


array: array.cc ../../tools/array.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) array.cc -o array

DFA: DFA.cc ../../tools/DFA.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) DFA.cc -o DFA

lexer_utils: lexer_utils.cc ../../tools/lexer_utils.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) lexer_utils.cc -o lexer_utils

macros: macros.cc ../../tools/macros.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) macros.cc -o macros

meta: meta.cc ../../tools/meta.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) meta.cc -o meta

NFA: NFA.cc ../../tools/NFA.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) NFA.cc -o NFA

Random: Random.cc ../../tools/Random.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) Random.cc -o Random

reflection: reflection.cc ../../tools/reflection.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) reflection.cc -o reflection

RegEx: RegEx.cc ../../tools/RegEx.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) RegEx.cc -o RegEx

signal: signal.cc ../../tools/signal.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) signal.cc -o signal

vector: vector.cc ../../tools/vector.h $(TOOL_DEPEND)
	$(CXX_native) $(CFLAGS_native) vector.cc -o vector


debug-%: $*.cc
	$(CXX_native) $(CFLAGS_native) $< -o $@

clean:
	rm -rf debug-* *~ *.dSYM $(TARGETS)

# Debugging information
#print-%: ; @echo $*=$($*)
print-%: ; @echo '$(subst ','\'',$*=$($*))'