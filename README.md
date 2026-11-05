# Competição de Ordenação em C

Programa em C que lê uma lista desordenada de inteiros (`.txt`, um por linha), ordena em ordem
crescente e imprime **somente o tempo da ordenação** (medido com `clock()`, sem contar a leitura do disco).

## Equipe (individual)

1. Maria Eduarda — Líder que enviou

## Compilação

Linux / macOS / WSL / MinGW (GCC):

```bash
gcc -O3 -march=native -o ordenacao ordenacao.c
```

Ou com o Makefile: `make`

## Execução

```bash
./ordenacao listaOdenada.txt                  # só ordena e mede
./ordenacao listaOdenada.txt saida.txt        # também grava o resultado ordenado
```

No Windows (MinGW): `gcc -O3 -o ordenacao.exe ordenacao.c` e `ordenacao.exe listaOdenada.txt`.

## Saída esperada (exemplo)

```
Elementos lidos: 10000000
Tempo de ordenação: 0.040797 segundos (40.797000 ms)
Verificação de ordem crescente: OK
Verificação de integridade (sem perda de elementos): OK
Elementos repetidos adjacentes: 7500000 (ordem não-decrescente; valores iguais ficam juntos)
```

## Estratégia algorítmica

| Caso | Algoritmo | Complexidade |
|------|-----------|--------------|
| Faixa de valores pequena (`max - min < 4N`) — caso do arquivo da competição (1 a 2.500.000 em 10.000.000 de elementos) | **Counting Sort** | O(N + K) |
| Qualquer outra faixa de inteiros (inclui negativos) | **Radix Sort LSD** base 256 | O(N · k), k ≤ 8 |

Ambos são lineares, portanto mais rápidos que Quick Sort / Merge Sort (O(N log N)) para inteiros.
Nenhum elemento é perdido: os valores repetidos são mantidos.

## Isolamento da medição de tempo

O cronômetro é disparado imediatamente antes de `ordenarVetor()` e parado logo depois:

```c
clock_t inicio = clock();
ordenarVetor(v, n);
clock_t fim = clock();
```

Leitura do arquivo, verificação de corretude e gravação da saída ficam **fora** da medição.

## Verificação de corretude

Feita após a medição:

- percorre o vetor conferindo `v[i-1] <= v[i]` (ordem crescente);
- compara quantidade, soma e XOR dos elementos antes e depois (garante que nada foi perdido ou alterado);
- reporta quantos valores repetidos existem (o arquivo tem cada valor 4 vezes, então a ordem é
  crescente com valores iguais agrupados — estrita é impossível sem descartar elementos).

## Gerenciamento de memória

Toda alocação (`malloc`/`calloc`) é verificada e liberada com `free` no fim da execução.

## Estrutura

```
ordenacao.c   código-fonte comentado
Makefile      compilação
README.md     este arquivo
print_execucao.png / saida_terminal.txt   evidência da execução
```
