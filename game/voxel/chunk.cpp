#include "chunk.h"

Chunk::Chunk() {
	m_Blocks = new Block**[CHUNK_SIZE];
	for( int i=0; i<CHUNK_SIZE; i++ ) {
		m_Blocks[i] = new Block*[CHUNK_SIZE];
		for( int j=0; j<CHUNK_SIZE; j++ ) {
			m_Blocks[i][j] = new Block[CHUNK_SIZE];
		}
	}
}

Chunk::~Chunk() {
	for( int i=0; i<CHUNK_SIZE; i++ ) {
		for( int j=0; j<CHUNK_SIZE; j++ ) {
			delete m_Blocks[i][j];
		}
		delete m_Blocks[i];
	}
	delete m_Blocks;
}

Chunk::setDensityDist(vec4<float> Origin, DistributionFunction func) {
	vec4<float> pos;
	for( int i=0; i<CHUNK_SIZE; i++ ) {
		for( int j=0; j<CHUNK_SIZE; j++ ) {
			for( int k=0; k<CHUNK_SIZE; k++ ) {
				pos = {i,j,k,0};
				subeq(pos,Origin);
				m_Blocks[i][j][k][bDensity] = func(pos);
			}
		}
	}
}


