/*
 * Ejercicio 2.1/2.2 - Multiplicación de matrices por bloques y variante lineal ikj.
 * Uso: ./matmul_blocks N [BS_min BS_max BS_step]   -> barrido de BS
 *      ./matmul_blocks N compare                    -> compara bloqueado (BS=96) vs lineal ikj
 */
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <string>

/* Requiere N % BS == 0; bucles internos sin comprobación de borde. */
static void matrix_mult_blocks(float* A, float* B, float* C, int N, int BS) {
    int i, j, k, ii, jj, kk;
    for (ii = 0; ii < N; ii += BS)
        for (jj = 0; jj < N; jj += BS)
            for (kk = 0; kk < N; kk += BS)
                for (i = ii; i < ii + BS; i++)
                    for (j = jj; j < jj + BS; j++)
                        for (k = kk; k < kk + BS; k++)
                            C[i * N + j] += A[i * N + k] * B[k * N + j];
}

/* Orden de bloques ii, kk, jj: reutiliza el bloque de A en caché. Requiere N % BS == 0. */
static void matrix_mult_blocks_ikkjj(float* A, float* B, float* C, int N, int BS) {
    int i, j, k, ii, jj, kk;
    for (ii = 0; ii < N; ii += BS)
        for (kk = 0; kk < N; kk += BS)
            for (jj = 0; jj < N; jj += BS)
                for (i = ii; i < ii + BS; i++)
                    for (j = jj; j < jj + BS; j++)
                        for (k = kk; k < kk + BS; k++)
                            C[i * N + j] += A[i * N + k] * B[k * N + j];
}

static void matrix_mult_ikj(float* A, float* B, float* C, int N) {
    int i, j, k;
    for (i = 0; i < N; i++)
        for (k = 0; k < N; k++)
            for (j = 0; j < N; j++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}

static void fill_random(float* p, size_t n) {
    for (size_t i = 0; i < n; ++i)
        p[i] = (float)(rand() % 100) / 100.0f;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " N [BS_min BS_max BS_step]\n";
        std::cerr << "     " << argv[0] << " N compare\n";
        return 1;
    }
    int N = std::atoi(argv[1]);
    if (N <= 0) {
        std::cerr << "N debe ser positivo.\n";
        return 1;
    }
    std::cerr << "[matmul_blocks] N = " << N << "\n";

    size_t total = (size_t)N * N;
    size_t bytes = total * sizeof(float);
    double n3 = (double)N * N * N;

    std::cerr << "[matmul_blocks] Allocating 3 matrices (" << (3 * bytes / (1024 * 1024)) << " MB)...\n";
    float* A = new float[total];
    float* B = new float[total];
    float* C = new float[total];

    std::cerr << "[matmul_blocks] Filling A, B with random data...\n";
    srand(12345);
    fill_random(A, total);
    fill_random(B, total);

    bool compare_mode = (argc >= 3 && std::string(argv[2]) == "compare");
    int BS_OPT = 60;  /* valor por defecto: ./matmul_blocks N compare [BS]; 60 = óptimo típico para N=1800 */
    if (argc >= 4 && compare_mode)
        BS_OPT = std::atoi(argv[3]);

    if (compare_mode) {
        std::cerr << "[matmul_blocks] Mode: compare (BS_OPT=" << BS_OPT << ")\n";
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "N = " << N << " (3 matrices ~ " << (3 * bytes / (1024 * 1024)) << " MB)";
        if (N % BS_OPT == 0)
            std::cout << "  [N múltiplo de " << BS_OPT << "]";
        std::cout << "\n\n";

        if (N % BS_OPT == 0) {
            std::cerr << "[matmul_blocks] Running bloqueado (ii,jj,kk)...\n";
            std::memset(C, 0, bytes);
            auto start = std::chrono::high_resolution_clock::now();
            matrix_mult_blocks(A, B, C, N, BS_OPT);
            auto end = std::chrono::high_resolution_clock::now();
            double ms_b = std::chrono::duration<double, std::milli>(end - start).count();
            double mflops_b = n3 / (ms_b / 1000.0 * 1e6);
            std::cerr << "[matmul_blocks]   -> " << ms_b << " ms, " << mflops_b << " MFLOPS\n";
            std::cout << "Bloqueado (ii,jj,kk)\ttiempo_ms=" << ms_b << "\tMFLOPS=" << mflops_b << "\n";

            std::cerr << "[matmul_blocks] Running bloqueado (ii,kk,jj)...\n";
            std::memset(C, 0, bytes);
            start = std::chrono::high_resolution_clock::now();
            matrix_mult_blocks_ikkjj(A, B, C, N, BS_OPT);
            end = std::chrono::high_resolution_clock::now();
            double ms_ikkjj = std::chrono::duration<double, std::milli>(end - start).count();
            double mflops_ikkjj = n3 / (ms_ikkjj / 1000.0 * 1e6);
            std::cerr << "[matmul_blocks]   -> " << ms_ikkjj << " ms, " << mflops_ikkjj << " MFLOPS\n";
            std::cout << "Bloqueado (ii,kk,jj)\ttiempo_ms=" << ms_ikkjj << "\tMFLOPS=" << mflops_ikkjj << "\n";
        }

        std::cerr << "[matmul_blocks] Running lineal ikj...\n";
        std::memset(C, 0, bytes);
        auto start = std::chrono::high_resolution_clock::now();
        matrix_mult_ikj(A, B, C, N);
        auto end = std::chrono::high_resolution_clock::now();
        double ms_ikj = std::chrono::duration<double, std::milli>(end - start).count();
        double mflops_ikj = n3 / (ms_ikj / 1000.0 * 1e6);
        std::cerr << "[matmul_blocks]   -> " << ms_ikj << " ms, " << mflops_ikj << " MFLOPS\n";
        std::cout << "Lineal ikj\t\t\ttiempo_ms=" << ms_ikj << "\tMFLOPS=" << mflops_ikj << "\n";

        std::cerr << "[matmul_blocks] Compare done. Freeing memory.\n";
        delete[] A;
        delete[] B;
        delete[] C;
        return 0;
    }

    std::cerr << "[matmul_blocks] Mode: barrido de BS\n";
    std::cout << "N = " << N << " (3 matrices ~ " << (3 * bytes / (1024 * 1024)) << " MB)\n";

    int bs_min = 8, bs_max = std::min(512, N), bs_step = 8;
    if (argc >= 5) {
        bs_min = std::atoi(argv[2]);
        bs_max = std::atoi(argv[3]);
        bs_step = std::atoi(argv[4]);
        std::cerr << "[matmul_blocks] BS_min=" << bs_min << " BS_max=" << bs_max << " BS_step=" << bs_step << "\n";
    }

    /* Solo considerar BS que dividen a N (N = BS * x); evita bloques parciales.
     * Si se dio BS_step, solo se prueban divisores alineados: bs_min, bs_min+step, ... */
    std::vector<int> divisores;
    for (int d = 1; d <= N; d++) {
        if (N % d != 0 || d < bs_min || d > bs_max)
            continue;
        if (bs_step > 0 && (d - bs_min) % bs_step != 0)
            continue;
        divisores.push_back(d);
    }

    std::cerr << "[matmul_blocks] Found " << divisores.size() << " divisors of N in [" << bs_min << "," << bs_max << "]";
    if (bs_step > 0)
        std::cerr << " (step=" << bs_step << ")";
    std::cerr << ". Starting sweep.\n";
    std::cout << "BS_min=" << bs_min << " BS_max=" << bs_max;
    if (bs_step > 0)
        std::cout << " BS_step=" << bs_step;
    std::cout << " (solo divisores de N)\n";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << "BS\ttiempo_ms\tMFLOPS\n";

    double best_mflops = 0;
    int best_bs = 0;
    int idx = 0;
    for (int BS : divisores) {
        idx++;
        std::cerr << "[matmul_blocks]   [" << idx << "/" << divisores.size() << "] Testing BS=" << BS << " ... ";
        std::cerr.flush();
        std::memset(C, 0, bytes);
        auto start = std::chrono::high_resolution_clock::now();
        matrix_mult_blocks(A, B, C, N, BS);
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
        double mflops = n3 / (ms / 1000.0 * 1e6);
        std::cerr << ms << " ms, " << mflops << " MFLOPS\n";
        std::cout << BS << "\t" << ms << "\t" << mflops << "\n";
        if (mflops > best_mflops) {
            best_mflops = mflops;
            best_bs = BS;
        }
    }

    std::cerr << "[matmul_blocks] Sweep done. Best BS = " << best_bs << " (" << best_mflops << " MFLOPS)\n";
    std::cout << "\nMejor BS = " << best_bs << " (" << best_mflops << " MFLOPS)\n";

    std::cerr << "[matmul_blocks] Freeing memory.\n";
    delete[] A;
    delete[] B;
    delete[] C;
    return 0;
}
