#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <wchar.h>
#include <locale.h>
#include <string.h>

#define LINHAS 11
#define COLUNAS 12

#define VAZIO           0
#define JOGADOR         1
#define INIMIGO         2
#define PAREDE_VERTICAL 3
#define PAREDE_HORIZONTAL 4
#define OBSTACULO       5
#define CANTO_SUP_ESQ   6
#define CANTO_SUP_DIR   7
#define CANTO_INF_ESQ   8
#define CANTO_INF_DIR   9
#define VIDA            10
#define SETA_CIMA       11
#define SETA_BAIXO      12
#define SETA_DIR        13
#define SETA_ESQ        14
#define CHAVE           15
#define SAIDA           16
#define ENERGIA         17

#define DIR_DIREITA  0
#define DIR_ESQUERDA 1
#define DIR_CIMA     2
#define DIR_BAIXO    3

int ascii[] = {
    0,     /* 0  VAZIO            */
    9787,  /* 1  JOGADOR      ☻  */
    9785,  /* 2  INIMIGO      ☹  */
    9553,  /* 3  PAREDE_VERT  ║  */
    9552,  /* 4  PAREDE_HORIZ ═  */
    9639,  /* 5  OBSTACULO    ▧  */
    9556,  /* 6  CANTO_SUP_ESQ ╔ */
    9559,  /* 7  CANTO_SUP_DIR ╗ */
    9562,  /* 8  CANTO_INF_ESQ ╚ */
    9565,  /* 9  CANTO_INF_DIR ╝ */
    9829,  /* 10 VIDA          ♥  */
    9650,  /* 11 SETA_CIMA     ▲  */
    9660,  /* 12 SETA_BAIXO    ▼  */
    9658,  /* 13 SETA_DIR      ►  */
    9668,  /* 14 SETA_ESQ      ◄  */
    9919,  /* 15 CHAVE         ⚿  */
    9635,  /* 16 SAIDA         ▣  */
    9889   /* 17 ENERGIA       ⚡  */
};

/* ── utilitários de tela ── */

void limparTela() {
    fflush(stdout);
    system("clear");
}

void exibeChar(int codigo) {
    wprintf(L"%lc", (wchar_t)codigo);
}

/* ── impressão ── */

void imprimeMatriz(int mat[LINHAS][COLUNAS]) {
    int i, j;
    limparTela();
    for (i = 0; i < LINHAS; i++) {
        for (j = 0; j < COLUNAS; j++) {
            int v = mat[i][j];
            if (v == VAZIO) {
                wprintf(L"  ");
            } else if (v == PAREDE_HORIZONTAL) {
                exibeChar(ascii[v]);
                exibeChar(ascii[v]);
            } else if (v == CANTO_SUP_ESQ || v == CANTO_INF_ESQ) {
                exibeChar(ascii[v]);
                exibeChar(ascii[PAREDE_HORIZONTAL]);
            } else {
                exibeChar(ascii[v]);
                wprintf(L" ");
            }
        }
        wprintf(L"\n");
    }
}

void imprimeStatus(int jL, int jC, int iL, int iC,
                   int vidas, int energia, int pontos, int temChave) {
    wprintf(L"\n");
    exibeChar(ascii[JOGADOR]);
    wprintf(L": x = %d, y = %d\n", jC, jL);
    exibeChar(ascii[INIMIGO]);
    wprintf(L": x = %d, y = %d\n", iC, iL);
    wprintf(L"Vidas do ");
    exibeChar(ascii[JOGADOR]);
    wprintf(L": %d\n", vidas);
    wprintf(L"Chave: %ls\n", temChave ? L"SIM" : L"NAO");
    wprintf(L"Energia: %d | Pontos: %d\n", energia, pontos);
}

/* ── funções de direção ── */

int ehParede(int v) {
    return v == PAREDE_VERTICAL   || v == PAREDE_HORIZONTAL ||
           v == CANTO_SUP_ESQ    || v == CANTO_SUP_DIR     ||
           v == CANTO_INF_ESQ    || v == CANTO_INF_DIR     ||
           v == OBSTACULO;
}

int ehSeta(int v) {
    return v == SETA_CIMA || v == SETA_BAIXO ||
           v == SETA_DIR  || v == SETA_ESQ;
}

int direcaoDaSeta(int v) {
    if (v == SETA_CIMA)  return DIR_CIMA;
    if (v == SETA_BAIXO) return DIR_BAIXO;
    if (v == SETA_DIR)   return DIR_DIREITA;
    if (v == SETA_ESQ)   return DIR_ESQUERDA;
    return -1;
}

int direcaoOposta(int dir) {
    if (dir == DIR_DIREITA)  return DIR_ESQUERDA;
    if (dir == DIR_ESQUERDA) return DIR_DIREITA;
    if (dir == DIR_CIMA)     return DIR_BAIXO;
    return DIR_CIMA;
}

/* Gira 90° no sentido horário */
int direcaoHorario(int dir) {
    if (dir == DIR_DIREITA)  return DIR_BAIXO;
    if (dir == DIR_BAIXO)    return DIR_ESQUERDA;
    if (dir == DIR_ESQUERDA) return DIR_CIMA;
    return DIR_DIREITA;
}

void proximaPosicao(int l, int c, int dir, int *nl, int *nc) {
    *nl = l; *nc = c;
    if      (dir == DIR_DIREITA)  (*nc)++;
    else if (dir == DIR_ESQUERDA) (*nc)--;
    else if (dir == DIR_CIMA)     (*nl)--;
    else                          (*nl)++;
}

/*
 * Escolhe a direção de movimento:
 * 1º tenta a direção atual; se bloqueada, tenta horário, anti-horário e oposta.
 * bloquearSaida=1 trata SAIDA como parede (usado quando jogador não tem a chave).
 * Retorna -1 se encurralado.
 */
int escolherDirecao(int mat[LINHAS][COLUNAS], int l, int c, int dir,
                    int bloquearSaida) {
    int tentativas[4], nl, nc, k;
    tentativas[0] = dir;
    tentativas[1] = direcaoHorario(dir);
    tentativas[2] = direcaoHorario(direcaoOposta(dir));
    tentativas[3] = direcaoOposta(dir);

    for (k = 0; k < 4; k++) {
        proximaPosicao(l, c, tentativas[k], &nl, &nc);
        int dest = mat[nl][nc];
        if (ehParede(dest))               continue;
        if (bloquearSaida && dest == SAIDA) continue;
        return tentativas[k];
    }
    return -1;
}

/* ── movimento do jogador ── */

/* Retorna: 0=normal, -1=derrota, 1=vitória */
int moverJogador(int mat[LINHAS][COLUNAS],
                 int *jL, int *jC, int *jDir, int *fundoJ,
                 int *vidas, int *energia, int *pontos, int *temChave) {
    int nl, nc;
    int dir = escolherDirecao(mat, *jL, *jC, *jDir, !(*temChave));
    if (dir == -1) return 0;
    *jDir = dir;

    proximaPosicao(*jL, *jC, *jDir, &nl, &nc);
    int dest = mat[nl][nc];

    if (dest == INIMIGO) {
        (*vidas)--;
        return (*vidas <= 0) ? -1 : 0;
    }

    if (dest == SAIDA) return 1;

    mat[*jL][*jC] = *fundoJ;
    *jL = nl;
    *jC = nc;

    if (dest == CHAVE) {
        *temChave = 1;
        *pontos  += 50;
        *fundoJ   = VAZIO;
    } else if (dest == ENERGIA) {
        *energia += 20;
        if (*energia > 100) *energia = 100;
        *pontos  += 10;
        *fundoJ   = VAZIO;
    } else if (dest == VIDA) {
        if (*vidas < 5) (*vidas)++;
        *pontos  += 15;
        *fundoJ   = VAZIO;
    } else {
        *fundoJ = dest;
        if (ehSeta(dest)) *jDir = direcaoDaSeta(dest);
    }

    mat[*jL][*jC] = JOGADOR;
    return 0;
}

/* ── movimento do inimigo ── */

void moverInimigo(int mat[LINHAS][COLUNAS],
                  int *iL, int *iC, int *iDir, int *fundoI,
                  int *vidas, int *derrota) {
    int tentativas[4], nl, nc, k, dir = -1;
    tentativas[0] = *iDir;
    tentativas[1] = direcaoHorario(*iDir);
    tentativas[2] = direcaoHorario(direcaoOposta(*iDir));
    tentativas[3] = direcaoOposta(*iDir);

    for (k = 0; k < 4; k++) {
        proximaPosicao(*iL, *iC, tentativas[k], &nl, &nc);
        if (!ehParede(mat[nl][nc])) { dir = tentativas[k]; break; }
    }
    if (dir == -1) return;
    *iDir = dir;

    proximaPosicao(*iL, *iC, *iDir, &nl, &nc);
    int dest = mat[nl][nc];

    if (dest == JOGADOR) {
        (*vidas)--;
        if (*vidas <= 0) *derrota = 1;
        return;
    }
    if (ehParede(dest)) return;

    mat[*iL][*iC] = *fundoI;
    *iL = nl;
    *iC = nc;
    *fundoI = mat[*iL][*iC];
    mat[*iL][*iC] = INIMIGO;

    if (ehSeta(*fundoI)) *iDir = direcaoDaSeta(*fundoI);
}

int main() {
    setlocale(LC_ALL, "");


    int mat[LINHAS][COLUNAS] = {    /*
     * Mapa:
     *   13=SETA_DIR(►)  12=SETA_BAIXO(▼)  14=SETA_ESQ(◄)
     *   10=VIDA(♥)  17=ENERGIA(⚡)  15=CHAVE(⚿)  16=SAIDA(▣)
     *
     *        0   1   2   3   4   5   6   7   8   9  10  11
     */
        { 6,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  7},  /* 0  borda sup  */
        { 3,  0,  0,  0,  0,  0,  0,  0,  0,  0, 17,  3},  /* 1  ⚡(1,10)   */
        { 3,  0,  5,  5,  0, 13,  0,  0,  5,  0,  0,  3},  /* 2  ►(2,5)     */
        { 3,  0,  5,  0,  0,  0,  0, 10,  0,  0,  0,  3},  /* 3  ♥(3,7)     */
        { 3,  0,  0,  0,  5,  0,  0,  0, 12,  0,  5,  3},  /* 4  ▼(4,8)     */
        { 3,  5,  0, 17,  0,  0,  5,  0,  0,  0,  0,  3},  /* 5  ⚡(5,3)    */
        { 3,  0,  0,  0,  5,  0,  5,  0,  5,  0,  0,  3},  /* 6             */
        { 3,  0,  5,  0, 14,  0,  0,  0,  0,  5,  0,  3},  /* 7  ◄(7,4)     */
        { 3,  0,  5,  5,  0,  5,  0, 17,  0,  5,  0,  3},  /* 8  ⚡(8,7)    */
        { 3, 15,  0,  0,  0,  0,  0,  0,  0,  0, 16,  3},  /* 9  ⚿(9,1) ▣(9,10) */
        { 8,  4,  4,  4,  4,  4,  4,  4,  4,  4,  4,  9}   /* 10 borda inf  */
    };

    int jL = 1, jC = 1, jDir = DIR_DIREITA, fundoJ = VAZIO;
    int iL = 9, iC = 9, iDir = DIR_ESQUERDA, fundoI = VAZIO;
    mat[jL][jC] = JOGADOR;
    mat[iL][iC] = INIMIGO;

    int vidas    = 3;
    int energia  = 100;
    int pontos   = 0;
    int temChave = 0;
    int rodada   = 0;
    int derrota  = 0;
    int resultado;

    while (1) {
        imprimeMatriz(mat);
        imprimeStatus(jL, jC, iL, iC, vidas, energia, pontos, temChave);
        sleep(1);

        energia--;
        rodada++;

        if (energia <= 0) {
            imprimeMatriz(mat);
            imprimeStatus(jL, jC, iL, iC, vidas, 0, pontos, temChave);
            wprintf(L"\nEnergia esgotada! Derrota! :(((\n");
            break;
        }

        resultado = moverJogador(mat, &jL, &jC, &jDir, &fundoJ,
                                 &vidas, &energia, &pontos, &temChave);
        if (resultado == 1) {
            imprimeMatriz(mat);
            imprimeStatus(jL, jC, iL, iC, vidas, energia, pontos, temChave);
            wprintf(L"\nO jogador encontrou a chave e chegou ate a porta! Vitoria! :)))\n");
            break;
        }
        if (resultado == -1) {
            imprimeMatriz(mat);
            imprimeStatus(jL, jC, iL, iC, vidas, energia, pontos, temChave);
            wprintf(L"\nO inimigo te alcancou e voce nao tinha vidas extras! :(((\n");
            break;
        }

        moverInimigo(mat, &iL, &iC, &iDir, &fundoI, &vidas, &derrota);
        if (derrota) {
            imprimeMatriz(mat);
            imprimeStatus(jL, jC, iL, iC, vidas, energia, pontos, temChave);
            wprintf(L"\nO inimigo te alcancou e voce nao tinha vidas extras! :(((\n");
            break;
        }
    }

    wprintf(L"Pontuacao final: %d pontos em %d rodadas\n", pontos, rodada);
    return 0;
}
