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
/*
Con 32*32, como vimos en clase, el warp contiene 32 hilos,
En nuestro codigo, para un warp dado, cada hilo lee un int de 4 bytes de la matriz in y escribe un int de 4 bytes en la matriz out, 
En la lectura de in, esos accesos son consecutivos dentro de una fila, se hace un acceso coalesced; en cambio en la escritura de out, no se hace un acceso coalesced,

*/
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

/* N = 16, 32, ..., 4096 (9 valores) */
#define NUM_N 2048

int main(int argc, char* argv[]) {
    float best_ms = FLT_MAX;
    int best_i = 0, best_j = 0;
    int N = 2048;
    for (int i = 1; i < 64; i *= 2) {
        for (int j = 1; j < 64; j *= 2) {
            float ms = ejecutarKernel(N, i, j);
            if (ms < best_ms) {
                best_ms = ms;
                best_i = i;
                best_j = j;
            }
        }
    }




    printf("\n=== Tamaño de bloque óptimo por N (kernel transpuesta) ===\n");
    printf("N=%5d  ->  óptimo blockDim (%d, %d)  tiempo mínimo: %f ms\n",N, best_i, best_j, best_ms);

    return 0;
}