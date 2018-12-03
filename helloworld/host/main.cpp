/**
 * OpenCL Hello World Program
 * Created by Zixiao.Wang @ 2018.12.1
**/
//#include ..common/
//#include <CL/cl.h>

#include "ocl_util.h"
#include <CL/cl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <fstream>

using namespace std;
using namespace ocl_util;

#define VENDOR altera
void cleanup();

//define as global variables
cl_uint numDevices = 0;  
cl_device_id * devices;
cl_uint numPlatforms;   //the number of platforms
cl_platform_id platform = NULL; //the chosen platform
cl_platform_id* platforms;
cl_int status;
cl_context context;//3
cl_command_queue commandQueue;//4
cl_program program;//5
cl_mem inputBuffer;//7
cl_mem outputBuffer;//7
cl_kernel kernel;//8

const char* input = "ABCDEFG";//set what string is going to send to FPGA
char* output;//get the return value from FPGA 


/*convert the kernel file into a string*/
int convertToString(const char* filename, std::string& s)
{
    size_t size;
    char* str;
    std::fstream f(filename, (std::fstream::in | std::fstream::binary));

    if(f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);
        str = new char[size+1];
        if(!str)
        {
            f.close();
            return 0;
        }

        f.read(str, fileSize);
        f.close();
        str[size] = '\0';
        s = str;
        delete[] str;
        return 0;
    }
    cout << "Error: filed to open file\n:" << filename << endl;
    return -1;
}


//main
int main(int argc, char* argv[])
{
    /****************************step1: Getting platforms and choose an available one.**************************************/
    //cl_uint numPlatforms;   //the number of platforms
    //cl_platform_id platform = NULL; //the chosen platform
    status = clGetPlatformIDs(0, NULL, &numPlatforms);       //first call: get number of platforms, saved in 'numPlatforms
    if(status != CL_SUCCESS)
    {
        cout << "Error: Getting platforms!" << endl;
        return 1;
    }
    /*we just choose the first platform.*/
    if(numPlatforms > 0)
    {
        platforms = (cl_platform_id*)malloc(numPlatforms * sizeof(cl_platform_id));
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);   //second call: get the first platform id, saved in 'platforms
        platform = platforms[0];

        //print platform infos
        size_t size;
        cl_int err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, 0, NULL, &size); 
        char* Pname = (char*)malloc(size);
        err = clGetPlatformInfo(platform, CL_PLATFORM_NAME, size, Pname, NULL);
        cout << "CL_PLATFORM_NAME: " << Pname << endl;
        //free(&size);
        free(Pname);
        free(platforms);
    }

    /****************************step2: Query the platform and choose the first ACCLERATOR device if has one,
     * otherwise use th CPU as device**************************************/
    //cl_uint numDevices = 0;  //define as global variables
    //cl_device_id * devices;
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, 0, NULL, &numDevices);//first call: get number of CL_DEVICE_TYPE_ACCELERATOR
    if(numDevices == 0) //No FPGA found, choose CPU
    {
        cout << "No FPGA device is available"<< endl;
        cout << "Choose CPU as defalut device."<< endl;
        status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, 0, NULL, &numDevices);
        devices = (cl_device_id*)malloc(numDevices * sizeof(cl_device_id));

        status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numDevices, devices, NULL);
    }
    else//choose FPGA
    {
        devices =(cl_device_id*)malloc(numDevices * sizeof(cl_device_id));
        status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_ACCELERATOR, numDevices, devices, NULL);//second call: get device ID, saved in 'devices
    }


    //get device info
    size_t size;
    cl_int err = clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &size);
    char* Pname = (char*)malloc(size);
    err = clGetDeviceInfo(devices[0], CL_DEVICE_NAME, size, Pname, NULL);
    cout << "CL_DEVICE_NAME: " << Pname << endl;
    //free(&size);
    free(Pname);

    /*************************step3: Create context**************************/
    context = clCreateContext(NULL, 1, devices, NULL, NULL, NULL);

    /*************************step4: Create command queue assoicate with the context**************************/
    commandQueue = clCreateCommandQueue(context, devices[0], 0, NULL);

    /*************************step5: Create program object**************************/
    //const char* filename = "device/HelloWorld_Kernel.cl";
    if (argc != 2)
    {
        printf("Error: wrong command format, usage: \n");
        printf("%s <binaryfile>\n", argv[0]);
    }
    
    const char* kernel_file_name = argv[1];
    //const char* filename = argv[1];
    //string sourceStr;
    //status = convertToString(filename, sourceStr);
    //const char* source = sourceStr.c_str();
    //size_t sourceSize[] = {strlen(source)};
    //cl_program program = clCreateProgramWithSource(context, 1, &source, sourceSize, NULL);
    program = createProgramFromFile(context, (const char*)kernel_file_name, devices, numDevices);

    /*************************step6: Build program**************************/
    status = clBuildProgram(program, 1, devices, NULL, NULL, NULL);

    /*************************step7: Initial input /output for the host and create memory objects for the kernal**********************/
    //const char* input = "ABCDEFG";
    size_t strlength = strlen(input);
    cout << "input string: "<<endl;
    cout << input << endl;
    output = (char*)malloc(strlength + 1);

    inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,  //create memory objects-inputBuffer
        (strlength + 1) * sizeof(char), (void*)input ,NULL);
    
    outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY,                        //create memory objects-outputBuffer
        (strlength + 1) * sizeof(char), NULL, NULL);

    /*************************step8: Create kernel object**************************/
    kernel = clCreateKernel(program, "helloworld", NULL);

    /*************************step9: Set kernel arguments**************************/
    status = clSetKernelArg(kernel, 0, sizeof(cl_mem), (void*)&inputBuffer);
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), (void*)&outputBuffer);

    /*************************step10: Run the kernel**************************/
    size_t global_work_size[1] = {strlength};
    status = clEnqueueNDRangeKernel(commandQueue, kernel, 1, NULL, global_work_size, NULL, 0, NULL, NULL);

    /*************************step11: Read the cout back to host memory**************************/
    status = clEnqueueReadBuffer(commandQueue, outputBuffer, CL_TRUE, 0,
        strlength * sizeof(char), output, 0, NULL, NULL);
    output[strlength] = '\0';       //add the terminal character to the end of output 
    cout << "output string: "<<endl;
    cout << output << endl;

     /*************************step12: Clean the resources**************************/
    cleanup();
    return 0;
}

//here we define a function void cleanup();
//cleanup() is  declared in ocl_util.cpp

void cleanup()
{
    status = clReleaseKernel(kernel);
    status = clReleaseProgram(program);
    status = clReleaseMemObject(inputBuffer);
    status = clReleaseMemObject(outputBuffer);
    status = clReleaseCommandQueue(commandQueue);
    status = clReleaseContext(context);

    if (output != NULL)
    {
        free(output);
        output = NULL;
    }

    if (devices != NULL)
    {
        free(devices);
        devices = NULL;
    }
}