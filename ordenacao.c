/*
 * Competição de Ordenação - C
 * ------------------------------------------------------------------
 * Lê um arquivo de texto com números inteiros (separados por espaço,
 * tabulação ou quebra de linha), ordena em ordem crescente e imprime
 * APENAS o tempo da ordenação (clock(), sem contar leitura de disco).
 *
 * Estratégia (híbrida, ambas lineares e mais rápidas que Quick/Merge O(N log N)):
 *   1) Counting Sort - usado quando a faixa de valores (max - min + 1) é pequena
 *      em relação a N (caso do arquivo da competição: valores de 1 a 2.500.000
 *      para 10.000.000 de elementos). Complexidade O(N + K), K = faixa.
 *   2) Radix Sort LSD (base 256) sobre chaves de 64 bits - fallback geral,
 *      O(N * k) com k <= 8 passadas. Funciona com negativos (inverte-se o bit
 *      de sinal) e pula passadas em que todos caem no mesmo balde.
 *   - Memória: Counting usa K contadores; Radix usa 1 vetor auxiliar de N.
 *     Tudo é liberado no fim.
 *
 * Verificações (fora da medição de tempo):
 *   - Ordem crescente (a[i] <= a[i+1] para todo i) e se há repetidos
 *   - Nenhum elemento perdido: mesma quantidade, soma e XOR antes/depois
 *
 * Uso: ./ordenacao ListahAhSerOrdenada.txt [saida_ordenada.txt]
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

/* ---------- leitura do arquivo (NÃO entra no tempo medido) ---------- */
static int64_t *ler_arquivo(const char *nome, size_t *n_out)
{
    FILE *f = fopen(nome, "rb");
    if (!f) { perror("Erro ao abrir o arquivo"); return NULL; }

    fseek(f, 0, SEEK_END);
    long tam = ftell(f);
    fseek(f, 0, SEEK_SET);

    char *buf = malloc((size_t)tam + 1);
    if (!buf) { fprintf(stderr, "Sem memoria para o buffer\n"); fclose(f); return NULL; }
    size_t lidos = fread(buf, 1, (size_t)tam, f);
    fclose(f);
    buf[lidos] = '\0';

    size_t cap = 1 << 20, n = 0;
    int64_t *v = malloc(cap * sizeof(int64_t));
    if (!v) { free(buf); return NULL; }

    char *p = buf, *fim = buf + lidos;
    while (p < fim) {
        /* pula separadores */
        while (p < fim && *p != '-' && *p != '+' && (*p < '0' || *p > '9')) p++;
        if (p >= fim) break;

        int neg = 0;
        if (*p == '-') { neg = 1; p++; }
        else if (*p == '+') { p++; }
        if (p >= fim || *p < '0' || *p > '9') continue;

        uint64_t x = 0;
        while (p < fim && *p >= '0' && *p <= '9') { x = x * 10 + (uint64_t)(*p - '0'); p++; }
        if (p < fim && (*p == '.' || *p == ',' || *p == 'e' || *p == 'E')) {
            fprintf(stderr, "Aviso: valor nao inteiro encontrado (elemento %zu). "
                            "Este programa espera inteiros.\n", n + 1);
            free(buf); free(v); return NULL;
        }

        if (n == cap) {
            cap *= 2;
            int64_t *t = realloc(v, cap * sizeof(int64_t));
            if (!t) { free(buf); free(v); return NULL; }
            v = t;
        }
        v[n++] = neg ? -(int64_t)x : (int64_t)x;
    }
    free(buf);
    *n_out = n;
    return v;
}

/* ---------- ordenação: Radix Sort LSD (base 256) ---------- */
static void radixSort(int64_t *a, size_t n);

/* Counting Sort: indicado quando max - min + 1 é pequeno em relação a N */
static int countingSort(int64_t *a, size_t n, int64_t mn, int64_t mx)
{
    size_t faixa = (size_t)(mx - mn) + 1;
    uint32_t *cont = calloc(faixa, sizeof(uint32_t));
    if (!cont) return 0;                       /* sem memória -> usa radix */

    for (size_t i = 0; i < n; i++) cont[a[i] - mn]++;

    size_t pos = 0;
    for (size_t k = 0; k < faixa; k++) {
        int64_t valor = mn + (int64_t)k;
        for (uint32_t c = cont[k]; c > 0; c--) a[pos++] = valor;
    }
    free(cont);
    return 1;
}

static void ordenarVetor(int64_t *a, size_t n)
{
    if (n < 2) return;

    int64_t mn = a[0], mx = a[0];
    for (size_t i = 1; i < n; i++) {
        if (a[i] < mn) mn = a[i];
        if (a[i] > mx) mx = a[i];
    }
    /* faixa pequena (<= 4N e sem overflow) -> Counting Sort */
    if ((uint64_t)(mx - mn) < 4ULL * n && countingSort(a, n, mn, mx)) return;

    radixSort(a, n);
}

/* Radix Sort LSD base 256 (fallback para qualquer faixa de inteiros) */
static void radixSort(int64_t *a, size_t n)
{
    uint64_t *src = (uint64_t *)a;
    uint64_t *dst = malloc(n * sizeof(uint64_t));
    if (!dst) { fprintf(stderr, "Sem memoria para o vetor auxiliar\n"); exit(1); }

    const uint64_t SINAL = 0x8000000000000000ULL;

    /* converte para chave sem sinal (inteiros negativos ficam antes) */
    for (size_t i = 0; i < n; i++) src[i] ^= SINAL;

    /* histogramas de todos os 8 bytes em uma única varredura */
    static size_t cont[8][256];
    memset(cont, 0, sizeof(cont));
    for (size_t i = 0; i < n; i++) {
        uint64_t x = src[i];
        for (int b = 0; b < 8; b++) cont[b][(x >> (8 * b)) & 0xFF]++;
    }

    for (int b = 0; b < 8; b++) {
        /* pula a passada se todos os elementos têm o mesmo byte */
        if (cont[b][(src[0] >> (8 * b)) & 0xFF] == n) continue;

        size_t pos[256], soma = 0;
        for (int k = 0; k < 256; k++) { pos[k] = soma; soma += cont[b][k]; }

        for (size_t i = 0; i < n; i++) {
            uint64_t x = src[i];
            dst[pos[(x >> (8 * b)) & 0xFF]++] = x;
        }
        uint64_t *t = src; src = dst; dst = t;   /* troca os papéis */
    }

    /* se o resultado final ficou no auxiliar, copia de volta */
    if (src != (uint64_t *)a) memcpy(a, src, n * sizeof(uint64_t));

    for (size_t i = 0; i < n; i++) ((uint64_t *)a)[i] ^= SINAL;  /* restaura sinal */

    /* libera o buffer auxiliar (o que não é 'a') */
    free(src == (uint64_t *)a ? dst : src);
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "Uso: %s <arquivo.txt> [saida_ordenada.txt]\n", argv[0]);
        return 1;
    }

    size_t n = 0;
    int64_t *v = ler_arquivo(argv[1], &n);
    if (!v || n == 0) { fprintf(stderr, "Nenhum dado lido.\n"); return 1; }
    printf("Elementos lidos: %zu\n", n);

    /* assinatura para conferir perda/alteração de elementos */
    uint64_t soma_antes = 0, xor_antes = 0;
    for (size_t i = 0; i < n; i++) { soma_antes += (uint64_t)v[i]; xor_antes ^= (uint64_t)v[i]; }

    /* ===== medição: SOMENTE a ordenação ===== */
    clock_t inicio = clock();
    ordenarVetor(v, n);
    clock_t fim = clock();
    double tempo_gasto = ((double)(fim - inicio)) / CLOCKS_PER_SEC;
    /* ======================================== */

    printf("Tempo de ordenação: %f segundos (%f ms)\n", tempo_gasto, tempo_gasto * 1000.0);

    /* verificação de corretude (fora do tempo) */
    size_t inversoes = 0, repetidos = 0;
    uint64_t soma_depois = 0, xor_depois = 0;
    for (size_t i = 0; i < n; i++) {
        soma_depois += (uint64_t)v[i]; xor_depois ^= (uint64_t)v[i];
        if (i > 0) {
            if (v[i - 1] > v[i]) inversoes++;
            else if (v[i - 1] == v[i]) repetidos++;
        }
    }
    int integro = (soma_antes == soma_depois && xor_antes == xor_depois);
    printf("Verificação de ordem crescente: %s\n", inversoes == 0 ? "OK" : "FALHOU");
    printf("Verificação de integridade (sem perda de elementos): %s\n", integro ? "OK" : "FALHOU");
    printf("Elementos repetidos adjacentes: %zu%s\n", repetidos,
           repetidos ? " (ordem não-decrescente; valores iguais ficam juntos)" : " (ordem estritamente crescente)");

    if (argc >= 3) {
        FILE *o = fopen(argv[2], "w");
        if (o) { for (size_t i = 0; i < n; i++) fprintf(o, "%lld\n", (long long)v[i]); fclose(o); }
    }

    free(v);
    return (inversoes == 0 && integro) ? 0 : 2;
}
