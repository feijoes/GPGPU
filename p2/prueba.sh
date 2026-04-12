#!/bin/bash
#SBATCH --job-name=prueba
#SBATCH --output=prueba-%j.out
#SBATCH --error=prueba-%j.err
#SBATCH --nodes=1
#SBATCH --ntasks=1
#SBATCH --cpus-per-task=4
#SBATCH --gres=gpu:1
# Ajustar partición y tiempo según el clúster:
#SBATCH --partition=gpu
#SBATCH --time=01:00:00
# Descomentar si el clúster lo requiere:
##SBATCH --account=tu_cuenta

set -euo pipefail

cd "${SLURM_SUBMIT_DIR}"

echo "=== Slurm job ${SLURM_JOB_ID:-local} ==="
echo "Nodo: ${SLURMD_NODENAME:-$(hostname)}"
echo "Fecha: $(date -Is)"
echo

# Cargar CUDA (nombre del módulo según `module avail` en tu clúster)
if command -v module >/dev/null 2>&1; then
  module load cuda 2>/dev/null || true
fi

if command -v nvidia-smi >/dev/null 2>&1; then
  nvidia-smi
  echo
else
  echo "Aviso: nvidia-smi no encontrado en PATH."
fi

if command -v nvcc >/dev/null 2>&1; then
  nvcc --version
  echo
else
  echo "Aviso: nvcc no encontrado. Carga el módulo de CUDA antes de compilar."
fi

# Compilar y ejecutar de ejemplo (editar según lo que quieras probar)
for src in ej1.cu ej2.cu ej3.cu; do
  if [[ -f "$src" ]]; then
    base="${src%.cu}"
    echo "=== Compilando $src ==="
    nvcc -O2 -o "$base" "$src"
    echo "=== Ejecutando ./$base ==="
    "./$base"
    echo
  fi
done

echo "=== Fin $(date -Is) ==="
