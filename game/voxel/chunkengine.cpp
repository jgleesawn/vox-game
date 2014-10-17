#include "chunkengine.h"

ChunkEngine::ChunkEngine(CLEngine * cle_in) : PhysicsEngine("game/voxel/physics.cl", cle_in) { 
	int err;
	LoadKernel("clearBuffer");
	LoadKernel("ChunkKernel");
	LoadKernel("InterChunkKernel");

	backBuffer = createMemObj();
}

ChunkEngine::~ChunkEngine() { 
	int err = clReleaseMemObject(backBuffer);
	if( err < 0 ) { perror("Could not release backBuffer."); fprintf(stderr,"%i\n",err); exit(1); }
}


//Probably don't need the TEXTUREACCESS_TARGET for access in opencl.
cl_mem ChunkEngine::createMemObj(Chunk * c) {
//For use with dynamic CHUNK_SIZE
//	if( c )
//		const int CHUNK_SIZE = c->CHUNK_SIZE;
//	else
//		const int CHUNK_SIZE = 16;

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


	cl_mem_flags flags = CL_MEM_READ_WRITE;
	Block *** ptr = NULL;
	if( c != NULL ) {
		flags |= CL_MEM_COPY_HOST_PTR;
		ptr = c->getPtr();
	}
		
	int err;
//if it breaks, its probably the pointer.
	cl_mem ret = clCreateImage(cle->getContext(), flags, &cif, &cid, ptr, &err);
	if( err < 0 ) { fprintf(stderr, "%i\n", err); perror("Could not create 3d image buffer."); exit(1); }

	return ret;
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

	cl_mem temp = backBuffer;
	backBuffer = cp->focus;
	cp->focus = temp;

	kernel = kernels[2];
	size_t ctrOrigin[3];
	size_t adjOrigin[3];
	size_t region[3];
	for( int i=0; i<6; i++ ){
		if( sideOut[i] == 0 )
			continue;
		sideOut[i] = i;
		clEnqueueWriteBuffer(cle->getQueue(), clSideOut, CL_FALSE, 0, sizeof(cl_int), &sideOut[i], 0, NULL, NULL);
		if(err != CL_SUCCESS) { perror("Error writing dir int."); exit(1); }

//using backBuffer for both because each will write to the opposite sides of it.
//will not have overwritten data.	
		err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &cp->focus);
		err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &backBuffer);
		err |= clSetKernelArg(kernel, 2, sizeof(cl_mem), &cp->surrounding[i]);
		err |= clSetKernelArg(kernel, 3, sizeof(cl_mem), &backBuffer);
		err |= clSetKernelArg(kernel, 4, sizeof(cl_mem), &clSideOut);

		if(err != CL_SUCCESS) { perror("Error setting kernel2 arguments."); fprintf(stderr, "%i\n", err); exit(1); }

		err = clEnqueueNDRangeKernel(cle->getQueue(), kernel, 3, NULL, globalNum, localNum, 0, NULL, NULL);
		if(err != CL_SUCCESS) { perror("Error queuing kernel0 for execution."); fprintf(stderr, "%i\n", err); exit(1); }

		for( int j=0; j<3; j++ ) {
			if( j == i/2 ) {
				region[j] = 1;
				ctrOrigin[j] = (i%2)*(CHUNK_SIZE-1);
				adjOrigin[j] = ((i+1)%2)*(CHUNK_SIZE-1);
			} else {
				region[j] = CHUNK_SIZE;
				ctrOrigin[j] = 0;
				adjOrigin[j] = 0;
			}
		}
		
		err = clEnqueueCopyImage(cle->getQueue(), backBuffer, cp->focus, ctrOrigin, ctrOrigin, region, 0, NULL, NULL);
		err = clEnqueueCopyImage(cle->getQueue(), backBuffer, &cp->surrounding[i], adjOrigin, adjOrigin, region, 0, NULL, NULL);
	}

	clFinish(cle->getQueue());
	clReleaseMemObject(clSideOut);
}


