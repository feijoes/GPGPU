#include <stdio.h>
#include <stdlib.h>
#include "cuda.h"

#define CUDA_CHK(ans) { gpuAssert((ans), __FILE__, __LINE__); }
inline void gpuAssert(cudaError_t code, const char *file, int line, bool abort=true)
{
   if (code != cudaSuccess) 
   {
      fprintf(stderr,"GPUassert: %s %s %d\n", cudaGetErrorString(code), file, line);
      if (abort) exit(code);
   }
}

__device__ int modulo(int a, int b){
	int r = a % b;
	r = (r < 0) ? r + b : r;
	return r;
}
__device__ bool pertenece_submatriz(int fila, int columna, int tam_x, int tam_y) {
    return fila <= tam_y && columna <= tam_x;
}

__global__ void procesarMatriz_unidimensional(int* A, int N, int i1, int j1, int i2, int j2, int val) { 
    int id = blockIdx.x * blockDim.x + threadIdx.x;

    int tam_x = j2 - j1 + 1;
    int tam_y = i2 - i1 + 1;
   
    int fila = i1 + id / tam_x;
    int columna = j1 + modulo(id, tam_y);
    
    int idx = fila * N + columna;

    if (pertenece_submatriz(fila, columna, tam_x, tam_y)) {
        A[idx] = A[idx] + val;
    }
 }

__global__ void procesarMatriz_bidimensional(int* A, int N, int i1, int j1, int i2, int j2, int val) {
    int fila = i1 + blockIdx.y * blockDim.y + threadIdx.y;
    int columna = j1 + blockIdx.x * blockDim.x + threadIdx.x;

    int idx = fila * N + columna;
    int tam_x = j2 - j1 + 1;
    int tam_y = i2 - i1 + 1;

    if (pertenece_submatriz(fila, columna, tam_x, tam_y)) {    
        A[idx] = A[idx] + val;
    }
 }

int main(int argc, char* argv[]) {
    int N;
    int val;
    int i1;
    int j1;
    int i2;
    int j2;


    if (argc < 2) 
    {
        printf("Indicar el tamaño de la matriz"); exit(1);
    }
    if (argc < 3) 
    {
        printf("Indicar valor a sumar a la matriz"); exit(1);
    }
    if (argc < 7) 
    {
        printf("indicar los valores i1, j1, i2, j2 para la submatriz"); exit(1);
    }
	else {
		N = atoi(argv[1]);
		val = atoi(argv[2]);
        i1 = atoi(argv[3]);
        j1 = atoi(argv[4]);
        i2 = atoi(argv[5]);
        j2 = atoi(argv[6]);
    }
    int tam_submatriz = (i2 - i1 + 1) * (j2 - j1 + 1);

    int* h_A = (int*)malloc(N*N*sizeof(int));
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            h_A[i*N + j] = 0;
        }
    }
    printf("Matriz inicializada todo en 0\n\n");

    int* d_A;    
    int num_bloques;

    CUDA_CHK(cudaMalloc((void**)&d_A, N*N*sizeof(int)));
    CUDA_CHK(cudaMemcpy(d_A, h_A, N*N*sizeof(int), cudaMemcpyHostToDevice));
    // medir tiempo de ejecución, encontrado en https://stackoverflow.com/questions/54198279/timings-differ-while-measuring-a-cuda-kernel
    cudaEvent_t start, stop;
    float ms = 0.0f;
    CUDA_CHK(cudaEventCreate(&start));
    CUDA_CHK(cudaEventCreate(&stop));

    // grilla de hilos unidimensional

    int tam_bloque = 256;
    dim3 blockSize1D(tam_bloque, 1);
    if (tam_submatriz % tam_bloque == 0 ){
        num_bloques = tam_submatriz/tam_bloque;
    }else
        num_bloques = tam_submatriz / tam_bloque + 1;       

    dim3 gridSize1D(num_bloques, 1);

    printf("Grilla unidimensional: \n");
    printf("num_bloques: %d\n", num_bloques);
    printf("tam_bloque: %d\n", tam_bloque);
    printf("tam_submatriz: %d\n", tam_submatriz);
    printf("extra threads: %d\n", num_bloques * tam_bloque - tam_submatriz);
    CUDA_CHK(cudaEventRecord(start));
    procesarMatriz_unidimensional<<<gridSize1D, blockSize1D>>>(d_A, N, i1, j1, i2, j2, val);
    CUDA_CHK(cudaEventRecord(stop));

    CUDA_CHK(cudaEventSynchronize(stop));
    CUDA_CHK(cudaEventElapsedTime(&ms, start, stop));
    printf("Tiempo kernel: %f ms\n", ms);

    CUDA_CHK(cudaGetLastError());
    CUDA_CHK(cudaDeviceSynchronize());

    CUDA_CHK(cudaMemcpy(h_A, d_A, N*N*sizeof(int), cudaMemcpyDeviceToHost));

    if (N <= 10) {  
        printf("\nMatriz procesada\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", h_A[i*N + j]);
            }
            printf("\n");
        }
    }

    printf("\n");

    // grilla de hilos bidimensional

    // resetear la matriz a 0
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < N; j++) {
            h_A[i*N + j] = 0;
        }
    }
    CUDA_CHK(cudaMemcpy(d_A, h_A, N*N*sizeof(int), cudaMemcpyHostToDevice));

    int tam_x = j2 - j1 + 1;
    int tam_y = i2 - i1 + 1;

    int tam_bloque_x = 16;
    int tam_bloque_y = 16;

    int num_bloques_x;
    int num_bloques_y;

    if (tam_x % tam_bloque_x == 0 ){
        num_bloques_x = tam_x/tam_bloque_x;
    }else
        num_bloques_x = tam_x / tam_bloque_x + 1;       
        
    if (tam_y % tam_bloque_y == 0 ){
        num_bloques_y = tam_y/tam_bloque_y;
    }else
        num_bloques_y = tam_y / tam_bloque_y + 1;    

    printf("Grilla bidimensional: \n");
    printf("tam_bloque: (%d, %d)\n", tam_bloque_x, tam_bloque_y);
    printf("num_bloques: (%d, %d)\n", num_bloques_x, num_bloques_y);
    printf("tam_submatriz: (%d, %d)\n", tam_x, tam_y);
    printf("extra threads: (%d, %d)\n", num_bloques_x * tam_bloque_x - tam_x, num_bloques_y * tam_bloque_y - tam_y);
    dim3 gridSize2D(num_bloques_x, num_bloques_y);
    dim3 blockSize2D(tam_bloque_x, tam_bloque_y);

    CUDA_CHK(cudaEventRecord(start));
    procesarMatriz_bidimensional<<<gridSize2D, blockSize2D>>>(d_A, N, i1, j1, i2, j2, val);
    CUDA_CHK(cudaEventRecord(stop));

    CUDA_CHK(cudaEventSynchronize(stop));
    CUDA_CHK(cudaEventElapsedTime(&ms, start, stop));
    printf("Tiempo kernel: %f ms\n\n", ms);

    CUDA_CHK(cudaEventDestroy(start));
    CUDA_CHK(cudaEventDestroy(stop));

    CUDA_CHK(cudaGetLastError());
    CUDA_CHK(cudaDeviceSynchronize());

    CUDA_CHK(cudaMemcpy(h_A, d_A, N*N*sizeof(int), cudaMemcpyDeviceToHost));

    if (N <= 10) {  
        printf("\nMatriz procesada\n");
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < N; j++) {
                printf("%d ", h_A[i*N + j]);
            }
            printf("\n");
        }
    }

    free(h_A);
    cudaFree(d_A);
    return 0;
}