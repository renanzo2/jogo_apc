#include <stdio.h>
#include <stdlib.h>
#include <conio.h>  // Incluindo a conio.h para usar _kbhit() e _getch()
#include <time.h>
#include <string.h>
#include <windows.h> // Inclui para usar Sleep() no Windows

#define LARGURA 20
#define ALTURA 10
#define MAX_FASES 5

typedef struct {
    int x;
    int y;
} Ponto;

typedef struct {
    Ponto corpo[100];
    int comprimento;
    char direcao;
} Cobra;

typedef struct {
    int pontuacao;
    char nome[50];
} Jogador;

void limparTela() {
    system("cls");  // Limpa a tela no Windows
}

void desenharMapa(char mapa[ALTURA][LARGURA]) {
    for (int i = 0; i < ALTURA; i++) {
        for (int j = 0; j < LARGURA; j++) {
            printf("%c", mapa[i][j]);
        }
        printf("\n");
    }
}

void carregarMapa(int fase, char mapa[ALTURA][LARGURA]) {
    FILE *arquivo;
    char nomeArquivo[20];
    sprintf(nomeArquivo, "mapa_fase%d.txt", fase);

    arquivo = fopen(nomeArquivo, "r");
    if (!arquivo) {
        printf("Erro ao carregar o mapa da fase!\n");
        exit(1);
    }

    for (int i = 0; i < ALTURA; i++) {
        for (int j = 0; j < LARGURA; j++) {
            if ((mapa[i][j] = fgetc(arquivo)) == EOF) {
                printf("Erro ao ler o mapa!\n");
                exit(1);
            }
        }
        fgetc(arquivo); // Pular o caractere de nova linha
    }

    fclose(arquivo);
}

void iniciarCobra(Cobra *cobra) {
    cobra->comprimento = 1;
    cobra->corpo[0].x = LARGURA / 2;
    cobra->corpo[0].y = ALTURA / 2;
    cobra->direcao = 'D'; // Inicia indo para a direita
}

void gerarFruta(char mapa[ALTURA][LARGURA], Ponto *fruta) {
    srand(time(NULL));
    int x = rand() % (LARGURA - 2) + 1;
    int y = rand() % (ALTURA - 2) + 1;
    while (mapa[y][x] == '#') { // Evitar que a fruta apareça em um muro
        x = rand() % (LARGURA - 2) + 1;
        y = rand() % (ALTURA - 2) + 1;
    }
    fruta->x = x;
    fruta->y = y;
    mapa[y][x] = 'F'; // Representação da fruta
}

void atualizarCobra(Cobra *cobra, char direcao) {
    Ponto novaCabeca = cobra->corpo[0];

    switch (direcao) {
        case 'W': novaCabeca.y--; break; // Cima
        case 'S': novaCabeca.y++; break; // Baixo
        case 'A': novaCabeca.x--; break; // Esquerda
        case 'D': novaCabeca.x++; break; // Direita
    }

    // Deslocar o corpo da cobra
    for (int i = cobra->comprimento; i > 0; i--) {
        cobra->corpo[i] = cobra->corpo[i - 1];
    }
    cobra->corpo[0] = novaCabeca;
}

int verificarColisao(Cobra *cobra, char mapa[ALTURA][LARGURA]) {
    if (cobra->corpo[0].x <= 0 || cobra->corpo[0].x >= LARGURA - 1 || cobra->corpo[0].y <= 0 || cobra->corpo[0].y >= ALTURA - 1) {
        return 1;
    }
    for (int i = 1; i < cobra->comprimento; i++) {
        if (cobra->corpo[0].x == cobra->corpo[i].x && cobra->corpo[0].y == cobra->corpo[i].y) {
            return 1;
        }
    }
    return 0;
}

int checarFruta(Cobra *cobra, Ponto *fruta) {
    if (cobra->corpo[0].x == fruta->x && cobra->corpo[0].y == fruta->y) {
        return 1;
    }
    return 0;
}

void salvarPontuacao(Jogador jogador) {
    FILE *arquivo = fopen("pontuacoes.txt", "a");
    if (!arquivo) {
        printf("Erro ao abrir arquivo de pontuação!\n");
        return;
    }
    fprintf(arquivo, "%s %d\n", jogador.nome, jogador.pontuacao);
    fclose(arquivo);
}

void exibirEstatisticas() {
    FILE *arquivo = fopen("pontuacoes.txt", "r");
    if (!arquivo) {
        printf("Nenhuma estatística registrada!\n");
        return;
    }
    char nome[50];
    int pontuacao;
    printf("Estatísticas de Pontuação:\n");
    while (fscanf(arquivo, "%s %d", nome, &pontuacao) != EOF) {
        printf("%s - %d\n", nome, pontuacao);
    }
    fclose(arquivo);
}

void exibirMenu() {
    printf("Menu\n");
    printf("1. Nome\n");
    printf("2. Estatísticas\n");
    printf("3. Jogar\n");
    printf("Escolha uma opção: ");
}

void jogar() {
    Cobra cobra;
    char mapa[ALTURA][LARGURA];
    Ponto fruta;
    Jogador jogador;
    int fase = 1;
    int frutasComidas = 0;
    int gameOver = 0;

    // Carregar mapa da fase inicial
    carregarMapa(fase, mapa);

    printf("Digite o nome do jogador: ");
    fgets(jogador.nome, 50, stdin);
    jogador.nome[strcspn(jogador.nome, "\n")] = 0; // Remover nova linha

    // Iniciar cobra
    iniciarCobra(&cobra);

    while (!gameOver) {
        limparTela();
        gerarFruta(mapa, &fruta);

        desenharMapa(mapa);

        // Ler a direção
        if (_kbhit()) {  // Verifica se uma tecla foi pressionada
            char tecla = _getch();  // Lê a tecla pressionada
            if (tecla == 'W' || tecla == 'S' || tecla == 'A' || tecla == 'D') {
                cobra.direcao = tecla;
            }
        }

        // Atualizar a cobra
        atualizarCobra(&cobra, cobra.direcao);

        // Verificar se a cobra comeu a fruta
        if (checarFruta(&cobra, &fruta)) {
            frutasComidas++;
            cobra.comprimento++;
            mapa[fruta.y][fruta.x] = ' ';
            if (frutasComidas >= 10) {
                if (fase < MAX_FASES) {
                    fase++;
                    frutasComidas = 0;
                    carregarMapa(fase, mapa);
                } else {
                    gameOver = 1; // Vitória ao completar todas as fases
                    printf("Você venceu!\n");
                }
            }
        }

        // Verificar colisão
        if (verificarColisao(&cobra, mapa)) {
            gameOver = 1;
            printf("Game Over! Você colidiu.\n");
        }

        // Atraso para o jogo não ficar rápido demais
        Sleep(100);  // Sleep no Windows, em milissegundos
    }

    jogador.pontuacao = cobra.comprimento - 1; // Pontuação baseada no comprimento da cobra
    salvarPontuacao(jogador);
}

int main() {
    int opcao;

    while (1) {
        exibirMenu();
        scanf("%d", &opcao);
        getchar();  // Limpar buffer após leitura de scanf

        switch (opcao) {
            case 1:
                jogar();
                break;
            case 2:
                exibirEstatisticas();
                break;
            case 3:
                jogar();
                break;
            default:
                printf("Opção inválida!\n");
        }
    }

    return 0;
}
