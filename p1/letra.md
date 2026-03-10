# Programación Masivamente Paralela en Procesadores Gráficos

# Práctico 1 - Patrones de acceso a memoria

El objetivo de este práctico es reflexionar sobre la jerarquía de memoria, en especial la memoria principal y los distintos niveles de caché, y sobre cómo distintos patrones de acceso a los datos hacen un uso distinto de dicha jerarquía.

En los siguientes ejercicios se accederá a una estructura de datos realizando la misma cantidad de operaciones, aunque en distinto orden.

Para que los tiempos de ejecución sean lo más estables posible se recomienda ejecutar en un sistema con poca carga, medir tiempos lo suficientemente grandes (adaptando el tamaño de la entrada o la cantidad de repeticiones de las pruebas), y evitar la utilización de máquinas virtuales.

---

## Ejercicio 1 - Localidad espacial

1. Escriba un programa en C/C++ que reserve e inicialice un arreglo de `char` de gran tamaño (por ejemplo `100MB`).

   A continuación el programa debe recorrer el arreglo (por ejemplo incrementando el valor de cada posición) de manera secuencial (primero la posición `0`, luego la `1`, y así sucesivamente).

   Durante la recorrida, el siguiente índice a visitar debe leerse de un arreglo inicializado previamente.

   Registre el tiempo de ejecución de la recorrida.

2. Realice otra recorrida por el arreglo que visite la misma cantidad de elementos pero realizando saltos aleatorios.

   Durante la recorrida, el siguiente índice a visitar debe leerse de un arreglo inicializado previamente.

   Mida el tiempo de ejecución y reflexione sobre los resultados.

---

## Ejercicio 2

Una estrategia muy utilizada en bibliotecas de alto desempeño para mejorar el uso de la caché en operaciones como la multiplicación de matrices es trabajar por bloques pequeños, de tamaño `BS`, ajustado de acuerdo a las características del dispositivo.

Un ejemplo puede ser el siguiente:

```cpp
void matrix_mult(float *A, float *B, float *C, int N) {
    int i, j, k, ii, jj, kk;
    for (ii = 0; ii < N; ii += BS)
        for (jj = 0; jj < N; jj += BS)
            for (kk = 0; kk < N; kk += BS)
                for (i = ii; i < ii + BS; i++)
                    for (j = jj; j < jj + BS; j++)
                        for (k = kk; k < kk + BS; k++)
                            C[i * N + j] += A[i * N + k] * B[k * N + j];
}
```

1. Para tres matrices de tamaño más grande que la capacidad del caché de último nivel (LLC), determine experimentalmente el mejor valor de `BS`.

   Relacione el desempeño obtenido para los distintos valores de `BS` con los tamaños de la caché `L1` y `L2`.

   ¿Los valores son los que esperaría teóricamente?

   En caso de encontrar diferencias con la teoría, ¿qué las explica?

2. Compare el rendimiento de la versión a bloques con el tamaño de `BS` óptimo obtenido en la parte anterior con la variante lineal `i, k, j` del producto de matrices.

```cpp
for (i = 0; i < N; i++)
    for (k = 0; k < N; k++)
        for (j = 0; j < N; j++)
            C[i * N + j] += A[i * N + k] * B[k * N + j];
```

Reflexione acerca de los resultados.

¿Qué problemas enfrenta la variante a bloques?

¿Podría modificarla de forma sencilla para mejorar su desempeño?

3. Compare el tiempo de ejecución y rendimiento en MFLOPS (`N^3 / (segundos × 10^6)`) de la versión lineal para distintos órdenes de loop:

- `ijk`
- `jik`
- `ikj`
- `kij`
- `jki`
- `kji`

Al menos para los tamaños de matriz:

- `256`
- `260`
- `512`
- `550`
- `1024`
- `1050`

Puede extender el experimento incluyendo otros tamaños.

Reflexione sobre los resultados obtenidos.

---

## Entrega

- Se debe entregar un informe en PDF con la solución de los ejercicios que contenga, como máximo, **5 páginas**.
- No se entrega el código fuente.
- Las secciones relevantes del código que haya usado deben figurar en el informe.
