#include "block.h"

Block::Block( float den, float rig ) {
	props.data[bDensity] = den;
	props.data[bRigidity] = rig;
}
