# Análise empírica de ordenação — C23

Projeto para o Trabalho 1 de Análise de Algoritmos: Insertion, Selection,
Merge, Heap e Quick Sort, com medições reproduzíveis e análise estatística.

**Comece por [GUIA_MEDICOES.md](GUIA_MEDICOES.md).** Ele explica os comandos,
o planejamento, o significado de cada medida e como registrar os experimentos.

## Início rápido — terminal MSYS2 UCRT64 ou Linux

Na pasta extraída, usando GCC 16:

```bash
make
make test
mkdir -p resultados
./benchmark.exe --sizes 1000,2000,4000 --samples 5 --repeats 3 --out resultados/piloto
python tools/analisar.py resultados/piloto_raw.csv --out resultados/analise_piloto
```

Esse é um **piloto**, não a coleta final. Se seu terminal oferece `python3`,
substitua `python` por `python3`. Sem Make, consulte os comandos GCC no guia.

## Arquivos

| Arquivo | Função |
|---|---|
| `src/algoritmos.c` | Os cinco algoritmos comentados; compilado com e sem contadores |
| `src/algoritmos.h` | Interface e definição das métricas |
| `src/benchmark.c` | Amostragem, cronômetro, validação e CSV bruto |
| `src/dados.c`, `src/dados.h` | Gerador reproduzível e seis cenários de entrada |
| `src/tempo.c`, `src/tempo.h` | Relógio de alta resolução para Windows e POSIX |
| `tools/analisar.py` | Médias, desvios, razões de crescimento e gráficos opcionais |
| `tools/registrar_ambiente.py` | Sistema, compilador e hashes dos arquivos |
| `tests/test_algoritmos.c` | Correção das ordenações e contagens conhecidas |
| `tests/test_analise.py` | Verificação independente do agrupamento estatístico |
| `REGISTRO_EXPERIMENTO.md` | Ficha para preencher durante cada sessão |
| `VALIDACAO.md` | Ambiente e limites dos testes realizados na preparação |
| `Makefile` | Compilação C23 e testes |

Não há dependências externas para o programa C. As tabelas usam apenas a
biblioteca padrão do Python 3.10 ou superior. Matplotlib é opcional para gráficos.

O arquivo `src/algoritmos.c` é compilado duas vezes para que os contadores não
alterem o tempo cronometrado. Não compile apenas `gcc src/*.c`: use o Makefile
ou os comandos completos do guia. A extensão `.exe` também é usada no Linux
para manter os mesmos comandos; o executável continua sendo nativo do sistema.
