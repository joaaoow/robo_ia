# Relatório — Robô Aspirador como Agente Reflex em C

---

## 1. Introdução

Este trabalho implementa um **agente reflex simples** para o problema do robô limpador em linguagem C. O agente percebe o estado atual do ambiente (posição e grade) e decide uma ação a cada passo **sem guardar memória de passos anteriores**. O objetivo é simular o comportamento do agente e analisar suas capacidades e limitações em três mapas de complexidade crescente.

---

## 2. Arquitetura do Sistema

O projeto é dividido em quatro módulos:

| Módulo | Responsabilidade |
|---|---|
| `mapa.c/h` | Leitura, validação e representação da grade; funções `eh_bloqueio` e `eh_sujo` |
| `agente.c/h` | Lógica de decisão (`decide_reflex`) e execução da ação (`aplicar_acao`) |
| `log.c/h` | Buffer circular com histórico das últimas 64 ações |
| `main.c` | Loop de simulação, métricas e modo passo a passo |

### Representação do ambiente

| Símbolo | Significado |
|---|---|
| `.` | Célula limpa |
| `*` | Célula suja |
| `#` | Obstáculo intransponível |
| `S` | Posição inicial do robô (convertido para `.` internamente) |
| `!` | Célula de risco — transitável, mas com custo extra de 1 passo |

### Regras do agente (em ordem de prioridade)

1. **Regra 1** — Se a célula atual é suja → `LIMPAR`
2. **Regra 2** — Se há sujeira em vizinho imediato (N, S, L, O) → mover em direção a ela
3. **Regra 3** — Varredura zig-zag por linhas: linha par tenta L→S→O; linha ímpar tenta O→S→L
4. **Fallback** — Tenta Norte, depois Sul; se ambos bloqueados → `FICAR`

---

## 3. Resultados das Execuções

### Quadro de métricas

| Mapa | Passos | Limpezas | Bloqueios | % Removida | Tempo CPU |
|---|---|---|---|---|---|
| Fácil (5×7, T=80) | 80 | 2 | 0 | 66,67% | 0,000024 s |
| Médio (7×9, T=160) | 160 | 0 | 0 | 0,00% | 0,000035 s |
| Difícil (10×12, T=300) | 300 | 0 | 0 | 0,00% | 0,000026 s |

### Saída do terminal — Mapa Fácil

```
===== RESULTADOS =====
Passos executados : 80
Limpezas          : 2
Bloqueios         : 0
Sujeira inicial   : 3
Sujeira restante  : 1
Percentual limpo  : 66.67%
Entradas em risco : 0
Tempo de CPU      : 0.000024 s

Mapa final:
......#
..#....
..#....
..*..#.
......R
Posicao final do robo: (4, 6)
```

### Saída do terminal — Mapa Médio

```
===== RESULTADOS =====
Passos executados : 160
Limpezas          : 0
Bloqueios         : 0
Sujeira inicial   : 8
Sujeira restante  : 8
Percentual limpo  : 0.00%
Entradas em risco : 0
Tempo de CPU      : 0.000035 s

Mapa final:
..R#..*..
.##..#..*
..*..#...
..##..*..
...!*....
..*..#..*
..#...*..
Posicao final do robo: (0, 2)
```

### Saída do terminal — Mapa Difícil

```
===== RESULTADOS =====
Passos executados : 300
Limpezas          : 0
Bloqueios         : 0
Sujeira inicial   : 11
Sujeira restante  : 11
Percentual limpo  : 0.00%
Entradas em risco : 0
Tempo de CPU      : 0.000026 s

Mapa final:
..R#..*...#.
.##..#..*...
..*..#...#..
..##..*..#..
...!*....#..
..*..#..*..#
.#..##..*...
..*....#..#.
...#..*..#..
..#...*..#..
Posicao final do robo: (0, 2)
```

---

## 4. Decisões Observadas no Modo Passo a Passo (Mapa Fácil)

### Decisão 1 — Passo 3: Regra 2 (vizinho sujo)

```
Passo 3
Acao: MOVER_L
Regra: Regra 2: vizinho sujo ao leste

Mapa:
...R..#
..#..*.
..#....
..*..#.
.......
Sujeira restante: 3 | Limpezas: 0 | Bloqueios: 0
```

**Explicação:** O robô estava em `(0,2)`. Nos passos anteriores ele só via células limpas e seguia o zig-zag. No passo 3, detectou que a célula vizinha a leste `(0,3)` continha `*`. A Regra 2 tem prioridade sobre o zig-zag, então o agente ignorou a varredura e se moveu diretamente em direção à sujeira.

---

### Decisão 2 — Passo 4: Regra 1 (célula atual suja)

```
Passo 4
Acao: LIMPAR
Regra: Regra 1: celula atual suja

Mapa:
...R..#
..#..*.
..#....
..*..#.
.......
Sujeira restante: 2 | Limpezas: 1 | Bloqueios: 0
```

**Explicação:** Após se mover para `(0,3)` no passo anterior, o robô percebeu que a célula onde está contém `*`. A Regra 1 é a de maior prioridade — antes de qualquer verificação de vizinhos ou zig-zag, o agente limpa a própria posição. O contador de sujeira caiu de 3 para 2.

---

Esses dois passos consecutivos ilustram o padrão típico do agente: **primeiro se aproxima da sujeira (Regra 2), depois a limpa (Regra 1)**.

---

## 5. Limitações do Agente Reflex (comentário)

O agente reflex não planeja: ele apenas reage ao que vê no momento, sem lembrar onde já esteve. Isso faz com que ele entre em loops quando obstáculos bloqueiam o caminho, oscilando entre as mesmas células indefinidamente — como ocorreu nos mapas médio e difícil, onde o robô ficou preso entre as colunas 1 e 2 por todos os 160 e 300 passos. Ele também não minimiza o número de passos: pode limpar uma célula depois de dar dezenas de passos desnecessários. Por fim, não consegue planejar rotas para sujeira isolada atrás de obstáculos, simplesmente deixando essas células sem limpar.

---

## 6. Respostas à Rubrica

**Por que um agente reflex não garante o caminho mínimo?**
Porque ele não enxerga além dos vizinhos imediatos e não planeja. Cada decisão é local e sem memória — o agente pode dar voltas desnecessárias antes de encontrar a sujeira, enquanto um agente com BFS, por exemplo, calcularia o caminho mais curto diretamente.

**Em que situações as Regras 2 e 3 entram em conflito?**
A Regra 2 tem prioridade sobre a 3, então tecnicamente não há conflito — a Regra 3 só executa quando a Regra 2 não disparou. Porém há tensão quando a Regra 2 puxa o robô para uma direção e o zig-zag da Regra 3 puxaria para outra. Por exemplo: se há sujeira ao norte mas o zig-zag indicaria leste, o robô vai ao norte. Isso pode quebrar a varredura sistemática e fazer com que ele precise revisitar regiões já cobertas.

**Como um agente com modelo do mundo (memória de visitados) poderia melhorar a cobertura?**
Com um conjunto de células visitadas, o agente poderia evitar revisitar posições já limpas e reconhecer quando entrou em loop. Combinado com uma fila de prioridade (ex: BFS ou A*) para células sujas não visitadas, o agente encontraria toda a sujeira acessível no menor número de passos possível, em vez de depender de uma varredura que pode ser bloqueada por obstáculos.

---

## 7. Extensão Implementada (Bônus)

### Penalidade de risco (`!`)

A penalidade de risco foi implementada no código. Células marcadas com `!` são transitáveis, mas cada entrada nelas desconta 1 passo extra do limite T (custo total = 2 passos por entrada). O contador `risco` registra quantas vezes o robô entrou em uma célula `!`, e o valor é exibido nas métricas finais como **"Entradas em risco"**.

Nos mapas médio e difícil há células `!` inseridas. Como o robô ficou preso próximo ao início nesses mapas, não chegou a entrar nas zonas de risco — mas o mecanismo está funcional e pode ser verificado em mapas menores onde `!` fique no caminho do agente.

---

## 8. Conclusão

O agente reflex simples implementado demonstra as características clássicas dessa arquitetura: decisão em O(1) por passo, custo de memória mínimo e comportamento correto em ambientes sem obstáculos complexos (66,67% no mapa fácil). Porém, confirma-se a limitação teórica: **sem memória, o agente não consegue escapar de loops causados por obstáculos**, resultando em 0% de limpeza nos mapas médio e difícil.

Os resultados reforçam que a escolha da arquitetura do agente deve ser guiada pela complexidade do ambiente. Para mapas com obstáculos e sujeira distribuída, é indispensável ao menos um mecanismo de rastreamento de estado (agente reflex com estado) ou um planejador de caminho (agente baseado em objetivos).
