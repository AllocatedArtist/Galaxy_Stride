LIB = -L build/raylib/raylib \
			-L build/bullet/lib \
			-l raylib \
			-l gdi32 \
			-l winmm \
			-l BulletDynamics \
			-l BulletCollision \
			-l LinearMath \

INCLUDE = -I build/raylib/raylib/include \
					-I libs/bullet3/src \

FLAGS = -Wall \
				-std=c++17 \
				-g

OBJ = build/out/main.o \
			build/out/Camera.o \
			build/out/FlyCamera.o \
			build/out/Model.o \
			build/out/PhysicsWorld.o \

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
