#include "chunkengine.h"

ChunkEngine::ChunkEngine(CLEngine * cle_in) : PhysicsEngine("game/voxel/physics.cl", cle_in) { 
	int err;
	LoadKernel("clearBuffer");

	LoadKernel("ChunkKernel");
	input.back().push_back( clCreateBuffer(cle->getContext(), CL_MEM_READ_WRITE, 6*sizeof(cl_int), NULL, &err ));
	if( err < 0 ) { perror("Could not create int buffer."); exit(1); }

	LoadKernel("InterChunkKernel");
	input.back().push_back( clCreateBuffer(cle->getContext(), CL_MEM_READ_ONLY, sizeof(cl_int), NULL, &err ));
	if( err < 0 ) { perror("Could not create int buffer."); exit(1); }

	backBuffer = createMemObj();
}

ChunkEngine::~ChunkEngine() { 
	int err = clReleaseMemObject(backBuffer);
	if( err < 0 ) { perror("Could not release backBuffer."); fprintf(stderr,"%i\n",err); exit(1); }
}


//Probably don't need the TEXTUREACCESS_TARGET for access in opencl.
cl_mem ChunkEngine::createMemObj(Chunk * c) {
	if( c )
		const int CHUNK_SIZE = c->CHUNK_SIZE;
	else
		const int CHUNK_SIZE = 16;

	cl_image_format cif;
	cif.image_channel_order = CL_RGBA;
	cif.image_channel_data_type = CL_FLOAT;

	cl_image_desc cid;
	cid.image_type = CL_MEM_OBJECT_IMAGE3D;
	cid.image_width = CHUNK_SIZE;
	cid.image_height = CHUNK_SIZE;
	cid.image_depth = CHUNK_SIZE;
	cid.image_array_size = 1;
	cid.image_row_pitch = 0;
	cid.image_slice_pitch = 0
	cid.num_mip_levels = 0;
	cid.num_samples = 0;
	cid.buffer = NULL;

	int err;
	cl_mem ret = clCreateImage(cle->getContext(), CL_MEM_READ_WRITE, &cif, &cid, c->getPtr(), &err);
	if( err < 0 ) { fprintf(stderr, "%i\n", err); perror("Could not create 3d image buffer."); exit(1); }

	return ret;
}

void ChunkEngine::addTexture(SDL_Texture * newTex) {
	TextureProperty tp = getTexId(newTex);
//	fprintf(stderr, "tid: %i\nttype: %i\n", tp.tid, tp.ttype );

	int err;
	input.back().push_back( clCreateFromGLTexture2D(cle->getContext(), CL_MEM_READ_WRITE, tp.ttype, 0, tp.tid, &err) );
	if( err < 0 ) { fprintf(stderr, "%i\n", err); perror("Could not create texture buffer."); exit(1); }
}

void ChunkEngine::Step(void * in) {
	glFinish();

	int err;

	cePasser * cp = (cePasser *) in;

//globalNum should be based off CHUNK_SIZE instead	
	size_t globalNum[3];
	size_t localNum[3];
	for( int i=0; i<3; i++) {
		globalNum[i] = 16;
		localNum[i] = 8;

		if( globalNum[i] < localNum[i] )
			localNum[i] = globalNum[i];
		else
			globalNum[i] += globalNum[i] % 8;
	}

	cl_kernel kernel;

//May not need to use this to clear backBuffer.
	kernel = kernels[0];
	err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &backBuffer);
	if(err != CL_SUCCESS) { perror("Error setting kernel0 arguments."); fprintf(stderr, "%i\n", err); exit(1); }
	err = clEnqueueNDRangeKernel(cle->getQueue(), kernel, 3, NULL, globalNum, localNum, 0, NULL, NULL);
	if(err != CL_SUCCESS) { perror("Error queuing kernel0 for execution."); fprintf(stderr, "%i\n", err); exit(1); }

	kernel = kernels[1];
	cl_mem clSideOut = clCreateBuffer( cle->getContext(), CL_MEM_READ_WRITE, 6*sizeof(cl_int), NULL, &err);
	if(err != CL_SUCCESS) { perror("Error creating clSideOut buffer."); fprintf(stderr, "%i\n", err); exit(1); }
	err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), cp->focus);
	err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &backBuffer);
	err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &clSideOut);
	if(err != CL_SUCCESS) { perror("Error setting kernel1 arguments."); fprintf(stderr, "%i\n", err); exit(1); }

	err = clEnqueueNDRangeKernel(cle->getQueue(), kernel, 3, NULL, globalNum, localNum, 0, NULL, NULL);
	if(err != CL_SUCCESS) { perror("Error queuing kernel0 for execution."); fprintf(stderr, "%i\n", err); exit(1); }

	int sideOut[6];
	err = clEnqueueReadBuffer(cle->getQueue(), clSideOut, CL_TRUE, 0, sizeof(cl_float), sideOut, 0, NULL, NULL);
	if(err != CL_SUCCESS) { perror("Error reading CL's movMod."); fprintf(stderr, "%i\n", err); exit(1); }

	kernel = kernels[2];

	for( int i=0; i<6; i++ ){
		if( sideOut[i] == 0 )
			continue;
		sideOut[i] = i;
		clEnqueueWriteBuffer(cle->getQueue(), clSideOut, CL_FALSE, 0, sizeof(cl_int), &sideOut[i], 0, NULL, NULL);
		if(err != CL_SUCCESS) { perror("Error writing dir int."); exit(1); }
	
		err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input[0][0]);
		err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &input[0][1]);
		err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &input[0][2]);
		err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &input[0][3]);
		err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &clSideOut);

		if(err != CL_SUCCESS) { perror("Error setting kernel2 arguments."); fprintf(stderr, "%i\n", err); exit(1); }

		err = clEnqueueNDRangeKernel(cle->getQueue(), kernel, 3, NULL, globalNum, localNum, 0, NULL, NULL);
		if(err != CL_SUCCESS) { perror("Error queuing kernel0 for execution."); fprintf(stderr, "%i\n", err); exit(1); }
	}

	clFinish(cle->getQueue());
	clReleaseMemObject(clSideOut);
}


