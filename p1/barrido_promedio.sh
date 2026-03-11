#!/bin/bash
# Barrido de BS ejecutado varias veces; imprime promedios de tiempo y MFLOPS (desde tiempo promedio).
# Uso: ./barrido_promedio.sh N BS_min BS_max step [num_corridas]
# Ejemplo: ./barrido_promedio.sh 1800 10 100 5 5

N=${1:-1800}
BS_MIN=${2:-10}
BS_MAX=${3:-100}
STEP=${4:-5}
RUNS=${5:-5}
BIN="$(dirname "$0")/matmul_blocks"

if [ ! -x "$BIN" ]; then
  echo "No se encuentra ejecutable: $BIN"
  exit 1
fi

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

echo "N=$N, BS $BS_MIN..$BS_MAX step=$STEP, promediando $RUNS corridas (taskset -c 0)" >&2
echo "Cada corrida hace un barrido completo; puede tardar varios minutos." >&2
for r in $(seq 1 "$RUNS"); do
  echo "[$(date +%H:%M:%S)] Corrida $r/$RUNS iniciando..." >&2
  taskset -c 0 "$BIN" "$N" "$BS_MIN" "$BS_MAX" "$STEP" 2>/dev/null | awk '/^[0-9]+\t/{print $1"\t"$2"\t"$3}' > "$TMPDIR/run_$r.txt"
  echo "[$(date +%H:%M:%S)] Corrida $r/$RUNS terminada." >&2
done
echo "[$(date +%H:%M:%S)] Calculando promedios..." >&2

N3=$(echo "$N^3" | bc)
awk -v n3="$N3" -v runs="$RUNS" -v tmp="$TMPDIR" '
BEGIN {
  for (r = 1; r <= runs; r++) {
    f = tmp "/run_" r ".txt"
    while ((getline line < f) > 0) {
      split(line, a, "\t")
      bs = a[1]+0
      t  = a[2]+0
      sumt[bs] += t
      cnt[bs]++
    }
    close(f)
  }
}
END {
  for (key in sumt) {
    if (cnt[key] == 0) continue
    avg_ms = sumt[key] / cnt[key]
    mflops = n3 / (avg_ms / 1000.0 * 1e6)
    printf "%d\t%.2f\t%.2f\n", key, avg_ms, mflops
  }
}
' </dev/null > "$TMPDIR/avg.txt"

sort -n "$TMPDIR/avg.txt" > "$TMPDIR/sorted.txt"
best_line=$(sort -t$'\t' -k3 -nr "$TMPDIR/avg.txt" | head -1)
best_bs=$(echo "$best_line" | cut -f1)
best_mflops=$(echo "$best_line" | cut -f3)

echo "[$(date +%H:%M:%S)] Resultados:" >&2
echo ""
echo "BS	tiempo_promedio_ms	MFLOPS"
cat "$TMPDIR/sorted.txt"
echo ""
echo "Mejor BS (promedio de $RUNS corridas) = $best_bs ($best_mflops MFLOPS)"
