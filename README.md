# Jogo com Matriz em Linguagem C

Trabalho da disciplina de Lógica de Programação. Jogo automático executado no terminal, desenvolvido em C utilizando matriz bidimensional como tabuleiro.

## Tema

Robô explorador que percorre um labirinto coletando itens, buscando uma chave e tentando chegar à saída antes que a energia acabe ou o inimigo o alcance.

## Como compilar e executar

```bash
gcc -o jogo jogo.c
./jogo
```

## Como o jogo funciona

O jogo roda sozinho — não é necessário apertar nenhuma tecla. O jogador e o inimigo se movem automaticamente a cada 1 segundo.

**Objetivo:** coletar a chave (⚿) e chegar à saída (▣).

**Condições de derrota:**
- Energia chegar a zero
- Perder todas as vidas ao ser alcançado pelo inimigo

## Elementos do tabuleiro

| Símbolo | Significado |
|---------|-------------|
| ☻ | Jogador |
| ☹ | Inimigo |
| ▧ | Obstáculo interno |
| ♥ | Vida extra |
| ⚡ | Energia |
| ⚿ | Chave (necessária para abrir a saída) |
| ▣ | Saída |
| ►◄▲▼ | Setas (redirecionam o personagem) |

## Lógica de movimentação

Tanto o jogador quanto o inimigo seguem a mesma lógica de movimento automático:

1. Tenta continuar na direção atual
2. Se bloqueado por parede ou obstáculo, tenta girar 90° no sentido horário
3. Se ainda bloqueado, tenta 90° no sentido anti-horário
4. Se ainda bloqueado, inverte a direção

Ao pisar em uma seta, o personagem muda de direção para a direção indicada pela seta.

## Variáveis de estado

- **Vidas** — começa com 3; aumenta coletando ♥ (máx. 5); zera ao encostar no inimigo sem reserva
- **Energia** — começa em 100; diminui 1 por rodada; aumenta 20 ao coletar ⚡; zera = derrota
- **Pontos** — acumula ao coletar itens (⚡ = +10, ♥ = +15, ⚿ = +50)
- **Chave** — indica se a chave já foi coletada; sem ela a saída é bloqueada

## Estrutura do código

| Função | Descrição |
|--------|-----------|
| `imprimeMatriz` | Desenha o tabuleiro no terminal |
| `imprimeStatus` | Exibe posições, vidas, chave e energia |
| `escolherDirecao` | Decide a direção de movimento evitando paredes |
| `moverJogador` | Move o jogador e processa colisões e coletas |
| `moverInimigo` | Move o inimigo e verifica colisão com o jogador |
| `proximaPosicao` | Calcula a próxima posição dado uma direção |
| `ehParede` | Verifica se um valor da matriz é intransponível |
| `ehSeta` | Verifica se um valor da matriz é uma seta |
| `direcaoDaSeta` | Retorna a direção correspondente a uma seta |
| `direcaoOposta` | Retorna a direção oposta |
| `direcaoHorario` | Gira a direção 90° no sentido horário |

## Detecção de colisão

Toda colisão é feita consultando o valor da matriz na próxima posição antes de mover o personagem:

```c
int dest = mat[nl][nc];

if (ehParede(dest))  → não move
if (dest == INIMIGO) → perde vida
if (dest == CHAVE)   → coleta chave
if (dest == SAIDA)   → verifica se tem chave (vitória ou bloqueio)
```
