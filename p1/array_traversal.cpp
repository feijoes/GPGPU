/*
 * Ejercicio 1 - Localidad espacial (letra: arreglo 100 MB, recorrida secuencial y
 * aleatoria; en ambos casos el siguiente índice a visitar se lee de un arreglo
 * inicializado previamente).
 */
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>
#include <cstddef>
#include <cstdint>

int main() {
    const size_t ARRAY_SIZE = 100 * 1024 * 1024;  // 100 MB

    char* char_array = new char[ARRAY_SIZE];
    for (size_t i = 0; i < ARRAY_SIZE; ++i)
        char_array[i] = 0;

    // Arreglo de índices: en la recorrida secuencial contiene 0, 1, 2, ...
    std::vector<uint32_t> index_array(ARRAY_SIZE);
    for (size_t i = 0; i < ARRAY_SIZE; ++i)
        index_array[i] = static_cast<uint32_t>(i);

    // Recorrida secuencial: siguiente índice leído del arreglo (orden 0, 1, 2, ...)
    auto start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        uint32_t next_index = index_array[i];
        char_array[next_index] += 1;
    }
    auto end = std::chrono::high_resolution_clock::now();
    double time_sequential_ms = std::chrono::duration<double, std::milli>(end - start).count();

    // Arreglo de índices en orden aleatorio (permutación)
    std::vector<uint32_t> random_index_array(ARRAY_SIZE);
    for (size_t i = 0; i < ARRAY_SIZE; ++i)
        random_index_array[i] = static_cast<uint32_t>(i);
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(random_index_array.begin(), random_index_array.end(), gen);

    for (size_t i = 0; i < ARRAY_SIZE; ++i)
        char_array[i] = 0;

    // Recorrida aleatoria: siguiente índice leído del arreglo (misma cantidad de elementos)
    start = std::chrono::high_resolution_clock::now();
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        uint32_t next_index = random_index_array[i];
        char_array[next_index] += 1;
    }
    end = std::chrono::high_resolution_clock::now();
    double time_random_ms = std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "Elementos: " << ARRAY_SIZE << "\n";
    std::cout << "Secuencial (ms): " << time_sequential_ms << "\n";
    std::cout << "Aleatorio (ms):  " << time_random_ms << "\n";

    delete[] char_array;
    return 0;
}
