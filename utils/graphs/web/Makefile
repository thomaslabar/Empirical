CXX_web := emcc
# OFLAGS_web := -g4 -Wall
OFLAGS_web := -oz -DNDEBUG
CFLAGS_web := -std=c++11 $(OFLAGS_web) -s EXPORTED_FUNCTIONS="['_empLoadString']"

default: web

# What are the source files we are using?
SRC	:= vcover.cc

web:	MSS.js

MSS.js: $(SRC)
	$(CXX_web) $(CFLAGS_web) -o MSS.js $(SRC)

clean:
	rm -f MSS.js *~ *.js.map
