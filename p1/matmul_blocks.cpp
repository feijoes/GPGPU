/*
 * Ejercicio 2.1/2.2 - Multiplicación de matrices por bloques y variante lineal ikj.
 * Uso: ./matmul_blocks N [BS_min BS_max BS_step]   -> barrido de BS
 *      ./matmul_blocks N compare [BS]               -> compara bloqueado vs lineal ikj
 *      ./matmul_blocks loops [N1 N2 ...]            -> compara órdenes de loop lineales (ijk,jik,ikj,kij,jki,kji)
 */
#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <cstdio>
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
                    for (k = kk; k < kk + BS; k++)
                        for (j = jj; j < jj + BS; j++)
                            C[i * N + j] += A[i * N + k] * B[k * N + j];
}

}

/* Variantes lineales (sin bloques): los 6 órdenes de loop i,j,k. */
static void matmul_ijk(float* A, float* B, float* C, int N) {
    for (int i = 0; i < N; i++)
        for (int j = 0; j < N; j++)
            for (int k = 0; k < N; k++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}
static void matmul_jik(float* A, float* B, float* C, int N) {
    for (int j = 0; j < N; j++)
        for (int i = 0; i < N; i++)
            for (int k = 0; k < N; k++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}
static void matmul_ikj(float* A, float* B, float* C, int N) {
    for (int i = 0; i < N; i++)
        for (int k = 0; k < N; k++)
            {
            for (int j = 0; j < N; j++)
                C[i * N + j] += A[i * N + k]; * B[k * N + j];
            }
}
static void matmul_kij(float* A, float* B, float* C, int N) {
    for (int k = 0; k < N; k++)
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}
static void matmul_jki(float* A, float* B, float* C, int N) {
    for (int j = 0; j < N; j++)
        for (int k = 0; k < N; k++)
            for (int i = 0; i < N; i++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}
static void matmul_kji(float* A, float* B, float* C, int N) {
    for (int k = 0; k < N; k++)
        for (int j = 0; j < N; j++)
            for (int i = 0; i < N; i++)
                C[i * N + j] += A[i * N + k] * B[k * N + j];
}

static void fill_random(float* p, size_t n) {
    for (size_t i = 0; i < n; ++i)
        p[i] = (float)(rand() % 100) / 100.0f;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Uso: " << argv[0] << " N [BS_min BS_max BS_step]\n";
        std::cerr << "     " << argv[0] << " N compare [BS]\n";
        std::cerr << "     " << argv[0] << " loops [N1 N2 ...]\n";
        return 1;
    }

    /* Modo 3: comparar órdenes de loop lineales (ijk, jik, ikj, kij, jki, kji) para varios N.
     * Working set = 3*N^2*4 bytes. Heurística caché: L1 ~ 48 KiB, L2 ~ 512 KiB, L3 ~ 33 MiB. */
    if (std::string(argv[1]) == "loops") {
        std::vector<int> sizes;
        if (argc > 2) {
            for (int a = 2; a < argc; a++) {
                int n = std::atoi(argv[a]);
                if (n > 0)
                    sizes.push_back(n);
            }
        }
        if (sizes.empty())
            sizes = {256, 260, 512, 550, 1024, 1050};
        const size_t L1_B = 48 * 1024;
        const size_t L2_B = 512 * 1024;
        const size_t L3_B = 33 * 1024 * 1024;
        using loop_fn = void (*)(float*, float*, float*, int);
        const char* names[] = {"ijk", "jik", "ikj", "kij", "jki", "kji"};
        loop_fn funcs[] = {matmul_ijk, matmul_jik, matmul_ikj, matmul_kij, matmul_jki, matmul_kji};
        std::cout << std::fixed << std::setprecision(0);
        /* Cabecera con columnas alineadas (setw) */
        std::cout << std::left
                  << std::setw(5) << "N"
                  << std::setw(10) << "Working set"
                  << std::setw(14) << "Contiene"
                  << std::setw(12) << "Caché"
                  << std::right
                  << std::setw(6) << "ijk" << std::setw(6) << "jik" << std::setw(6) << "ikj"
                  << std::setw(6) << "kij" << std::setw(6) << "jki" << std::setw(6) << "kji"
                  << "\n";
        std::cout << std::left
                  << std::setw(5) << "-"
                  << std::setw(10) << "----------"
                  << std::setw(14) << "------------"
                  << std::setw(12) << "----------"
                  << std::right
                  << std::setw(6) << "---" << std::setw(6) << "---" << std::setw(6) << "---"
                  << std::setw(6) << "---" << std::setw(6) << "---" << std::setw(6) << "---"
                  << "\n";
        for (int N : sizes) {
            size_t total = (size_t)N * N;
            size_t bytes = total * sizeof(float);
            size_t working_set = 3 * bytes;  /* 3 matrices */
            double n3 = (double)N * N * N;
            float* A = new float[total];
            float* B = new float[total];
            float* C = new float[total];
            srand(12345);
            fill_random(A, total);
            fill_random(B, total);
            double mflops[6];
            for (int f = 0; f < 6; f++) {
                std::memset(C, 0, bytes);
                auto start = std::chrono::high_resolution_clock::now();
                funcs[f](A, B, C, N);
                auto end = std::chrono::high_resolution_clock::now();
                double ms = std::chrono::duration<double, std::milli>(end - start).count();
                mflops[f] = n3 / (ms / 1000.0 * 1e6);
            }
            char ws_buf[32];
            if (working_set < 1024)
                snprintf(ws_buf, sizeof(ws_buf), "%zu B", working_set);
            else if (working_set < 1024 * 1024)
                snprintf(ws_buf, sizeof(ws_buf), "%zu KiB", working_set / 1024);
            else
                snprintf(ws_buf, sizeof(ws_buf), "%.0f MiB", working_set / (1024.0 * 1024));
            const char* contiene;
            const char* cache;
            if (working_set <= L1_B) {
                contiene = "<= L1";
                cache = "L1";
            } else if (working_set <= L2_B) {
                contiene = "> L1, <= L2";
                cache = "L1+L2";
            } else if (working_set <= L3_B) {
                contiene = "> L2, <= L3";
                cache = "L1+L2+L3";
            } else {
                contiene = "> L3";
                cache = "Excede cache";
            }
            std::cout << std::left
                      << std::setw(5) << N
                      << std::setw(10) << ws_buf
                      << std::setw(14) << contiene
                      << std::setw(12) << cache
                      << std::right
                      << std::setw(6) << (int)mflops[0] << std::setw(6) << (int)mflops[1]
                      << std::setw(6) << (int)mflops[2] << std::setw(6) << (int)mflops[3]
                      << std::setw(6) << (int)mflops[4] << std::setw(6) << (int)mflops[5]
                      << "\n";
            delete[] A;
            delete[] B;
            delete[] C;
        }
        return 0;
    }

    int N = std::atoi(argv[1]);
    if (N <= 0) {
        std::cerr << "N debe ser positivo.\n";
        return 1;
    }

    size_t total = (size_t)N * N;
    size_t bytes = total * sizeof(float);
    double n3 = (double)N * N * N;

    float* A = new float[total];
    float* B = new float[total];
    float* C = new float[total];

    srand(12345);
    fill_random(A, total);
    fill_random(B, total);

    bool compare_mode = (argc >= 3 && std::string(argv[2]) == "compare");
    int BS_OPT = 45;  /* valor por defecto: ./matmul_blocks N compare [BS]; 60 = óptimo típico para N=1800 */
    if (argc >= 4 && compare_mode)
        BS_OPT = std::atoi(argv[3]);

    if (compare_mode) {
        std::cout << std::fixed << std::setprecision(2);
        std::cout << "N = " << N << " (3 matrices ~ " << (3 * bytes / (1024 * 1024)) << " MB)";
        if (N % BS_OPT == 0)
            std::cout << "  [N múltiplo de " << BS_OPT << "]";
        std::cout << "\n\n";

        if (N % BS_OPT == 0) {
            std::memset(C, 0, bytes);
            auto start = std::chrono::high_resolution_clock::now();
            matrix_mult_blocks(A, B, C, N, BS_OPT);
            auto end = std::chrono::high_resolution_clock::now();
            double ms_b = std::chrono::duration<double, std::milli>(end - start).count();
            double mflops_b = n3 / (ms_b / 1000.0 * 1e6);
            std::cout << "Bloqueado (ii,jj,kk)\ttiempo_ms=" << ms_b << "\tMFLOPS=" << mflops_b << "\n";

            std::memset(C, 0, bytes);
            start = std::chrono::high_resolution_clock::now();
            matrix_mult_blocks_ikkjj(A, B, C, N, BS_OPT);
            end = std::chrono::high_resolution_clock::now();
            double ms_ikkjj = std::chrono::duration<double, std::milli>(end - start).count();
            double mflops_ikkjj = n3 / (ms_ikkjj / 1000.0 * 1e6);
            std::cout << "Bloqueado (ii,kk,jj)\ttiempo_ms=" << ms_ikkjj << "\tMFLOPS=" << mflops_ikkjj << "\n";
        }

        std::memset(C, 0, bytes);
        auto start = std::chrono::high_resolution_clock::now();
        matmul_ikj(A, B, C, N);
        auto end = std::chrono::high_resolution_clock::now();
        double ms_ikj = std::chrono::duration<double, std::milli>(end - start).count();
        double mflops_ikj = n3 / (ms_ikj / 1000.0 * 1e6);
        std::cout << "Lineal ikj\t\t\ttiempo_ms=" << ms_ikj << "\tMFLOPS=" << mflops_ikj << "\n";

        delete[] A;
        delete[] B;
        delete[] C;
        return 0;
    }

    std::cout << "N = " << N << " (3 matrices ~ " << (3 * bytes / (1024 * 1024)) << " MB)\n";

    int bs_min = 8, bs_max = std::min(512, N), bs_step = 8;
    if (argc >= 5) {
        bs_min = std::atoi(argv[2]);
        bs_max = std::atoi(argv[3]);
        bs_step = std::atoi(argv[4]);
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
        std::memset(C, 0, bytes);
        auto start = std::chrono::high_resolution_clock::now();
        matrix_mult_blocks(A, B, C, N, BS);
        auto end = std::chrono::high_resolution_clock::now();
        double ms = std::chrono::duration<double, std::milli>(end - start).count();
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
