#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <omp.h>
#include "hash_table.h"

#define MAX_LINE 1024

// Função para extrair URL (igual)
char* extract_url(const char* line) {
    const char* get_start = strstr(line, "\"GET ");
    if (!get_start) return NULL;
    get_start += 5; // corrigido para manter '/'
    const char* http_end = strstr(get_start, " HTTP/");
    if (!http_end) return NULL;
    size_t url_len = http_end - get_start;
    if (url_len == 0) return NULL;
    char* url = (char*)malloc(url_len + 1);
    if (!url) return NULL;
    memcpy(url, get_start, url_len);
    url[url_len] = '\0';
    return url;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        fprintf(stderr, "Uso: %s <manifest.txt> <access_log.txt> <num_threads>\n", argv[0]);
        return 1;
    }

    const char* manifest_file = argv[1];
    const char* log_file = argv[2];
    int num_threads = atoi(argv[3]);
    omp_set_num_threads(num_threads);

    // Fase 1: Construir tabela hash (sequencial)
    FILE* mf = fopen(manifest_file, "r");
    if (!mf) {
        perror("Erro ao abrir manifesto");
        return 1;
    }
    HashTable* ht = ht_create(100000);
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), mf)) {
        line[strcspn(line, "\r\n")] = 0; // Remove newline and carriage return
        ht_put(ht, line);
    }
    fclose(mf);

    // Fase 2: Pré-carregar linhas do log (leitura sequencial segura)
    FILE* lf = fopen(log_file, "r");
    if (!lf) {
        perror("Erro ao abrir log");
        ht_destroy(ht);
        return 1;
    }
    char** lines = NULL;
    size_t lines_cap = 0, lines_count = 0;
    while (fgets(line, sizeof(line), lf)) {
        if (lines_count == lines_cap) {
            size_t new_cap = lines_cap ? lines_cap * 2 : 100000;
            char** tmp = (char**)realloc(lines, new_cap * sizeof(char*));
            if (!tmp) {
                perror("Erro ao realocar vetor de linhas");
                fclose(lf);
                ht_destroy(ht);
                return 1;
            }
            lines = tmp;
            lines_cap = new_cap;
        }
        size_t len = strlen(line);
        char* stored = (char*)malloc(len + 1);
        if (!stored) {
            perror("Erro ao alocar linha");
            fclose(lf);
            ht_destroy(ht);
            return 1;
        }
        strcpy(stored, line);
        lines[lines_count++] = stored;
    }
    fclose(lf);

    // Fase 2b: Processar em paralelo com incremento atômico
    #pragma omp parallel for schedule(static)
    for (long i = 0; i < (long)lines_count; i++) {
        char* url = extract_url(lines[i]);
        if (url) {
            CacheNode* node = ht_get(ht, url);
            if (node) {
                #pragma omp atomic update
                node->hit_count++;
            }
            free(url);
        }
    }
    for (size_t i = 0; i < lines_count; i++) free(lines[i]);
    free(lines);

    // Fase 3: Salvar resultados
    ht_save_results(ht, "results.csv");
    ht_destroy(ht);
    return 0;
}