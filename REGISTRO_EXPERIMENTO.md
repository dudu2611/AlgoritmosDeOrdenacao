# Registro de uma sessão de medições

Copie esta ficha para cada sessão. Campos vazios devem ser preenchidos com
o que ocorreu de fato; não estime resultados que não foram medidos.

## Identificação e máquina

| Item | Registro |
|---|---|
| Nome da sessão / prefixo `--out` | |
| Integrantes responsáveis | |
| Data e horário de início/fim, com fuso | |
| Objetivo e hipótese observável | |
| Processador | |
| RAM | |
| Sistema operacional e versão | |
| GCC: versão completa e target | |
| Flags usadas na compilação | |
| Hash do executável / arquivo ambiente.json | |
| Tomada ou bateria; plano de energia | |
| Outros programas em execução | |
| Condições relevantes de temperatura/ventilação | |

## Configuração

```text
Cole aqui o comando completo executado:

```

| Item | Registro |
|---|---|
| Algoritmos e variantes | |
| Cenários e construção das entradas | |
| Tamanhos, na ordem executada | |
| Amostras / repetições técnicas / aquecimentos | |
| Semente mestra | |
| Relógio, resolução e mediana do intervalo vazio | |
| Escopo: chamada de ordenação, sem alocação/cópia/contadores | |
| Validação inicial: testes passaram? | |

## Ocorrências durante a sessão

| Horário | Ocorrência observada | Medida tomada / identificação de nova sessão |
|---|---|---|
| | | |

## Conclusão da coleta

| Verificação | Registro |
|---|---|
| `_meta.txt` contém `completed=yes`? | |
| Número de linhas esperado e obtido | |
| Ordenações validadas? | |
| Quantidade de `short_measurement=1` | |
| Pasta com CSV bruto, metadados e registro do ambiente | |
| Comando de análise e pasta de saída | |
| Algum dado foi excluído? Justificativa e regra definida | |
| É necessário repetir? Motivo e novo prefixo | |

## Observações para a discussão

- Crescimento observado quando `n` aumenta:
- Comportamento por cenário e justificativa teórica:
- Dispersão entre repetições e entre amostras:
- Limitações que devem ser declaradas:
