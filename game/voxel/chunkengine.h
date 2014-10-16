#ifndef CHUNKENGINE_H
#define CHUNKENGINE_H

#include <SDL2/SDL.h>
#include <CL/opencl.h>

#include <string>
#include <vector>

#include "clengine/physicsengine.h"
#include "object/object.h"

struct wePasser {
	SDL_Texture * rendtex;
	Object * obj;
	float movMod;
};

class ChunkEngine : public PhysicsEngine {
public:
	ChunkEngine(CLEngine * cle_in=NULL);
	~ChunkEngine();

	bool Init(SDL_Texture *);

	void addTexture(SDL_Texture *);

	void Step(void *);
};

#endif
