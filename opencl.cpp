#include <CL/cl.h>
#include <vector>
#include <climits>
#include <cstring>
#include <iostream>
#include "common/graph.h"
#include "impl.h"

#define MAX_SOURCE_SIZE (0x100000)
#define BITS_PER_UINT 64
#define MAX_LOCAL_SIZE 32

typedef struct __attribute__ ((packed)) ClGraph {
    cl_int num_edges;
    cl_int num_nodes;

    cl_mem outgoing_starts;
    cl_mem outgoing_edges;
    cl_mem incoming_starts;
    cl_mem incoming_edges;
    cl_mem edges_weight;
} ClGraph;

void checkError(cl_int status, const char* msg) {
    if (status != CL_SUCCESS) {
        std::cerr << "Error: " << msg << " (" << status << ")" << std::endl;
        exit(1);
    }
}

size_t local_size(int n) {
    if (n <= MAX_LOCAL_SIZE) {
        return n;
    } else {
        // Find the maximum factor of n that is <= 32
        for (int factor = 32; factor > 1; factor--) {
            if (n % factor == 0) {
                return factor;
            }
        }
        // If no factor found (unlikely for practical values of n), return 1
        return 1;
    }
}

ClGraph init_cl_graph(Graph graph, cl_context* context, cl_int* status){
    ClGraph deviceGraph;
    deviceGraph.num_nodes = graph->num_nodes;
    deviceGraph.num_edges = graph->num_edges;

    deviceGraph.outgoing_starts = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(int) * num_nodes(graph), graph->outgoing_starts, status);
    deviceGraph.outgoing_edges = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(Vertex) * num_edges(graph), graph->outgoing_edges, status);
    deviceGraph.incoming_starts = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(int) * num_nodes(graph), graph->incoming_starts, status);
    deviceGraph.incoming_edges = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(Vertex) * num_edges(graph), graph->incoming_edges, status);
    deviceGraph.edges_weight = clCreateBuffer(*context, CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR, sizeof(Weight) * num_edges(graph), graph->edges_weight, status);
    
    return deviceGraph;
}

void free_cl_graph(ClGraph* graph){
    clReleaseMemObject(graph->outgoing_starts);
    clReleaseMemObject(graph->outgoing_edges);
    clReleaseMemObject(graph->incoming_starts);
    clReleaseMemObject(graph->incoming_edges);
    clReleaseMemObject(graph->edges_weight);
}

void setKernalArgs(ClGraph& graph, cl_mem& ansBuffer, const int n, cl_int& status, cl_kernel& kernel) {
    status = clSetKernelArg(kernel, 0, sizeof(int), &graph.num_edges);
    checkError(status, "clSetKernelArg(num_edges)");

    status = clSetKernelArg(kernel, 1, sizeof(int), &graph.num_nodes);
    checkError(status, "clSetKernelArg(num_nodes)");

    status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &graph.outgoing_starts);
    checkError(status, "clSetKernelArg(num_nodes)");
    status = clSetKernelArg(kernel, 3, sizeof(cl_mem), &graph.outgoing_edges);
    checkError(status, "clSetKernelArg(num_nodes)");
    status = clSetKernelArg(kernel, 4, sizeof(cl_mem), &graph.incoming_starts);
    checkError(status, "clSetKernelArg(num_nodes)");
    status = clSetKernelArg(kernel, 5, sizeof(cl_mem), &graph.incoming_edges);
    checkError(status, "clSetKernelArg(num_nodes)");
    status = clSetKernelArg(kernel, 6, sizeof(cl_mem), &graph.edges_weight);
    checkError(status, "clSetKernelArg(num_nodes)");

    status = clSetKernelArg(kernel, 7, sizeof(cl_mem), &ansBuffer);
    checkError(status, "clSetKernelArg(ans)");

    status = clSetKernelArg(kernel, 8, sizeof(int), &n);
    checkError(status, "clSetKernelArg(n)");

    status = clSetKernelArg(kernel, 9, sizeof(cl_bool) * n, NULL);
    checkError(status, "clSetKernelArg(visited)");
}

void dijk_opencl(Graph g, Answer ans, const int n) {
    cl_int status;

    // Load the OpenCL source code from file
    FILE* kernelFile;
    char* kernelSource;
    size_t kernelSize;

    kernelFile = fopen("dijkstra_kernel.cl", "r");
    if (!kernelFile) {
        std::cerr << "Error: Unable to open kernel file." << std::endl;
        exit(1);
    }

    kernelSource = (char*)malloc(MAX_SOURCE_SIZE);
    kernelSize = fread(kernelSource, 1, MAX_SOURCE_SIZE, kernelFile);
    fclose(kernelFile);

    // Initialize the OpenCL context and create a command queue
    cl_platform_id platform;
    status = clGetPlatformIDs(1, &platform, NULL);
    checkError(status, "clGetPlatformIDs");

    cl_device_id device;
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
    checkError(status, "clGetDeviceIDs");

    cl_context context = clCreateContext(NULL, 1, &device, NULL, NULL, &status);
    checkError(status, "clCreateContext");

    cl_command_queue queue = clCreateCommandQueue(context, device, 0, &status);
    checkError(status, "clCreateCommandQueue");

    // Create OpenCL buffers
    auto cl_graph = init_cl_graph(g, &context, &status);

    cl_mem ansBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, sizeof(Distance) * n * n, ans[0], &status);
    checkError(status, "clCreateBuffer(ans)");

    size_t visitedSizeInBits = (n + BITS_PER_UINT - 1) / BITS_PER_UINT;
    size_t visitedSizeInBytes = visitedSizeInBits * sizeof(cl_ulong);
    cl_ulong* visitedHost = (cl_ulong*)malloc(visitedSizeInBytes);
    memset(visitedHost, 0, visitedSizeInBytes);
    cl_mem visitedBuffer = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, visitedSizeInBytes, visitedHost, &status);
    checkError(status, "clCreateBuffer(visited)");

    // Create and compile the OpenCL program
    cl_program program = clCreateProgramWithSource(context, 1, (const char**)&kernelSource, (const size_t*)&kernelSize, &status);
    checkError(status, "clCreateProgramWithSource");

    status = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
    // checkError(status, "clBuildProgram");
    if (status == CL_BUILD_PROGRAM_FAILURE) {
        // Determine the size of the log
        size_t log_size;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);

        // Allocate memory for the log
        char *log = (char *) malloc(log_size);

        // Get the log
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);

        // Print the log
        printf("%s\n", log);
        free(log);
        exit(1);
    }

    // Create the OpenCL kernel
    cl_kernel kernel = clCreateKernel(program, "dijkstra_kernel", &status);
    checkError(status, "clCreateKernel");

    // Set the kernel arguments
    setKernalArgs(cl_graph, ansBuffer, n, status, kernel);

    // Set global and local work sizes
    size_t localSize = local_size(n);
    size_t globalWorkSize[] = { static_cast<size_t>(n) };
    size_t localWorkSize[] = { static_cast<size_t>(1) };

    // Execute the OpenCL kernel
    cl_event kernel_event;
    status = clEnqueueNDRangeKernel(queue, kernel, 1, 0, globalWorkSize, localWorkSize, 0, NULL, &kernel_event);
    checkError(status, "clEnqueueNDRangeKernel");

    // Wait for kernel execution to finish
    clWaitForEvents(1, &kernel_event);
    clReleaseEvent(kernel_event);

    // Read the result back from the device
    status = clEnqueueReadBuffer(queue, ansBuffer, CL_TRUE, 0, sizeof(Distance) * n * n, ans[0], 0, NULL, NULL);
    checkError(status, "clEnqueueReadBuffer(ans)");

    // Clean up
    free_cl_graph(&cl_graph);
    clReleaseMemObject(ansBuffer);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);
}
