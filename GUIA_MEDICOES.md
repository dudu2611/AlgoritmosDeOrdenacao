# Guia de execução e registro das medições

**Trabalho 1 — Análise Empírica de Algoritmos de Ordenação**  
Linguagem: C23 · Compilação prevista: GCC 16 · Plataformas: Windows e Linux/POSIX

## 1. O que o pacote entrega

O enunciado pede Insertion Sort, Selection Sort, Merge Sort, Heap Sort e Quick
Sort sob condições similares; medidas de tempo e operações; definição da
amostragem; várias execuções com média e desvio padrão; e discussão com a teoria.
Melhor/pior caso e caso médio aparecem como análise adicional. Este pacote
fornece o código e o procedimento para coletar esses dados.

Você vai executar o experimento na sua máquina. Os testes de desenvolvimento
deste pacote não são resultados para o relatório. Não há números experimentais
inventados ou tabelas previamente preenchidas.

| Componente | O que faz |
|---|---|
| Programa C | Gera entradas, ordena, cronometra, conta operações e exporta CSV |
| Análise Python | Produz tabelas de média, desvio padrão, mediana e crescimento |
| Gráficos opcionais | Exporta PNG e SVG por cenário, a partir dos seus dados |
| Registro do ambiente | Guarda sistema, compilador, sementes, parâmetros e hashes |
| Ficha de sessão | Ajuda a anotar condições e ocorrências durante a coleta |

## 2. Compilar com GCC 16

### Opção A: MSYS2 UCRT64, com Make

Extraia o ZIP. Abra o terminal **UCRT64** e entre na pasta do projeto. Exemplo,
se você extraiu em `D:\C++\Trabalho1\analise_ordenacao_c23`:

```bash
cd /d/C++/Trabalho1/analise_ordenacao_c23
gcc --version
make
make test
```

Se seu Make se chama `mingw32-make`, use esse nome. O programa de teste deve
terminar com `OK`. O `Makefile` usa `-std=c23 -O2` e avisos de compilação.
Não use `g++`: os fontes são C, não C++.

Se mudar flags ou os compiladores, execute `make clean` antes de recompilar.
O Make não detecta mudanças de flags por conta própria. Preserve as mesmas
flags para todos os algoritmos e durante toda a coleta.

### Opção B: somente GCC, sem Make

Na raiz do projeto, execute os comandos abaixo. Funcionam no terminal UCRT64
e também em CMD/PowerShell se `gcc` estiver no PATH. Crie `build` apenas se
a pasta ainda não existir:

```bash
mkdir build
gcc -std=c23 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Isrc -c src/algoritmos.c -o build/algoritmos.o
gcc -std=c23 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Isrc -DINSTRUMENTED -c src/algoritmos.c -o build/algoritmos_counted.o
gcc -std=c23 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Isrc src/benchmark.c src/dados.c src/tempo.c build/algoritmos.o build/algoritmos_counted.o -o benchmark.exe
gcc -std=c23 -O2 -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Isrc tests/test_algoritmos.c src/dados.c build/algoritmos.o build/algoritmos_counted.o -o test_algoritmos.exe
```

Execute `./test_algoritmos.exe` no UCRT64/Linux, `test_algoritmos.exe` no CMD
ou `.\test_algoritmos.exe` no PowerShell.

### Por que compilar algoritmos.c duas vezes?

Uma versão serve para medir **tempo sem instrumentação**. A outra recebe
`-DINSTRUMENTED`, gera funções com sufixo `_counted` e conta as operações.
As duas vêm do mesmo código, evitando manter algoritmos diferentes para cada
medida. As macros `LT`, `LE`, `PUT_MAIN` e `PUT_AUX` são comparações e atribuições
comuns na compilação normal; na outra, incrementam os contadores correspondentes.

Você pode estudar cada algoritmo diretamente em `src/algoritmos.c`. Os `static`
nesse arquivo indicam funções auxiliares restritas àquela unidade de compilação.

## 3. Faça primeiro um piloto

No UCRT64/Linux:

```bash
mkdir -p resultados
./benchmark.exe --sizes 1000,2000,4000 --samples 5 --repeats 3 --warmups 1 --seed 20260918 --scenarios aleatorio --out resultados/piloto
```

No CMD/PowerShell, crie a pasta com `mkdir resultados` e use, respectivamente,
`benchmark.exe` ou `.\benchmark.exe` no início dos comandos. Os parâmetros são iguais.

O piloto serve para verificar compilação, duração, resolução do relógio e
formato dos resultados. Abra `piloto_meta.txt` e confira `completed=yes`.
Não misture as cinco amostras do piloto com as da coleta final.

O programa cria:

| Arquivo | Conteúdo |
|---|---|
| `piloto_raw.csv` | Uma linha para cada ordenação cronometrada |
| `piloto_meta.txt` | Compilador, relógio, sementes, parâmetros e situação da execução |

O programa não sobrescreve arquivos. Para repetir, escolha outro prefixo,
como `--out resultados/piloto_02`. O diretório de saída precisa existir.
Em caso de interrupção, preserve os arquivos parciais como registro e rode de
novo com outro nome. A análise recusa ensaios sem `completed=yes`.

## 4. Plano de coleta sugerido

Comece com **30 amostras por tamanho e cenário**, **3 cronometragens por amostra**
e **1 aquecimento por algoritmo/tamanho/cenário**. Trinta é um ponto de partida
prático, não uma garantia de precisão estatística; ajuste após o piloto.

### Experimento principal: comparação justa dos cinco algoritmos

Use cinco tamanhos em progressão geométrica e permutações aleatórias:

```bash
./benchmark.exe --sizes 1000,2000,4000,8000,16000 --samples 30 --repeats 3 --warmups 1 --seed 20260918 --scenarios aleatorio --out resultados/principal
```

São `5 algoritmos × 5 tamanhos × 30 amostras × 3 repetições = 2.250` linhas
de tempo, além das execuções de aquecimento e contagem que não entram nesse
total. Todos os algoritmos usam exatamente os mesmos vetores de cada amostra.

### Experimento adicional: sensibilidade à entrada

```bash
./benchmark.exe --sizes 1000,2000,4000,8000,16000 --samples 30 --repeats 3 --warmups 1 --seed 20260918 --scenarios crescente,decrescente,quase_ordenado,poucos_valores,iguais --out resultados/cenarios
```

Este segundo comando produz 11.250 linhas. Pode levar consideravelmente mais
tempo: o Quick Sort fornecido tem comportamento quadrático em entradas
crescentes, decrescentes e iguais. Faça um piloto com esses cenários também
se o tempo disponível for curto. Defina a faixa final antes de iniciar a coleta.

### Extensão para tamanhos grandes

Após a comparação na faixa comum, investigue apenas os algoritmos mais
adequados a tamanhos grandes, com entradas aleatórias:

```bash
./benchmark.exe --algorithms merge,heap,quick --sizes 32000,64000,128000,256000,512000 --samples 30 --repeats 3 --warmups 1 --seed 20260918 --scenarios aleatorio --out resultados/escala
```

Apresente essa extensão separadamente. Não atribua resultados a Insertion ou
Selection nos tamanhos em que eles não foram executados. A opção de algoritmos
não modifica os vetores gerados para uma mesma semente/tamanho/cenário/amostra.

Por padrão, o programa rejeita `n > 32768` se houver Insertion/Selection ou
Quick Sort em cenários diferentes de `aleatorio`. Isso evita iniciar por engano
bilhões de comparações. `--allow-slow` permite executar esses casos conscientemente.
O limite absoluto do programa é dez milhões de elementos; quatro vetores de
`int32_t` usam aproximadamente `16n` bytes, além das demais estruturas.

## 5. O que significa cada cenário

| Nome no comando | Construção exata | Interpretação |
|---|---|---|
| `aleatorio` | Fisher–Yates sobre `0,1,...,n-1` | Permutação pseudoaleatória uniforme de chaves distintas, usando rejeição para evitar viés modular |
| `crescente` | `0,1,...,n-1` | Já ordenado |
| `decrescente` | `n-1,...,1,0` | Ordem inversa, sem empates |
| `quase_ordenado` | Crescente, seguido de `max(1,floor(n/100))` trocas de posições distintas | Perturbações aleatórias; posições podem reaparecer e trocas podem se desfazer |
| `poucos_valores` | Cada posição recebe um inteiro pseudoaleatório entre 0 e 7 | Muitos valores repetidos |
| `iguais` | Todos os elementos valem 7 | Caso extremo de duplicação |

O gerador é SplitMix64 com estado explícito; não depende da implementação de
`rand()`. Cada entrada tem uma semente derivada da semente mestra, tamanho,
cenário e número da amostra. O código do gerador faz parte do experimento.

`quase_ordenado` não significa que exatamente 1% dos elementos estão fora do
lugar, nem que há 1% de inversões. As trocas podem envolver posições distantes.
Descreva a construção real no relatório.

Em `crescente`, `decrescente` e `iguais`, as 30 rodadas repetem o mesmo vetor
para um dado `n`: muda a execução, não a entrada. Nesses casos, a dispersão do
tempo representa principalmente ruído do ambiente. Nos cenários aleatórios,
ela também reflete diferenças entre entradas. Entradas distintas não são uma
prova de independência estatística; o desenho usa um gerador determinístico.

## 6. Exatamente o que é medido

### Tempo

O intervalo cronometrado contém somente a chamada da ordenação sem contadores.
A medição usa `QueryPerformanceCounter` no Windows e
`clock_gettime(CLOCK_MONOTONIC)` no Linux/POSIX. Mede tempo decorrido, inclusive
interferência do sistema, e não apenas tempo de CPU. A infraestrutura do relógio
e da chamada deixa um pequeno custo residual; ele não é subtraído dos resultados.

Geração da entrada, alocação, cópia, geração da referência com `qsort`, validação,
contagem de operações, impressão e gravação do CSV ficam fora desse intervalo.
O Merge Sort recebe um buffer auxiliar pré-alocado. Logo, a comparação é do
**núcleo da ordenação com memória já disponível**; não inclui o custo de alocar
a memória adicional do Merge Sort. Registre essa escolha na metodologia.

O programa restaura o vetor antes de toda ordenação, inclusive aquecimentos e
contagens. Não cronometra repetidamente um vetor já ordenado por acidente.
A ordem dos algoritmos é embaralhada a cada rodada de cronometragem, usando um
estado aleatório separado da geração dos dados.

Após cada execução, o resultado completo é comparado com uma referência
ordenada. Isso verifica tanto a ordem quanto a preservação dos elementos e
faz a saída da ordenação ser utilizada pelo programa.

As cópias e validações acessam a memória. Portanto, este protocolo trabalha
com memória recentemente acessada e não controla cache frio. O aquecimento
acontece antes da primeira amostra de cada tamanho/cenário, usando aquela entrada.
A ordem de cenários é fixa e a de tamanhos segue `--sizes`; a randomização dos
algoritmos reduz um viés de ordem, mas não elimina efeitos térmicos da sessão.

### Operações

As contagens são feitas em uma execução separada sobre a mesma entrada original.

| Coluna | Definição |
|---|---|
| `comparisons` | Comparações efetivamente executadas entre chaves, incluindo a comparação que falha ao encerrar uma busca |
| `swaps` | Trocas entre índices diferentes; trocar um índice consigo mesmo não conta |
| `writes_main` | Atribuições a posições do vetor principal |
| `writes_aux` | Atribuições ao vetor auxiliar do Merge Sort |
| `writes_total` | Soma das duas escritas, calculada pela ferramenta de análise |

Comparações de índices, condições de laços, leituras, atribuições a variáveis
temporárias e operações da infraestrutura não entram nesses contadores.
Uma troca entre posições diferentes conta como uma troca e duas escritas no
vetor, mesmo se os valores forem iguais. Não some trocas com escritas como
se fossem categorias independentes.

Insertion e Merge fazem **zero trocas** nesta implementação: movimentam dados
por atribuições. Isso não significa custo zero; use também as escritas para
comparar o movimento de dados. O Insertion faz a atribuição final da chave
mesmo quando ela permanece na mesma posição.

São operações lógicas do código-fonte, não instruções de máquina, acessos à RAM
ou contadores de hardware. Uma comparação ou escrita lógica não tem custo fixo
em nanossegundos.

## 7. Registrar o ambiente e a sessão

Depois de compilar, antes da coleta:

```bash
python tools/registrar_ambiente.py --out resultados/ambiente_principal.json
```

O script registra sistema, processador quando disponível, versão e alvo do GCC,
além de hashes SHA-256 do executável, fontes e ferramentas. As flags no registro
são **declaradas**, não descobertas no executável. Se usar flags diferentes:

```bash
python tools/registrar_ambiente.py --out resultados/ambiente_alternativo.json --flags="-std=c23 -O3 -Wall -Wextra -Wpedantic -Wconversion -Wshadow"
```

Preencha uma cópia de `REGISTRO_EXPERIMENTO.md` com RAM, plano de energia,
uso de tomada/bateria, programas abertos, comando completo, horário e ocorrências.
Use a mesma máquina e as mesmas condições entre algoritmos. Evite executar
experimentos em paralelo. Em notebook, mantenha a alimentação e o plano de
energia consistentes. Não rode medições de desempenho com sanitizadores,
depurador ou a compilação de testes.

Se quiser checar efeito de ordem, faça uma segunda sessão com novos prefixos
e a lista de tamanhos invertida. Analise as sessões separadamente antes de
decidir combiná-las; mudar semente, compilação, máquina ou condições altera
o experimento. O script de análise aceita um CSV por vez para preservar isso.

## 8. Transformar os dados brutos em estatísticas

Python 3.10 ou superior é suficiente para gerar as tabelas; não precisa de pandas.
No Windows, se `python` não estiver disponível, tente `py -3`.

```bash
python tools/analisar.py resultados/principal_raw.csv --out resultados/analise_principal
python tools/analisar.py resultados/cenarios_raw.csv --out resultados/analise_cenarios
```

Mantenha `_raw.csv` e `_meta.txt` juntos, com os nomes originais. A análise
verifica conclusão, número de linhas, repetições, entradas compartilhadas,
contagens consistentes e validação das ordenações. Ela exige uma pasta nova
ou vazia, para preservar análises anteriores.

| Saída | Unidade de observação e conteúdo |
|---|---|
| `amostras.csv` | Uma linha por algoritmo/cenário/tamanho/amostra; média das repetições técnicas e dispersão dentro da amostra |
| `resumo.csv` | Uma linha por algoritmo/cenário/tamanho; média, desvio padrão amostral, mediana, mínimo, máximo e CV |
| `crescimento.csv` | Razões entre tamanhos consecutivos, quando há pelo menos dois tamanhos |
| `LEIA-ME.txt` | Definições estatísticas e origem dos dados |

### Por que não usar as 90 cronometragens como 90 entradas independentes?

Com `S=30` amostras e `R=3` repetições, primeiro calculamos a média das três
cronometragens da mesma entrada:

$$x_i = \frac{1}{R}\sum_{j=1}^{R} t_{ij}.$$

Depois calculamos a média das 30 médias e o desvio padrão **amostral**:

$$\bar{x}=\frac{1}{S}\sum_{i=1}^{S}x_i,\qquad
s=\sqrt{\frac{\sum_{i=1}^{S}(x_i-\bar{x})^2}{S-1}}.$$

O desvio em `time_std_ms` descreve a dispersão das médias por amostra.
`technical_std_mean_ms` é a média dos desvios dentro de cada entrada, útil
para examinar ruído entre repetições. Se `R=1`, esse último campo fica vazio:
um desvio amostral não é definido com uma única observação.

As contagens repetidas nas três linhas brutas são a mesma contagem, feita uma
vez. O script usa uma por amostra; não as soma nem as considera independentes.
As barras de erro, quando presentes, são **média ± um desvio padrão**, não
intervalos de confiança. Não conclua significância estatística apenas pela
aparência dessas barras.

### Dicionário do CSV bruto

| Coluna | Significado |
|---|---|
| `algorithm`, `scenario`, `n` | Algoritmo, construção da entrada e número de elementos |
| `sample` | Índice da entrada/rodada; começa em 1 |
| `repeat` | Índice da repetição técnica da mesma entrada; começa em 1 |
| `seed_hex` | Semente da entrada em hexadecimal, 64 bits |
| `input_hash` | Hash FNV-1a da entrada original; ajuda a conferir reprodução |
| `order` | Posição do algoritmo na rodada de cronometragem, após embaralhamento |
| `time_ns` | Tempo da chamada, em nanossegundos |
| `comparisons`, `swaps`, `writes_main`, `writes_aux` | Contagens de uma ordenação instrumentada |
| `validated` | 1 indica resultado igual à referência |
| `short_measurement` | 1 sinaliza um intervalo curto perante o diagnóstico do relógio |

O hash não é prova matemática de igualdade; o benchmark fornece as mesmas
entradas por cópia do mesmo vetor. No Excel/LibreOffice, importe os CSVs como
UTF-8, delimitador vírgula e decimal ponto. Importe `seed_hex` e `input_hash`
como texto. Faça cálculos nas colunas numéricas, preservando o arquivo bruto.
`1 ms = 1.000.000 ns`; não misture as unidades nas tabelas.

## 9. Gerar gráficos a partir dos seus dados

Esta etapa é opcional para a coleta, mas os gráficos serão úteis no relatório.
Instale Matplotlib no mesmo Python usado para executar a análise:

```bash
python -m pip install -r requirements-optional.txt
python tools/analisar.py resultados/principal_raw.csv --out resultados/analise_principal_graficos --plots
```

O script salva, para cada cenário, versões PNG e SVG de:

- tempo médio com desvio padrão, eixo vertical linear;
- tempo médio em escala logarítmica, para enxergar algoritmos de custos muito diferentes;
- número médio de comparações em escala logarítmica;
- número médio de escritas totais em escala logarítmica.

O eixo horizontal usa log₂ de `n`. Os gráficos de tempo logarítmico mostram
as médias; os desvios continuam disponíveis na tabela e no gráfico linear.
Zeros não aparecem em escala logarítmica, mas permanecem nos CSVs. Uma linha
entre tamanhos conecta medições; não representa dados coletados entre eles.

## 10. Como relacionar os resultados à teoria

As previsões abaixo dizem respeito às variantes deste pacote.

| Algoritmo | Previsão de tempo/comparações em termos assintóticos | Memória auxiliar do algoritmo |
|---|---|---|
| Insertion | Melhor Θ(n) em crescente; média sob permutações aleatórias e pior caso Θ(n²) | O(1) |
| Selection | Θ(n²) em todos os cenários, independentemente do número de trocas | O(1) |
| Merge | Θ(n log n) em todos os cenários: sempre divide e intercala, sem atalho para partes ordenadas | O(n), mais pilha O(log n) |
| Heap | O(n log n) em qualquer entrada e pior caso Θ(n log n); o caso de todos iguais é Θ(n) nesta variante com parada antecipada do sift-down | O(1) |
| Quick | Melhor Θ(n log n) com partições balanceadas; esperança Θ(n log n) em permutações aleatórias distintas; pior Θ(n²) | Pilha O(log n) por recursão somente na parte menor |

**No Quick Sort daqui, crescente e decrescente não são melhor caso.** O pivô
é o último elemento e a partição usa `<=`; essas entradas, assim como todos
iguais, produzem partições degeneradas. A eliminação da recursão na parte maior
evita uma pilha linear, mas não elimina o tempo quadrático. Muitos repetidos
também são desfavoráveis para essa partição em duas partes.

Os seis cenários não representam automaticamente o melhor e o pior caso de
todos os algoritmos. Por exemplo, não há um gerador dedicado de entradas que
garanta partições perfeitamente balanceadas no Quick Sort. Não chame um cenário
de melhor/pior caso sem justificar para aquela implementação.

### Relações exatas para conferir as contagens

Para `n >= 2`:

- Selection: `comparisons = n(n-1)/2` em qualquer entrada.
- Insertion crescente: `comparisons = n-1`, `writes_main = n-1`, `swaps = 0`.
- Insertion decrescente com chaves distintas: `comparisons = n(n-1)/2` e
  `writes_main = n(n-1)/2 + n-1`.
- Quick crescente, decrescente ou todos iguais: `comparisons = n(n-1)/2`.
- Merge, quando `n` é potência de dois: `writes_main = writes_aux = n log₂ n`.

Os testes incluídos verificam essas relações. Elas ajudam a detectar erros
de implementação e a explicar por que contagens podem ter desvio padrão zero
mesmo quando os tempos oscilam.

### Razões e normalização

Quando `n` dobra, compare o crescimento observado com:

| Modelo | Razão esperada T(2n)/T(n), desconsiderando constantes e termos menores |
|---|---|
| Linear | 2 |
| n log₂ n | `2 × log₂(2n) / log₂(n)` |
| Quadrático | 4 |

`crescimento.csv` faz a comparação usando os tamanhos efetivamente escolhidos,
mesmo quando não dobram. `resumo.csv` também contém `T(n)/n`, `T(n)/(n log₂ n)`
e `T(n)/n²`, além de normalizações das comparações. Uma estabilização aproximada
é evidência compatível com o modelo; medições finitas não provam uma classe
assintótica. O caso médio empírico é relativo à distribuição definida, não a
uma média universal sobre qualquer tipo de dado.

## 11. Como lidar com ruído e tempos pequenos

O programa mede 1.001 pares consecutivos de leituras do relógio e registra a
mediana dos intervalos vazios. Um tempo recebe `short_measurement=1` quando é
menor que `100 × max(mediana_do_intervalo_vazio, resolução_nominal)`.
Esse fator é um diagnóstico conservador escolhido para o experimento, não
uma garantia de erro relativo de 1%: chamadas, escalonamento e caches também
contribuem para a variação. Não há subtração automática do custo do relógio.

Se houver muitos sinais de tempo curto, aumente `n` para os algoritmos rápidos
em um experimento separado. Aumentar apenas o número de repetições ajuda com
ruído aleatório, mas não remove o custo fixo presente em cada chamada.
Use as contagens para interpretar tamanhos nos quais o relógio ainda domina.

O coeficiente de variação é `CV = 100 × s / média`. CV elevado pede investigação,
sem um corte universal. Observe se houve outros processos, aquecimento térmico,
mudança de energia ou uma interrupção. Registre ocorrências e, se necessário,
repita a sessão completa com outro prefixo. Não apague seletivamente tempos
altos só para deixar a curva mais bonita. O script mantém todas as observações.

## 12. O que guardar ao terminar

Guarde juntos os fontes e Makefile usados, hashes/versão do executável, comando
completo e flags, `_raw.csv`, `_meta.txt`, registro do ambiente, ficha preenchida,
tabelas e gráficos produzidos. Esses dados permitem explicar e reproduzir a análise.

Para a discussão posterior, responda com os dados: o que aconteceu ao dobrar
`n`; quais entradas mudaram mais o desempenho; se comparações e escritas explicam
as diferenças de tempo; quanto ruído foi observado; e quais limites decorrem
do hardware, da faixa de tamanhos e das variantes implementadas.

## Referências técnicas de implementação

- [GCC — opções de dialeto C, incluindo `-std=c23`](https://gcc.gnu.org/onlinedocs/gcc/C-Dialect-Options.html).
- [Microsoft — medição de intervalos com QueryPerformanceCounter](https://learn.microsoft.com/en-us/windows/win32/sysinfo/acquiring-high-resolution-time-stamps).
- [Linux man-pages — clock_gettime e CLOCK_MONOTONIC](https://man7.org/linux/man-pages/man3/clock_gettime.3.html).

O material foi preparado a partir do enunciado anexado, Trabalho 1 — Análise
Empírica, de Análise de Algoritmos, IGCE/UNESP. Veja `VALIDACAO.md` para o ambiente
em que o pacote foi efetivamente testado.
