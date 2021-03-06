deppath=../deps/

CC=g++ -std=c++11
CXX=g++ -std=c++11

#SDL2Libs=$(shell sdl2-config --libs)
SDL2Libs=-lSDL2
SDL2Libs+= -lSDL2_image

CLLibs=-lOpenCL -lGLU

CLInclude=-I../deps/openCL/

LibPath=-L/usr/lib/ -L/usr/local/lib/
IncPath=$(CLInclude) -I. -I/usr/local/include/

Libs=$(SDL2Libs) $(CLLibs)

filedir=generator
OBJDIR=build

files=$(addprefix generator/, generator.cpp clengine.cpp shader_util.cpp physicsengine.cpp)
___objs=$(addprefix $(OBJDIR)/, main.o)
utl_objs=$(addprefix $(OBJDIR)/, sdlutil.o)
cle_objs=$(addprefix $(OBJDIR)/, shader_util.o clengine.o physicsengine.o)
obj_objs=$(addprefix $(OBJDIR)/, sprite.o object.o)
wld_objs=$(addprefix $(OBJDIR)/, world.o worldengine.o)
gen_objs=$(addprefix $(OBJDIR)/, generator.o generatorengine.o)
objects=$(___objs) $(utl_objs) $(cle_objs) $(obj_objs) $(wld_objs) 


all:	executable

debug: clean
debug: CXX += -g
debug: CC += -g
debug: executable

executable: $(objects)
	$(CC) $^ -o sdl-app $(LibPath) $(Libs)

$(gen_objs): $(OBJDIR)/%.o: generator/%.cpp
	$(CC) -c $< -o $@ $(IncPath)
$(wld_objs): $(OBJDIR)/%.o: world/%.cpp
	$(CC) -c $< -o $@ $(IncPath)
$(obj_objs): $(OBJDIR)/%.o: object/%.cpp
	$(CC) -c $< -o $@ $(IncPath)
$(cle_objs): $(OBJDIR)/%.o: clengine/%.cpp
	$(CC) -c $< -o $@ $(IncPath)
$(utl_objs): $(OBJDIR)/%.o: util/%.cpp
	$(CC) -c $< -o $@ $(IncPath)
$(___objs): $(OBJDIR)/%.o: %.cpp
	$(CC) -c $< -o $@ $(IncPath)


clean:
	rm -Rf *.o sdl-app
	rm -Rf build/*

setup:
	sudo ./run.sh
