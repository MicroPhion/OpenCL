#include <iostream>
#include "CL/cl.h"
void VectorAdd();

int main(){
	std::cout << "Hello World!" << std::endl;
	VectorAdd();
	return 0;  
}  
const char* programSource= {   
	"__kernel void VectorAdd(__global int *a, __global int *b, __global int *c)\n" 
	"{\n"
	"    int gid = get_global_id(0);\n"  
	"    c[gid] = a[gid] + b[gid];\n" 
	"}\n"  
}; 

void VectorAdd(){

	//This code executed on the OpenCL host
	//Host data

	int *A = NULL;//Input Array
	int *B = NULL;//Input Array
	int *C = NULL;//Output Array

	//Elements in each array 
	const int elements = 256;

	//compute the size of the data
	size_t datasize = sizeof(int)*elements;

	//Allocate space for input/output data
	A = (int*)malloc(datasize);
	B = (int*)malloc(datasize);
	C = (int*)malloc(datasize);
	//initialize the inputdata
	for (int i = 0; i < elements; i ++)
	{
		A[i] = i;
		B[i] = i;
		C[i] = 0;
	}

	//use this to check the output of each API call
	cl_int status;

	//1.0 Discover and initialize the platforms

	cl_uint numPlatforms = 0;
	cl_platform_id *platforms = NULL;

	//use clGetPaltfromIDs() to retrieve the number of platforms
	status = clGetPlatformIDs(0,NULL,&numPlatforms);
	//Allocate enough space for each platform 
	platforms = (cl_platform_id *)malloc(numPlatforms*sizeof(cl_platform_id));
	//Fill in platforms with clGetPlatformIDs()
	status = clGetPlatformIDs(numPlatforms,platforms,NULL);



	//2.0 Discover and initialize the devices
	cl_uint numDevices = 0;
	cl_device_id* devices = NULL;
	//use clGetDeviceIDs() to retrieve the number of devices
	status = clGetDeviceIDs(platforms[0],CL_DEVICE_TYPE_ALL,0,NULL,&numDevices);
	//Allocate enough space for each device
	devices = (cl_device_id*)malloc(numDevices*sizeof(cl_device_id));
	//Fill in devices with clGetDeviceIDs()
	status = clGetDeviceIDs(platforms[0],CL_DEVICE_TYPE_ALL,numDevices,devices,NULL);

	//3.0 Create a context
	cl_context context = NULL;
	//Create a context using clCreateContext() and associate it with the devices
	context = clCreateContext(NULL,numDevices,devices,NULL,NULL,&status);

	//4.0 Create a command queue
	cl_command_queue cmdQueue;
	//Create a command queue using clCreateCommandQueue(), and associate it with the device you want to execute on 
	cmdQueue = clCreateCommandQueue(context,devices[0],0,&status);

	//5.0 Create device buffers
	cl_mem bufferA;//input array on the device
	cl_mem bufferB;//input array on the device
	cl_mem bufferC;//output array on the device

	//use clCreateBuffer() to create a buffer object(d_A) that will contain the data from the host array A
	bufferA = clCreateBuffer(context,CL_MEM_READ_ONLY,datasize,NULL,&status);
	//use clCreateBuffer() to create a buffer object(d_B) that will contain the data from the host array B
	bufferB = clCreateBuffer(context,CL_MEM_READ_ONLY,datasize,NULL,&status);
	//use clCreateBuffer() to create a buffer object(d_C) that will contain the data from the host array C
	bufferC = clCreateBuffer(context,CL_MEM_READ_ONLY,datasize,NULL,&status);


	//6.0 wirte host data to device buffers
	//use clEnqueueWriteBuffer() to write input arrayA to the device buffer bufferA
	status = clEnqueueWriteBuffer(cmdQueue,bufferA,CL_FALSE,0,datasize,A,0,NULL,NULL);
	//use clEnqueueWriteBuffer() to write input arrayA to the device buffer bufferA
	status = clEnqueueWriteBuffer(cmdQueue,bufferB,CL_FALSE,0,datasize,B,0,NULL,NULL);
	//use clEnqueueWriteBuffer() to write input arrayA to the device buffer bufferA
	status = clEnqueueWriteBuffer(cmdQueue,bufferC,CL_FALSE,0,datasize,C,0,NULL,NULL);

	//7.0 Create and compile the program 
	//Create a program using clCreateProgramWithSource()
	cl_program program = clCreateProgramWithSource(context,1,(const char**)&programSource,NULL,&status);
	//Build the program for the devices with clBuildProgram()
	status = clBuildProgram(program,numDevices,devices,NULL,NULL,NULL);

	//8.0 Create the kernel 
	cl_kernel kernel = NULL;
	//use clCreateKernel() to create a kernel from the vector addition function( named "VectorAdd")
	kernel = clCreateKernel(program,"VectorAdd",&status);

	//9.0 Set the kernel arguments

	//Associate the input and output buffers with the kernel using clSetKernelArg()
	status = clSetKernelArg(kernel,0,sizeof(cl_mem),&bufferA);
	status = clSetKernelArg(kernel,1,sizeof(cl_mem),&bufferB);
	status = clSetKernelArg(kernel,2,sizeof(cl_mem),&bufferC);

	//10.0 Configure the work-item structure
	//Define an index space (global work size) is not required but can be used
	size_t globalWorkSize[1];
	//There are 'elements' work-items
	globalWorkSize[0] = elements;

	//11.0 Enqueue the kernel for execution
	//Execute the kernel by using clEnqueueNDRangeKernel().'globalWorkSize' is the ID dimension of the work-items
	status = clEnqueueNDRangeKernel(cmdQueue,kernel,1,NULL,globalWorkSize,NULL,0,NULL,NULL);

	//12.0 Read the output buffer back to the host 
	//Use clEnqueueReadBuffer() to read the CL output
	//buffer( bufferC ) to the host output array (C)
	clEnqueueReadBuffer(cmdQueue,bufferC,CL_TRUE,0,datasize,C,0,NULL,NULL);

	//Verify the output
	bool result = true;
	for (int i = 0; i < elements; i ++)
	{
		if(C[i] != i+i){
			result = false;
			break;
		}
	}
	if(result){
		std::cout << "Output is correct!" << std::endl;
	}else{
		std::cout << "Output is incorrect!" << std::endl;
	}


	//13.0 Release OpenCL resources
	//Free OpenCL resources
	clReleaseKernel(kernel);
	clReleaseProgram(program);
	clReleaseCommandQueue(cmdQueue);
	clReleaseMemObject(bufferA);
	clReleaseMemObject(bufferB);
	clReleaseMemObject(bufferC);
	clReleaseContext(context);

	//Free hsot resources
	free(A);
	free(B);
	free(C);
	free(platforms);
	free(devices);

}