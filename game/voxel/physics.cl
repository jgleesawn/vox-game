//Much spent on conversion to floats, push conversion of float to division at the end.
float sum( float3 sumee ) {
	return sumee.x+sumee.y+sumee.z;
}
float avg( float v1, float v2) {
	return (v1+v2)/2;
}

const sampler_t sampler = CLK_NORMALIZED_COORDS_FALSE |
CLK_ADDRESS_CLAMP |
CLK_FILTER_NEAREST;

__kernel void clearBuffer( __write_only image3d_t backBuffer ) {
	pos = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);
	float4 zero = (float4)(0.0, 0.0, 0.0, 0.0);
	write_imagef(backBuffer, pos, zero);
}

//SideOut is int[6], clear to 0 before calling kernel.
__kernel void ChunkKernel( __read_only image3d_t blocks,
			__write_only image3d_t backBuffer,
			__global int * SideOut ) {
	int4 pos, offset;
	int4 bnd,zero,dims;
	float4 cprop, oprop;
	float divCnt = 4;	//Base value for a corner, self+3 surrounding.

	pos = (int4)(get_global_id(0), get_global_id(1), get_global_id(2), 0);
	cprop = read_imagef( blocks, sampler, pos );

	
	zero = (int4)(0,0,0,0);
	dims = get_image_dim( backBuffer );
	dims.w = 1;

	oprop = (float4)(0.0, 0.0, 0.0, 0.0);
	int offval;
	for( int i=0; i<6; i++ ) {
		offval = 2*(i%2) - 1;
		offset = (int4)((i/2 == 0)*offval, (i/2 == 1)*offval, (i/2 == 2)*offval, 0);
		oprop += read_imagef( blocks, sampler, pos+offset );
	}
	bnd = pos == zero;
	sideOut[0] |= bnd.x;
	sideOut[2] |= bnd.y;
	sideOut[4] |= bnd.z;
	bnd = pos == dims;
	sideOut[1] |= bnd.x;
	sideOut[3] |= bnd.y;
	sideOut[5] |= bnd.z;
	bnd |= pos == zero;

//Vector true = -1
	divCnt -= bnd.x;
	divCnt -= bnd.y;
	divCnt -= bnd.z;

	cprop.x += oprop.x/6.0;	//each surround block is surrounded by 6 total
	if( cprop.w == 0.0 )
		cprop.x /= divCnt; //count of all immediates, permits flow out of not-allowed blocks
	else
		cprop.x /= oprop.w+1.0; //count of surrounding+self

	bnd = pos < dims;
	if( bnd.x && bnd.y && bnd.z )
		write_imagef( backBuffer, pos, cprop );
}

__kernel InterChunkKernel( __read_only image3d_t centerR,
			__write_only image3d_t centerW,
			__read_only image3d_t adjR,
			__write_only image3d_t adjW, 
			__global int * adjDir ) {
	int4 dims, cpos, apos;
	float4 cprop, aprop;

	int dir,dirX,dirY,dirZ, buf;
	dir = adjDir[0];

	dims = get_image_dim( centerR );

	dirX = dir/2 == 0;
	dirY = dir/2 == 1;
	dirZ = dir/2 == 2;

	cpos = (int4)( (dirY | dirZ)*get_global_id(0) + (dir%2)*dirX*dims.x,
			(dirX | dirZ)*get_global_id(1) + (dir%2)*dirY*dims.y,
			(dirX | dirY)*get_global_id(2) + (dir%2)*dirZ*dims.z,
			0 );
	apos = (int4)( (dirY | dirZ)*get_global_id(0) + ((dir+1)%2)*dirX*dims.x,
			(dirX | dirZ)*get_global_id(1) + ((dir+1)%2)*dirY*dims.y,
			(dirX | dirY)*get_global_id(2) + ((dir+1)%2)*dirZ*dims.z,
			0 );
	cprop = read_imagef( centerR, sampler, cpos );
	aprop = read_imagef( adjR, sampler, apos );
	buf = cprop.x / 6;
	buf += aprop.x / 6;
	cprop.x *= 5/6;
	aprop.x *= 5/6;

	if( cprop.w > 0.0 ) {
		cprop.x += buf;
	}
	if( aprop.w > 0.0 ) {
		aprop.x += buf;
	}
	buf /= 2.0;
	if( (cprop.w > 0.0 && aprop.w > 0.0) || (cprop.w == 0.0 && aprop.w == 0.0) ) {
		cprop.x -= buf;
		aprop.x -= buf;
	}

	write_imagef( centerW, cpos, cprop );
	write_imagef( adjW, apos, aprop );
}

//#pragma OPENCL EXTENSION cl_khr_gl_sharing : enable
__kernel void Equilibrium(__constant int * wpos, __constant int * hpos,
			__constant int * w, __constant int * h,
			__constant int * spriteFrame, __global float * movMod,
			__local float4 * sum, __global float4 * gsum) { 
			__read_only image3d_t readIm, __write_only image3d_t writeIm) {

	int x = get_global_id(0);
	int y = get_global_id(1);
	int z = get_global_id(2);
	
	int lid = get_local_id(0)+get_local_id(1)*get_local_size(0);
	sum[lid] = (float4)(0.0,0.0,0.0,1.0);//0);

	int x = get_global_id(0);
	int y = get_global_id(1);	
	int bgx = wpos[0]+x;
	int bgy = hpos[0]+y;


	float4 stp = read_imagef( sTex, sampler, (int2)(spriteFrame[0]*w[0]+x,y) );
	sum[lid] = fabs( stp - read_imagef(bgTex,sampler,(int2)(bgx,bgy)) );
//	sum[lid] = abs_diff( stp, read_imagef(bgTex,sampler,(int2)(bgx,bgy)) );

	if( stp.w > 0.0 )
		sum[lid].w = 1;
	else
		sum[lid] = (float4)(0.0,0.0,0.0,0.0);

	barrier(CLK_LOCAL_MEM_FENCE);
	float4 sumd = (float4)(0.0, 0.0, 0.0, 0.0);
	if( lid == 0 ) {
		for( int i=0; i<get_local_size(0)*get_local_size(1); i++)
			sumd += sum[i];
		gsum[get_group_id(0)+get_group_id(1)*get_num_groups(0)] = sumd;
	}

	barrier(CLK_GLOBAL_MEM_FENCE);
	if( get_global_id(0)+get_global_id(1) ==0 ) {
		sumd = (float4)(0.0, 0.0, 0.0, 0.0);
		for( int i=0; i<get_num_groups(0)*get_num_groups(1); i++)
			sumd += gsum[i];

		float diff = 0;
		diff += sumd.x;
		diff += sumd.y;
		diff += sumd.z;
		diff /= (float)(sumd.w*3);
		diff *= 4;	//decreases slowdown

		movMod[0] = diff;
	}

}

