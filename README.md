# CDN Cache Analyzer — Contagem de acessos com OpenMP

> **English:** Academic lab in C that counts URL hits from large CDN access logs (10 million lines each) using a chained hash table, comparing a sequential implementation against two OpenMP parallel versions (`atomic` vs `critical`), on a uniform workload and on a high-contention "hotspot" workload.

Projeto acadêmico de Computação Paralela. O programa lê um manifesto com as URLs de um CDN e um log de acessos, conta quantas vezes cada URL foi requisitada e grava o resultado em CSV. O objetivo é comparar o desempenho e a corretude de uma versão sequencial com duas estratégias de sincronização em OpenMP.

## Como funciona

1. **Manifesto → tabela hash.** As URLs do manifesto são carregadas (sequencialmente) em uma tabela hash com encadeamento (`hash_table.c`, 100.000 buckets). Cada nó guarda a URL e um contador `hit_count`.
2. **Leitura do log.** As linhas do log são carregadas em memória.
3. **Contagem.** Para cada linha, extrai-se a URL da requisição (`"GET ... HTTP/"`) e incrementa-se o contador da URL correspondente.
   - `analyzer_seq.c`: versão sequencial (referência).
   - `analyzer_par_atomic.c`: laço paralelo com `#pragma omp parallel for` e incremento com `#pragma omp atomic update`.
   - `analyzer_par_critical.c`: laço paralelo protegendo o incremento com `omp critical`.
4. **Resultado.** A tabela é salva em `results.csv`.

## Dados de teste

O script `generate_cdn_data.py` gera um manifesto com 100.000 URLs únicas e dois logs de 10 milhões de linhas cada, além de um gabarito (CSV) para conferir a contagem:

- **Distribuído:** acessos espalhados de forma uniforme entre as URLs.
- **Concorrente (hotspot):** 90% do tráfego concentrado em 10 URLs, o que gera alta disputa pelos mesmos contadores.

## Estrutura

| Arquivo | Descrição |
| --- | --- |
| `hash_table.h` / `hash_table.c` | Tabela hash com encadeamento (create, put, get, save, destroy) |
| `analyzer_seq.c` | Versão sequencial |
| `analyzer_par_atomic.c` | Versão paralela com `atomic` |
| `analyzer_par_critical.c` | Versão paralela com `critical` |
| `generate_cdn_data.py` | Gerador de manifesto, logs e gabaritos |
| `Makefile` | Compilação, geração de dados, testes e verificação |
| `Relatorio.pdf` | Relatório do projeto |

## Como executar

Requisitos: `gcc` com suporte a OpenMP, Python 3 e `unzip`.

```bash
make                # compila as três versões
make generate-data  # gera e extrai manifesto, logs e gabaritos
make test           # roda sequencial e paralelo (4 threads) nos dois logs
make verify         # compara os resultados sequenciais com os gabaritos
```

Execução manual de uma versão paralela:

```bash
./analyzer_par_atomic.exe manifest.txt log_distribuido.txt 4   # <manifesto> <log> <threads>
```

O resultado fica em `results.csv` (`url,hit_count`).

## Relatório

A análise e os resultados do projeto estão em [Relatorio.pdf](Relatorio.pdf).
