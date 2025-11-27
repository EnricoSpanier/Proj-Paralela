# Makefile para Projeto CDN Cache Analyzer
# Compilação de versões sequencial e paralelas (OpenMP)

CC = gcc
CFLAGS = -O2 -Wall
OMPFLAGS = -fopenmp
SOURCES = hash_table.c
TARGETS = analyzer_seq.exe analyzer_par_atomic.exe analyzer_par_critical.exe

# Regra padrão: compila todas as versões
all: $(TARGETS)

# Versão Sequencial
analyzer_seq.exe: $(SOURCES) analyzer_seq.c
	$(CC) $(CFLAGS) $(SOURCES) analyzer_seq.c -o $@

# Versão Paralela com Atomic
analyzer_par_atomic.exe: $(SOURCES) analyzer_par_atomic.c
	$(CC) $(CFLAGS) $(OMPFLAGS) $(SOURCES) analyzer_par_atomic.c -o $@

# Versão Paralela com Critical
analyzer_par_critical.exe: $(SOURCES) analyzer_par_critical.c
	$(CC) $(CFLAGS) $(OMPFLAGS) $(SOURCES) analyzer_par_critical.c -o $@

# Limpar executáveis e resultados
clean:
	# Remove executáveis, resultados, gabaritos, manifesto, logs e o zip gerado
	rm -f $(TARGETS) results*.csv gabarito_*.csv cdn_data_logs.zip manifest.txt log_distribuido.txt log_concorrente.txt

# Limpar apenas resultados
clean-results:
	rm -f results*.csv gabarito_*.csv

# Gerar dados de teste
generate-data:
	python generate_cdn_data.py
	unzip -o cdn_data_logs.zip -d .

# Executar testes sequenciais
test-seq: analyzer_seq.exe
	@echo "=== Teste Sequencial - Log Distribuido ==="
	./analyzer_seq.exe manifest.txt log_distribuido.txt
	@mv results.csv results_seq_dist.csv
	@echo "=== Teste Sequencial - Log Concorrente ==="
	./analyzer_seq.exe manifest.txt log_concorrente.txt
	@mv results.csv results_seq_conc.csv

# Executar testes paralelos (4 threads)
test-par: analyzer_par_atomic.exe analyzer_par_critical.exe
	@echo "=== Teste Atomic (4 threads) - Log Distribuido ==="
	./analyzer_par_atomic.exe manifest.txt log_distribuido.txt 4
	@mv results.csv results_atomic_dist.csv
	@echo "=== Teste Atomic (4 threads) - Log Concorrente ==="
	./analyzer_par_atomic.exe manifest.txt log_concorrente.txt 4
	@mv results.csv results_atomic_conc.csv
	@echo "=== Teste Critical (4 threads) - Log Distribuido ==="
	./analyzer_par_critical.exe manifest.txt log_distribuido.txt 4
	@mv results.csv results_critical_dist.csv
	@echo "=== Teste Critical (4 threads) - Log Concorrente ==="
	./analyzer_par_critical.exe manifest.txt log_concorrente.txt 4
	@mv results.csv results_critical_conc.csv

# Executar todos os testes
test: test-seq test-par
	@echo "=== Todos os testes concluidos ==="

# Verificar corretude comparando com gabaritos
verify:
	@echo "Verificando resultados contra gabaritos..."
	@diff -q results_seq_dist.csv gabarito_distribuido.csv && echo "✓ Sequencial Distribuido: OK" || echo "✗ Sequencial Distribuido: DIFERENTE"
	@diff -q results_seq_conc.csv gabarito_concorrente.csv && echo "✓ Sequencial Concorrente: OK" || echo "✗ Sequencial Concorrente: DIFERENTE"

# Ajuda
help:
	@echo "Comandos disponiveis:"
	@echo "  make              - Compila todas as versoes"
	@echo "  make clean        - Remove executaveis e resultados"
	@echo "  make generate-data - Gera dados de teste"
	@echo "  make test-seq     - Executa testes sequenciais"
	@echo "  make test-par     - Executa testes paralelos"
	@echo "  make test         - Executa todos os testes"
	@echo "  make verify       - Verifica corretude dos resultados"
	@echo "  make help         - Mostra esta mensagem"

.PHONY: all clean clean-results generate-data test-seq test-par test verify help
