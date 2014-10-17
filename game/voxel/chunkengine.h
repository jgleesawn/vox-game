#ifndef CHUNKENGINE_H
#define CHUNKENGINE_H

#include <CL/opencl.h>

#include <string>
#include <vector>

#include "clengine/physicsengine.h"
#include "chunk.h"

struct cePasser {
	cl_mem * focus;
	cl_mem * surrounding;
};

class ChunkEngine : public PhysicsEngine {
	cl_mem backBuffer;
public:
	ChunkEngine(CLEngine * cle_in=NULL);
	~ChunkEngine();

	cl_mem createMemObj(Chunk * c = NULL);

	void addTexture(SDL_Texture *);

	void Step(void *);
};

#endif
