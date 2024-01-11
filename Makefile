LIB = -L build/raylib/raylib \
			-L build/bullet/lib \
			-l raylib \
			-l gdi32 \
			-l winmm \
			-l BulletDynamics \
			-l BulletCollision \
			-l LinearMath \

HEADERS = -I src

INCLUDE = -I build/raylib/raylib/include \
					-I libs/bullet3/src \
					-I libs/json/single_include/nlohmann \

FLAGS = -Wall \
				-std=c++17 \
				-O0 \
				-g

OBJ = build/out/main.o \
			build/out/Camera.o \
			build/out/FlyCamera.o \
			build/out/Model.o \
			build/out/PhysicsWorld.o \
			build/out/PlayerMovement.o \
			build/out/LevelEditor.o \

GPP = g++

all: $(OBJ)
	$(GPP) -o build/app.exe $(OBJ) $(LIB) $(FLAGS) $(HEADERS)

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

clean:
	rm -f build/out/*.o
