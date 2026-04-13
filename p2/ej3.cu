#include <cuda_runtime.h>
#include <float.h>
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

bool esTranspuesta(const int* in, const int* out, int N) {
    for (int fila = 0; fila < N; fila++) {
        for (int col = 0; col < N; col++) {
            if (out[col * N + fila] != in[fila * N + col]) {
                return false;
            }
        }
    }
    return true;
}

__global__ void kernel(int* in, int* out, int N) {
    int fila = blockIdx.y * blockDim.y + threadIdx.y;
    int col  = blockIdx.x * blockDim.x + threadIdx.x;

    if (fila < N && col < N) {
        out[col * N + fila] = in[fila * N + col];
    }
}

float ejecutarKernel(int N, int blockSizeX, int blockSizeY) {
    
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
    cudaEventDestroy(start);
    cudaEventDestroy(stop);
    CUDA_CHK(cudaGetLastError());
    CUDA_CHK(cudaDeviceSynchronize());
    CUDA_CHK(cudaMemcpy(h_B, d_B, size, cudaMemcpyDeviceToHost));
    cudaFree(d_A);
    cudaFree(d_B);
    free(h_A);
    free(h_B);

    return ms;
}

int main(int argc, char* argv[]) {
    int N = 2048;
    float ms = ejecutarKernel(N, 32, 32);
    printf("Tiempo kernel (32x32): %f ms\n", ms);
    ms = ejecutarKernel(N, 8, 16);
    printf("Tiempo kernel (8x16): %f ms\n", ms);

    return 0;
}