#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash_table.h"

#define MAX_LINE 1024

// Função para extrair URL da linha de log
char* extract_url(const char* line) {
    const char* get_start = strstr(line, "\"GET ");
    if (!get_start) return NULL;
    // Avança exatamente 5 caracteres: '"' 'G' 'E' 'T' ' ' para apontar para o '/'
    get_start += 5; 
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
    if (argc != 3) {
        fprintf(stderr, "Uso: %s <manifest.txt> <access_log.txt>\n", argv[0]);
        return 1;
    }

    const char* manifest_file = argv[1];
    const char* log_file = argv[2];

    // Fase 1: Construir tabela hash a partir do manifesto
    FILE* mf = fopen(manifest_file, "r");
    if (!mf) {
        perror("Erro ao abrir manifesto");
        return 1;
    }
    HashTable* ht = ht_create(100000); // Tamanho aproximado para 100k URLs
    char line[MAX_LINE];
    while (fgets(line, sizeof(line), mf)) {
        line[strcspn(line, "\r\n")] = 0; // Remove newline and carriage return
        ht_put(ht, line);
    }
    fclose(mf);

    // Fase 2: Processar log sequencialmente
    FILE* lf = fopen(log_file, "r");
    if (!lf) {
        perror("Erro ao abrir log");
        ht_destroy(ht);
        return 1;
    }
    long total_linhas = 0, extraidas = 0, encontradas = 0;

    while (fgets(line, sizeof(line), lf)) {
        total_linhas++;
        char* url = extract_url(line);
        if (url) {
            extraidas++;
            CacheNode* node = ht_get(ht, url);
            if (node) {
                encontradas++;
                node->hit_count++;
            }
            free(url);
        }
    }

    fprintf(stderr,
            "Resumo debug: linhas=%ld url_extraidas=%ld url_encontradas=%ld\n",
            total_linhas, extraidas, encontradas);
    fclose(lf);

    // Fase 3: Salvar resultados
    ht_save_results(ht, "results.csv");
    ht_destroy(ht);
    return 0;
}