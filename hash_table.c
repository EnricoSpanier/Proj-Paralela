#include "hash_table.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Função de Hash (djb2) */
static size_t hash_djb2(const char* str, size_t size) {
    unsigned long hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }
    return hash % size;
}

/* Cria um novo nó */
static CacheNode* create_node(const char* url) {
    CacheNode* node = (CacheNode*)malloc(sizeof(CacheNode));
    if (!node) {
        perror("Erro ao alocar CacheNode");
        exit(EXIT_FAILURE);
    }
    node->url = (char*)malloc(strlen(url) + 1);
    if (!node->url) {
        perror("Erro ao alocar string da URL");
        free(node);
        exit(EXIT_FAILURE);
    }
    strcpy(node->url, url);
    node->hit_count = 0;
    node->next = NULL;
    return node;
}

HashTable* ht_create(size_t size) {
    if (size < 1) {
        fprintf(stderr, "Tamanho da tabela deve ser ao menos 1\n");
        return NULL;
    }
    HashTable* ht = (HashTable*)malloc(sizeof(HashTable));
    if (!ht) {
        perror("Erro ao alocar HashTable");
        return NULL;
    }
    ht->table = (CacheNode**)calloc(size, sizeof(CacheNode*));
    if (!ht->table) {
        perror("Erro ao alocar buckets da tabela");
        free(ht);
        return NULL;
    }
    ht->size = size;
    return ht;
}

void ht_destroy(HashTable* ht) {
    if (!ht) return;
    for (size_t i = 0; i < ht->size; i++) {
        CacheNode* current = ht->table[i];
        while (current) {
            CacheNode* next = current->next;
            free(current->url);
            free(current);
            current = next;
        }
    }
    free(ht->table);
    free(ht);
}

void ht_put(HashTable* ht, const char* url) {
    if (!ht || !url) return;
    size_t index = hash_djb2(url, ht->size);
    CacheNode* current = ht->table[index];
    while (current) {
        if (strcmp(current->url, url) == 0) {
            return; /* já existe */
        }
        current = current->next;
    }
    CacheNode* new_node = create_node(url);
    new_node->next = ht->table[index];
    ht->table[index] = new_node;
}

CacheNode* ht_get(HashTable* ht, const char* url) {
    if (!ht || !url) return NULL;
    size_t index = hash_djb2(url, ht->size);
    CacheNode* current = ht->table[index];
    while (current) {
        if (strcmp(current->url, url) == 0) {
            return current;
        }
        current = current->next;
    }
    return NULL;
}

static int cmp_nodes_by_url(const void* a, const void* b) {
    const CacheNode* na = *(const CacheNode* const*)a;
    const CacheNode* nb = *(const CacheNode* const*)b;
    return strcmp(na->url, nb->url);
}

void ht_save_results(HashTable* ht, const char* filename) {
    if (!ht || !filename) {
        fprintf(stderr, "Erro: Tabela ou nome de arquivo nulo ao salvar resultados.\n");
        return;
    }
    FILE* fp = fopen(filename, "w");
    if (!fp) {
        perror("Erro ao abrir arquivo de resultados");
        return;
    }
    /* Contar nós */
    size_t total = 0;
    for (size_t i = 0; i < ht->size; i++) {
        for (CacheNode* c = ht->table[i]; c; c = c->next) total++;
    }
    CacheNode** arr = (CacheNode**)malloc(total * sizeof(CacheNode*));
    if (!arr) {
        perror("Erro ao alocar array para ordenação");
        fclose(fp);
        return;
    }
    size_t idx = 0;
    for (size_t i = 0; i < ht->size; i++) {
        for (CacheNode* c = ht->table[i]; c; c = c->next) {
            arr[idx++] = c;
        }
    }
    qsort(arr, total, sizeof(CacheNode*), cmp_nodes_by_url);
    for (size_t i = 0; i < total; i++) {
        fprintf(fp, "%s,%ld\n", arr[i]->url, arr[i]->hit_count);
    }
    free(arr);
    fclose(fp);
}

void ht_print(HashTable* ht) {
    if (!ht) return;
    printf("--- Estado da Tabela Hash (Size: %zu) ---\n", ht->size);
    for (size_t i = 0; i < ht->size; i++) {
        printf("Bucket[%zu]: ", i);
        CacheNode* current = ht->table[i];
        if (!current) {
            printf("~ VAZIO ~\n");
            continue;
        }
        while (current) {
            printf("[\"%s\" (%ld)] -> ", current->url, current->hit_count);
            current = current->next;
        }
        printf("NULL\n");
    }
    printf("-----------------------------------------\n");
}
