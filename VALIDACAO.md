# Validação do pacote

## Ambiente efetivamente utilizado

- Linux x86-64, GCC 13.3.0, Python 3.12.
- O compilador disponível não aceita a grafia `-std=c23`; a validação local
  usou `-std=c2x`, com `-O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Werror`.
- O Makefile e os comandos de entrega usam `-std=c23` para o GCC 16 solicitado.
  O código é C válido nesse padrão, sem exigir recursos novos não implementados
  pelo compilador local. A compilação exata com GCC 16 não foi executada aqui.
- O ramo de relógio Windows usa as APIs documentadas QueryPerformanceCounter
  e QueryPerformanceFrequency, mas não foi executado em Windows neste ambiente.
  Execute `make test` e o piloto na máquina onde fará a coleta.

## Verificações concluídas

1. Compilação otimizada sem avisos, tratando avisos como erros.
2. 47.795 entradas verificadas nos cinco algoritmos, nas versões normal e
   instrumentada: todas as permutações até oito elementos; entradas aleatórias,
   ordenadas, invertidas e com duplicatas; comprimentos pares/ímpares; e extremos
   INT32_MIN/INT32_MAX. Também foi testada a chamada com comprimento zero.
3. Contagens manuais de `[3,1,2]` e relações fechadas para Selection, Insertion,
   Quick e Merge, conforme `tests/test_algoritmos.c`.
4. Execução dos testes e de um benchmark curto com AddressSanitizer e
   UndefinedBehaviorSanitizer, sem diagnósticos desses sanitizadores.
   LeakSanitizer não funciona sob a instrumentação deste ambiente e foi desativado
   com `ASAN_OPTIONS=detect_leaks=0`; não se afirma uma verificação de vazamentos.
5. Fluxo de coleta com seis cenários, quatro tamanhos, quatro amostras e três
   repetições: 1.440 linhas brutas, 480 linhas de amostras e 120 linhas de resumo.
6. Reexecução com os mesmos parâmetros confirmou sementes, hashes, contagens e
   ordem de execução. Tempos não são esperados iguais entre execuções.
7. Verificação independente das fórmulas de média por entrada, desvio amostral,
   contagens e normalizações em `tests/test_analise.py`.
8. Exportação de 24 gráficos em PNG e 24 em SVG, com inspeção visual de um
   gráfico de tempo. Os dados usados foram somente de teste.
9. Rejeição de parâmetros inválidos, proteção contra sobrescrita do CSV e
   recusa de ensaio interrompido pelo analisador.

Os CSVs e gráficos de validação não integram o pacote. Eles serviram para
testar as ferramentas, não para representar o desempenho na máquina do aluno.

## Reexecutar os testes no seu ambiente

```bash
make test
python tests/test_analise.py
```

Os testes validam correção e tratamento dos dados. Eles não substituem o piloto
nem demonstram, por si só, precisão dos tempos coletados em outro ambiente.
