#include <iostream>
#include <iomanip>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstdlib>

int main() {
    const size_t ARRAY_SIZE = 100 * 1024 * 1024;  // 100 MB
    
    std::cout << "Reservando arreglo de char de " << (ARRAY_SIZE / (1024 * 1024)) << " MB..." << std::endl;
    
    // Reservar e inicializar el arreglo de char
    char* char_array = new char[ARRAY_SIZE];
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        char_array[i] = 0;
    }
    std::cout << "Arreglo de char inicializado." << std::endl;
    
    // Reservar e inicializar el arreglo de índices (orden secuencial: 0, 1, 2, ...)
    // Usamos uint32_t ya que 100M < 2^32, ahorrando memoria
    std::vector<uint32_t> index_array(ARRAY_SIZE);
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        index_array[i] = static_cast<uint32_t>(i);
    }
    std::cout << "Arreglo de índices inicializado (orden secuencial)." << std::endl;
    
    // Recorrer el arreglo: el siguiente índice se lee del arreglo de índices
    std::cout << "Iniciando recorrida secuencial..." << std::endl;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        uint32_t next_index = index_array[i];
        char_array[next_index] += 1;  // Incrementar el valor en la posición indicada
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Recorrida secuencial completada." << std::endl;
    double time_sequential_ms = duration.count();
    std::cout << "Tiempo de ejecución (secuencial): " << time_sequential_ms << " ms" << std::endl;
    
    // === RECORRIDA ALEATORIA ===
    // Crear arreglo de índices en orden aleatorio (permutación usando Fisher-Yates)
    std::cout << "\nInicializando arreglo de índices con saltos aleatorios..." << std::endl;
    std::vector<uint32_t> random_index_array(ARRAY_SIZE);
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        random_index_array[i] = static_cast<uint32_t>(i);
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(random_index_array.begin(), random_index_array.end(), gen);
    std::cout << "Arreglo de índices aleatorios listo." << std::endl;
    
    // Reinicializar el arreglo de char para la segunda recorrida
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        char_array[i] = 0;
    }
    
    std::cout << "Iniciando recorrida con saltos aleatorios..." << std::endl;
    start = std::chrono::high_resolution_clock::now();
    
    for (size_t i = 0; i < ARRAY_SIZE; ++i) {
        uint32_t next_index = random_index_array[i];
        char_array[next_index] += 1;
    }
    
    end = std::chrono::high_resolution_clock::now();
    duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Recorrida aleatoria completada." << std::endl;
    double time_random_ms = duration.count();
    std::cout << "Tiempo de ejecución (aleatorio): " << time_random_ms << " ms" << std::endl;
    
    // === REFLEXIÓN SOBRE LOS RESULTADOS ===
    std::cout << "\n========== REFLEXIÓN ==========" << std::endl;
    std::cout << "Elementos visitados: " << ARRAY_SIZE << " (en ambas recorridas)" << std::endl;
    std::cout << "Secuencial: " << time_sequential_ms << " ms" << std::endl;
    std::cout << "Aleatorio:  " << time_random_ms << " ms" << std::endl;
    if (time_sequential_ms > 0) {
        double ratio = time_random_ms / time_sequential_ms;
        std::cout << "La recorrida aleatoria es ~" << std::fixed << ratio 
                  << "x más lenta que la secuencial." << std::endl;
    }
    std::cout << "\nExplicación: El acceso secuencial aprovecha la localidad espacial."
                 "\nLa CPU pre carga bloques contiguos en la caché (cache line). Con índices"
                 "\naleatorios, cada acceso puede fallar en caché (cache miss), forzando"
                 "\nlecturas desde RAM, que es ~100x más lenta que la caché L1." << std::endl;
    std::cout << "================================\n" << std::endl;
    
    // Verificación: todas las posiciones deberían tener valor 1
    size_t check_count = 0;
    for (size_t i = 0; i < ARRAY_SIZE && check_count < 5; ++i) {
        if (char_array[i] != 1) {
            std::cout << "Verificación: char_array[" << i << "] = " << (int)char_array[i] << std::endl;
            check_count++;
        }
    }
    if (check_count == 0) {
        std::cout << "Verificación: todas las posiciones contienen el valor esperado (1)." << std::endl;
    }
    
    delete[] char_array;
    
    return 0;
}
