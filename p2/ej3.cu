#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

#define CUDA_CHK(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

__global__ void kernel(int* in, int* out, int N) {
    int fila = blockIdx.y * blockDim.y + threadIdx.y;
    int col  = blockIdx.x * blockDim.x + threadIdx.x;

    if (fila < N && col < N) {
        out[col * N + fila] = in[fila * N + col];
    }
}

__global__ void kernel(const int* in, int* out, int N) {
    int col  = blockIdx.x * blockDim.x + threadIdx.x;
    int fila = blockIdx.y * blockDim.y + threadIdx.y;

    if (fila < N && col < N) {
        out[col * N + fila] = in[fila * N + col];
    }
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("Indicar el tamaño de la matriz"); exit(1);
    }
    if (argc < 3) {
        printf("Indicar el tamaño de bloque en x"); exit(1);
    }
    if (argc < 4) {
        printf("Indicar el tamaño de bloque en y"); exit(1);
    }
    int blockSizeX = atoi(argv[2]);
    int blockSizeY = atoi(argv[3]);
    int N = atoi(argv[1]);
    int size = N * N * sizeof(int);

    int* h_A = (int*)malloc(size);
    int* h_B = (int*)malloc(size);

    for (int i = 0; i < N * N; i++) h_A[i] = i;


    int *d_A, *d_B;
    CUDA_CHK(cudaMalloc((void**)&d_A, size));
    CUDA_CHK(cudaMalloc((void**)&d_B, size));

    CUDA_CHK(cudaMemcpy(d_A, h_A, size, cudaMemcpyHostToDevice));

    dim3 blockSize(blockSizeX, blockSizeY);
    dim3 gridSize((N + blockSize.x - 1) / blockSize.x,
                  (N + blockSize.y - 1) / blockSize.y);

    cudaEvent_t start, stop;
    float ms = 0.0f;
    
    cudaEventCreate(&start);
    cudaEventCreate(&stop);
    
    cudaEventRecord(start);
    
    kernel<<<gridSize, blockSize>>>(d_A, d_B, N);
    
    cudaEventRecord(stop);
    cudaEventSynchronize(stop);
    cudaEventElapsedTime(&ms, start, stop);
    
    printf("Tiempo kernel: %f ms\n", ms);
    
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    CUDA_CHK(cudaGetLastError());
    CUDA_CHK(cudaDeviceSynchronize());

    CUDA_CHK(cudaMemcpy(h_B, d_B, size, cudaMemcpyDeviceToHost));

   
    cudaFree(d_A);
    cudaFree(d_B);
    free(h_A);
    free(h_B);

    return 0;
}