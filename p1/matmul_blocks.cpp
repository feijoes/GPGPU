/*
 * Ejercicio 2.1 - Multiplicación de matrices por bloques.
 * Encontrar el BS óptimo para matrices mayores que la LLC.
 * Uso: ./matmul_blocks N [BS_min BS_max BS_step]
 *      Si no se dan BS_*, se barre BS desde 8 hasta min(256, N) en potencias de 2 y algunos intermedios.
 */
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cmath>

static void matrix_mult_blocks(float* A, float* B, float* C, int N, int BS) {
    int i, j, k, ii, jj, kk;
    int b = BS;
    for (ii = 0; ii < N; ii += b)
        for (jj = 0; jj < N; jj += b)
            for (kk = 0; kk < N; kk += b)
                for (i = ii; i < ii + b && i < N; i++)
                    for (j = jj; j < jj + b && j < N; j++)
                        for (k = kk; k < kk + b && k < N; k++)
                            C[i * N + j] += A[i * N + k] * B[k * N + j];
}

static void fill_random(float* p, size_t n) {
    for (size_t i = 0; i < n; ++i)
        p[i] = (float)(rand() % 100) / 100.0f;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " N [BS_min BS_max BS_step]\n";
        return 1;
    }
    int N = std::atoi(argv[1]);
    if (N <= 0) {
        std::cerr << "N debe ser positivo.\n";
        return 1;
    }

    size_t total = (size_t)N * N;
    size_t bytes = total * sizeof(float);
    std::cout << "N = " << N << " (3 matrices ~ " << (3 * bytes / (1024 * 1024)) << " MB)\n";

    float* A = new float[total];
    float* B = new float[total];
    float* C = new float[total];

    srand(12345);
    fill_random(A, total);
    fill_random(B, total);

    int bs_min = 8, bs_max = std::min(512, N), bs_step = 8;
    if (argc >= 5) {
        bs_min = std::atoi(argv[2]);
        bs_max = std::atoi(argv[3]);
        bs_step = std::atoi(argv[4]);
    }

    std::cout << "BS_min=" << bs_min << " BS_max=" << bs_max << " step=" << bs_step << "\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "BS\ttiempo_ms\tMFLOPS\n";

    double best_mflops = 0;
    int best_bs = 0;

    for (int BS = bs_min; BS <= bs_max; BS += bs_step) {
        std::memset(C, 0, bytes);
        auto start = std::chrono::high_resolution_clock::now();
        matrix_mult_blocks(A, B, C, N, BS);
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        double n3 = (double)N * N * N;
        double mflops = n3 / (ms / 1000.0 * 1e6);
        std::cout << BS << "\t" << ms << "\t" << mflops << "\n";
        if (mflops > best_mflops) {
            best_mflops = mflops;
            best_bs = BS;
        }
    }

    std::cout << "\nMejor BS = " << best_bs << " (" << best_mflops << " MFLOPS)\n";

    delete[] A;
    delete[] B;
    delete[] C;
    return 0;
}
