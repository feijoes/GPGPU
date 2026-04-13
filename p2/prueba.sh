#!/bin/bash
#SBATCH --job-name=mitrabajo
#SBATCH --ntasks=1
#SBATCH --mem=16G
#SBATCH --time=00:05:00

#SBATCH --gres=gpu:1
# para ejecutar en la gtx1060 ---> #SBATCH --gres=gpu:n1060:1
# para ejecutar en la rtx2080ti ---> #SBATCH --gres=gpu:n2080ti:1

#SBATCH --partition=cursos
#SBATCH --qos=gpgpu

PATH=$PATH:/usr/local/cuda/bin
LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/local/cuda/lib64

echo "--- Compilando ---"
nvcc ej1.cu -o ej1
nvcc ej2.cu -o ej2
nvcc ej3.cu -o ej3

echo ""
echo "--- Ejercicio 1 ---"
echo ""
./ej1 secreto.txt

echo ""
echo "--- Ejercicio 2 ---"
echo ""
# N=10, val=5, i1=512, j1=768, i2=3583, j2=3327
./ej2 4096 5 512 768 3583 3327

echo ""
echo "--- Ejercicio 3 ---"
echo ""
./ej3 