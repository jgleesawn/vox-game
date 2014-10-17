#ifndef BLOCK_H
#define BLOCK_H

#include "util/othertypes.h"

enum bProps {
	bDensity,
	bRigidity,
	bPermeability,
	bAllowed
};

class Block {
	vec4<float> props;
public:
	Block( float den=0, float rig=0 );
	float& operator[](int idx) { return props.data[idx]; }
}


#endif
