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

//fix static CHUNK_SIZE;
class ChunkEngine : public PhysicsEngine {
	cl_mem backBuffer;
public:
	ChunkEngine(CLEngine * cle_in=NULL);
	~ChunkEngine();

	static const int CHUNK_SIZE = 16;

	cl_mem createMemObj(Chunk * c = NULL);

	void Step(void *);
};

#endif
