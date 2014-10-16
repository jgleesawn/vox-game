#ifndef CHUNKMANAGER_H
#define CHUNKMANAGER_H

#include <vector>

#include "chunk.h"
#include "chunkengine.h"

class ChunkManager {
	ChunkEngine ce;
	std::vector<Chunk> m_ChunkList;
public:
	ChunkManager();
};

#endif
