PREFIX=$(CURDIR)

ifeq (, $(shell which x86_64-w64-mingw32-g++))
GCC=g++
INCLUDE=$(PREFIX)/glfw/include
LIBRARY=$(PREFIX)/glfw/src
else
GCC=x86_64-w64-mingw32-g++
INCLUDE=$(PREFIX)/include
LIBRARY=$(PREFIX)/lib-mingw-w64
GCC_EXTRA_ARGS=-static -static-libgcc -static-libstdc++
endif

build:
	@echo Using compiler: $(GCC) with args: $(GCC_EXTRA_ARGS)
	@echo Compiling to: $(PREFIX)/bin
	@echo Compiling to: $(CUR__INSTALL)/bin
	@mkdir -p $(PREFIX)/bin
	@$(GCC) -o $(PREFIX)/bin/glfw_wrapper.o -I$(OCAMLLIB) -I$(INCLUDE) -c ./glfw_wrapper.cc
	ocamlmklib -o glfw_wrapper_stubs $(PREFIX)/bin/glfw_wrapper.o
	ocamlc -i glfw.ml > glfw.mli
	ocamlc -c glfw.mli
	ocamlc -c glfw.ml
	ocamlc -a -custom -o glfw.cma glfw.cmo -dllib dllglfw_wrapper_stubs.dll
	ocamlopt -c glfw.ml
	ocamlopt $(PREFIX)/bin/glfw_wrapper.o glfw.cmx -o test_app -ccopt -L$(LIBRARY) -cclib -lglfw3 -ccopt "-framework OpenGL" -ccopt "-framework Cocoa" -ccopt "-framework IOKit" -ccopt "-framework CoreVideo"
	#ocamlopt -a -o glfw.cmxa glfw.cmx -ccopt -cclib -lwrap_stubs -lglfw3 -lgdi32 -lopengl32

test:
	@node ./tests/test.js && node ./tests/xtest.js