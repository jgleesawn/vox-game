#ifndef OTHERTYPES_H
#define OTHERTYPES_H

template<typename T>
struct vec4 {
	T data[4];
};
template<typename T, typename U>
void subeq( vec4<T>& lhs, vec4<U>& rhs ) {
	for( int i=0; i<4; i++ )
		lhs.data[i] -= rhs.data[i];
}

#endif
