#ifndef CHUNK_H
#define CHUNK_H

#include "block.h"
#include "util/othertypes.h"

typedef float (*DistributionFunction)(vec4<float>);

class Chunk {
	Block** m_Blocks;
//	CLEngine * cle;
//	ChunkEngine ce;
public:
	Chunk();
	~Chunk();

	static const int CHUNK_SIZE = 16;

	void setDistribution(vec4<float>, DistributionFunction);
};

float distalDist( vec4<float> pos ) {
	return pos.data[0]*pos.data[0] + pos.data[1]*pos.data[1] + pos.data[2]*pos.data[2] + pos.data[3]*pos.data[3];
}
float nearDist( vec4<float> pos ) {
	float dist_2 = distalDist(pos);
	if( dist_2 < 1 )
		dist_2 = 1;
	return 1/dist_2;
}

#endif
