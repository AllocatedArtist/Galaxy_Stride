LIB = -L build/raylib/raylib \
			-l raylib \
			-l gdi32 \
			-l winmm \

INCLUDE = -I build/raylib/raylib/include

FLAGS = -Wall \
				-std=c++17 \
				-g

OBJ = build/out/main.o \
			build/out/Camera.o \

GPP = g++

all: $(OBJ)
	$(GPP) -o build/app.exe $(OBJ) $(LIB) $(FLAGS)

build/out/%.o: %.cc
	echo "$< -> $@"
	$(GPP) -c $< $(INCLUDE) -o $@

build/out/%.o: src/%.cc
	echo "$< -> $@"
	$(GPP) -c $< $(INCLUDE) -o $@

build_run:
	make
	make run

run:
	./build/app.exe
