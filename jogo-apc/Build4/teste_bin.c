#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <conio.h>
#include <windows.h>
#include <locale.h>

#ifndef CONSOLE_FULLSCREEN_MODE
#define CONSOLE_FULLSCREEN_MODE 1
#endif
#define WIDTH 50  // Aumentado para tela cheia
#define HEIGHT 30  // Aumentado para tela cheia
#define MAX_SNAKE_LENGTH 200
#define MAX_RECORDES 10

typedef struct {
    char nome[50];
    int pontos;
    int nivel;
} Recorde;
// Estruturas e variáveis globais
char jogador[20];
HANDLE hConsole;
int screenWidth, screenHeight;

enum {
    KEY_ESC     = 27,
    KEY_ENTER   = 13,
    ARROW_UP    = 256 + 72,
    ARROW_DOWN  = 256 + 80,
    ARROW_LEFT  = 256 + 75,
    ARROW_RIGHT = 256 + 77
};

char menus[5][1000] = {
    "Jogar cobrinha",
    "Ver recordes",
    "Creditos"
    
};

typedef struct {
    int x, y;
} Point;

typedef struct {
    Point pos;
    char symbol;
} Obstacle;

// Variáveis globais do jogo
int currentLevel = 1;
int foodToNextLevel = 5;
int foodEatenInLevel = 0;
int gameSpeed = 200;
Obstacle obstacles[50];
int numObstacles = 0;

// Protótipos das funções
void initializeConsole(void);
void generateMenu(int choice);
int get_code(void);
void mostrarGameOver(int pontos);
void jogarCobrinha(int nivel);
void verRecordes(void);
void creditos(void);
void irColunaLinha(int coluna, int linha);
void setupLevel(void);
void carregarJogo(void);
void mostrarLimitesMatriz(void);
void proximaFase(void);
void ordenarRecordes(void);


void initializeConsole() {
    // Configura console para tela cheia
    hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    HWND consoleWindow = GetConsoleWindow();
    ShowWindow(consoleWindow, SW_MAXIMIZE);
    
    // Obtém tamanho da tela
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(hConsole, &csbi);
    screenWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    screenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    
    // Esconde o cursor
    CONSOLE_CURSOR_INFO cursorInfo;
    cursorInfo.dwSize = 100;
    cursorInfo.bVisible = FALSE;
    SetConsoleCursorInfo(hConsole, &cursorInfo);
}

void irColunaLinha(int coluna, int linha) {
    // Centraliza na tela
    int centerX = (screenWidth - WIDTH) / 2;
    int centerY = (screenHeight - HEIGHT) / 2;
    SetConsoleCursorPosition(hConsole, (COORD){coluna + centerX, linha + centerY});
}

void setupLevel() {
    switch(currentLevel) {
        case 1:
            gameSpeed = 100;  // Mais rápido (era 200)
            foodToNextLevel = 4;
            for(int i = 5; i < HEIGHT-5; i++) {
                irColunaLinha(WIDTH/4, i);
                printf("#");
                irColunaLinha(3*WIDTH/4, i);
                printf("#");
            }
            break;
            
        case 2:
            gameSpeed = 85;   // Mais rápido (era 170)
            foodToNextLevel = 5;
            for(int i = 5; i < HEIGHT-5; i++) {
                // Paredes verticais principais
                irColunaLinha(WIDTH/4, i);
                printf("#");
                irColunaLinha(3*WIDTH/4, i);
                printf("#");
                
                // Obstáculos adicionais
                if(i % 3 == 0) {
                    irColunaLinha(WIDTH/2, i);
                    printf("#");
                }
                if(i % 4 == 0) {
                    irColunaLinha(WIDTH/3, i);
                    printf("#");
                    irColunaLinha(2*WIDTH/3, i);
                    printf("#");
                }
            }
            break;
            
        case 3:
            gameSpeed = 70;   // Mais rápido (era 140)
            foodToNextLevel = 6;
            for(int i = 5; i < HEIGHT-5; i++) {
                // Mais obstáculos em padrão complexo
                irColunaLinha(WIDTH/4, i);
                printf("#");
                irColunaLinha(3*WIDTH/4, i);
                printf("#");
                
                if(i % 3 == 0) {
                    irColunaLinha(WIDTH/2, i);
                    printf("#");
                    irColunaLinha(WIDTH/3, i);
                    printf("#");
                    irColunaLinha(2*WIDTH/3, i);
                    printf("#");
                }
                if(i % 2 == 0) {
                    irColunaLinha(WIDTH/6, i);
                    printf("#");
                    irColunaLinha(5*WIDTH/6, i);
                    printf("#");
                }
            }
            break;
            
        case 4:
            gameSpeed = 55;   // Mais rápido (era 110)
            foodToNextLevel = 7;
            for(int i = 5; i < HEIGHT-5; i++) {
                // Labirinto mais denso
                irColunaLinha(WIDTH/4, i);
                printf("#");
                irColunaLinha(3*WIDTH/4, i);
                printf("#");
                
                if(i % 2 == 0) {
                    irColunaLinha(WIDTH/2, i);
                    printf("#");
                    irColunaLinha(WIDTH/3, i);
                    printf("#");
                    irColunaLinha(2*WIDTH/3, i);
                    printf("#");
                }
                if(i % 3 == 0) {
                    for(int j = WIDTH/4; j < 3*WIDTH/4; j+=WIDTH/8) {
                        irColunaLinha(j, i);
                        printf("#");
                    }
                }
            }
            break;
            
        case 5:
            gameSpeed = 40;   // Mais rápido (era 80)
            foodToNextLevel = 8;
            for(int i = 5; i < HEIGHT-5; i++) {
                // Labirinto muito complexo
                for(int j = WIDTH/8; j < 7*WIDTH/8; j+=WIDTH/8) {
                    if(i % 2 == 0 || j % (WIDTH/4) == 0) {
                        irColunaLinha(j, i);
                        printf("#");
                    }
                }
            }
            break;
    }
}
void jogarCobrinha(int nivel) {
    int cobraPosicaoX[WIDTH*HEIGHT] = {};
    int cobraPosicaoY[HEIGHT*WIDTH] = {};
    int PontoPosicaoX, PontoPosicaoY;
    int i, tamanhoCobra = 3;
    int pontos = 0;
    int ch = ARROW_RIGHT;  // Inicializa com direção para direita
    int velocidade = gameSpeed;
    
    carregarJogo();
    mostrarLimitesMatriz();
    currentLevel = nivel;
    setupLevel();

    // Posição inicial da cobra
    cobraPosicaoX[0] = 3;
    cobraPosicaoY[0] = 3;
    
    // Gera primeira comida
    do {
        PontoPosicaoX = (rand() % (WIDTH-4)) + 2;
        PontoPosicaoY = (rand() % (HEIGHT-4)) + 2;
    } while(PontoPosicaoX == cobraPosicaoX[0] && PontoPosicaoY == cobraPosicaoY[0]);

    // Mensagem inicial
    irColunaLinha(WIDTH/2 - 15, HEIGHT + 2);
    printf("Pressione alguma tecla para iniciar...");
    
    irColunaLinha(cobraPosicaoX[0], cobraPosicaoY[0]);
    printf("O");
    
    getch();  // Espera primeira tecla
    
    // Limpa mensagem inicial
    irColunaLinha(WIDTH/2 - 15, HEIGHT + 2);
    printf("                                    ");
    
    while (ch != KEY_ESC) {
        // Move o corpo da cobra
        for(i = tamanhoCobra; i > 0; i--) {
            cobraPosicaoX[i] = cobraPosicaoX[i-1];
            cobraPosicaoY[i] = cobraPosicaoY[i-1];
        }

        // Apaga última posição
        irColunaLinha(cobraPosicaoX[tamanhoCobra], cobraPosicaoY[tamanhoCobra]);
        printf(" ");

        // Verifica input
        if(kbhit()) {
            int newch = get_code();
            // Evita movimento na direção oposta
            if((newch == ARROW_UP && ch != ARROW_DOWN) ||
               (newch == ARROW_DOWN && ch != ARROW_UP) ||
               (newch == ARROW_LEFT && ch != ARROW_RIGHT) ||
               (newch == ARROW_RIGHT && ch != ARROW_LEFT)) {
                ch = newch;
            }
        }

        // Move a cabeça
        switch(ch) {
            case ARROW_UP: cobraPosicaoY[0]--; break;
            case ARROW_DOWN: cobraPosicaoY[0]++; break;
            case ARROW_LEFT: cobraPosicaoX[0]--; break;
            case ARROW_RIGHT: cobraPosicaoX[0]++; break;
        }

        // Verifica colisão com bordas
        if(cobraPosicaoX[0] <= 1 || cobraPosicaoX[0] >= WIDTH-1 ||
           cobraPosicaoY[0] <= 0 || cobraPosicaoY[0] >= HEIGHT-1) {
            mostrarGameOver(pontos);
            return;
        }

        // Verifica colisão com obstáculos
        COORD position = {cobraPosicaoX[0] + (screenWidth - WIDTH) / 2, 
                         cobraPosicaoY[0] + (screenHeight - HEIGHT) / 2};
        SMALL_RECT readRegion = {position.X, position.Y, position.X, position.Y};
        CHAR_INFO charInfo;
        ReadConsoleOutput(hConsole, &charInfo, (COORD){1,1}, (COORD){0,0}, &readRegion);

        if(charInfo.Char.AsciiChar == '#') {
            mostrarGameOver(pontos);
            return;
        }

        // Verifica colisão com o próprio corpo
        for(i = 1; i < tamanhoCobra; i++) {
            if(cobraPosicaoX[0] == cobraPosicaoX[i] && cobraPosicaoY[0] == cobraPosicaoY[i]) {
                mostrarGameOver(pontos);
                return;
            }
        }

        // Verifica se pegou comida
        if(cobraPosicaoX[0] == PontoPosicaoX && cobraPosicaoY[0] == PontoPosicaoY) {
            tamanhoCobra++;
            pontos++;
            foodEatenInLevel++;
            
            // Verifica se passa de fase
            if(foodEatenInLevel >= foodToNextLevel && currentLevel < 5) {
                currentLevel++;
                foodEatenInLevel = 0;
                velocidade = gameSpeed;
                
                proximaFase();
                mostrarLimitesMatriz();
                setupLevel();
                
                // Reseta cobra
                cobraPosicaoX[0] = 3;
                cobraPosicaoY[0] = 3;
                tamanhoCobra = 3;
                
                // Limpa buffer do teclado
                while(kbhit()) getch();
                
                // Aguarda nova tecla
                irColunaLinha(WIDTH/2 - 15, HEIGHT + 2);
                printf("Pressione alguma tecla para continuar...");
                
                irColunaLinha(cobraPosicaoX[0], cobraPosicaoY[0]);
                printf("O");
                
                getch();
                
                // Limpa mensagem
                irColunaLinha(WIDTH/2 - 15, HEIGHT + 2);
                printf("                                      ");
                
                // Reinicia movimento para direita
                ch = ARROW_RIGHT;
            }
            
            // Gera nova comida
            do {
                PontoPosicaoX = (rand() % (WIDTH-4)) + 2;
                PontoPosicaoY = (rand() % (HEIGHT-4)) + 2;
            } while(PontoPosicaoX == cobraPosicaoX[0] && PontoPosicaoY == cobraPosicaoY[0]);
        }

        // Desenha cobra e comida
        irColunaLinha(cobraPosicaoX[0], cobraPosicaoY[0]);
        printf("O");
        irColunaLinha(PontoPosicaoX, PontoPosicaoY);
        printf("*");

        // Atualiza placar
        irColunaLinha(2, HEIGHT + 1);
        printf("Pontos: %d | Nivel: %d | Proximo nivel: %d/%d", 
               pontos, currentLevel, foodEatenInLevel, foodToNextLevel);

        Sleep(velocidade);
    }

    getch();
    system("cls");
    generateMenu(0);
}
void mostrarLimitesMatriz() {
    system("cls");
    // Desenha bordas superiores
    irColunaLinha(0, 0);
    printf("<");
    for (int i = 0; i < WIDTH-2; i++) printf("=");
    printf(">\n");

    // Desenha laterais
    for (int i = 1; i < HEIGHT-1; i++) {
        irColunaLinha(0, i);
        printf("||");
        irColunaLinha(WIDTH-1, i);
        printf("||");
    }

    // Desenha bordas inferiores
    irColunaLinha(0, HEIGHT-1);
    printf("<");
    for (int i = 0; i < WIDTH-2; i++) printf("=");
    printf(">\n");
}

void mostrarGameOver(int pontos) {
    // Salva o recorde no arquivo binário
    FILE *arquivo = fopen("recordes.dat", "ab"); // "ab" para anexar em binário
    if (arquivo != NULL) {
        Recorde novoRecorde;
        strcpy(novoRecorde.nome, jogador);  // Assume que 'jogador' é uma variável global com o nome do jogador
        novoRecorde.pontos = pontos;
        novoRecorde.nivel = currentLevel;  // Assume que 'currentLevel' é uma variável global com o nível atual

        fwrite(&novoRecorde, sizeof(Recorde), 1, arquivo); // Salva o novo recorde no arquivo binário
        fclose(arquivo);
    }

    // Ordena os recordes
    ordenarRecordes();

    Sleep(600);
    mostrarLimitesMatriz();
    
    // Centraliza mensagem de Game Over
    int centerX = WIDTH / 2 - 15;
    int centerY = HEIGHT / 2 - 1;
    
    irColunaLinha(centerX, centerY - 1);
    printf("==============================");
    irColunaLinha(centerX + 6, centerY);
    printf("G A M E  O V E R");
    irColunaLinha(centerX, centerY + 1);
    printf("==============================");
    
    getch();
    system("cls");
    generateMenu(0);
}

void ordenarRecordes() {
    FILE *arquivo;
    Recorde recordes[MAX_RECORDES];
    int numRecordes = 0;

    // Lê todos os recordes do arquivo binário
    arquivo = fopen("recordes.dat", "rb");
    if (arquivo != NULL) {
        while (fread(&recordes[numRecordes], sizeof(Recorde), 1, arquivo) == 1 && numRecordes < MAX_RECORDES) {
            numRecordes++;
        }
        fclose(arquivo);
    }

    // Ordena (bubble sort)
    for (int i = 0; i < numRecordes - 1; i++) {
        for (int j = 0; j < numRecordes - i - 1; j++) {
            if (recordes[j].pontos < recordes[j + 1].pontos) {
                Recorde temp = recordes[j];
                recordes[j] = recordes[j + 1];
                recordes[j + 1] = temp;
            }
        }
    }

    // Reescreve o arquivo binário ordenado
    arquivo = fopen("recordes.dat", "wb");
    if (arquivo != NULL) {
        fwrite(recordes, sizeof(Recorde), numRecordes, arquivo);
        fclose(arquivo);
    }
}



void proximaFase() {
    system("cls");
    
    // Calcula o centro da tela
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int screenWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int screenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    
    // Mensagem de nível centralizada
    char mensagem[50];
    sprintf(mensagem, "N I V E L %d!", currentLevel);
    
    // Linha superior
    char linha[] = "==============================";
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
        (COORD){(screenWidth - strlen(linha)) / 2, screenHeight / 2 - 1});
    printf("%s", linha);
    
    // Mensagem do nível
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
        (COORD){(screenWidth - strlen(mensagem)) / 2, screenHeight / 2});
    printf("%s", mensagem);
    
    // Linha inferior
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
        (COORD){(screenWidth - strlen(linha)) / 2, screenHeight / 2 + 1});
    printf("%s", linha);
    
    Sleep(1500);
    system("cls");
}
void carregarJogo() {
    system("cls");
    printf("Aguarde o jogo ser carregado...\n");
    sleep(1);
    printf("Iniciando...");
    system("cls");
}
void verRecordes() {
    system("cls");
    
    // Calcula o centro da tela
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int screenWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int screenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    
    // Título
    char titulo[] = "=== RANKING DOS JOGADORES ===";
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
        (COORD){(screenWidth - strlen(titulo)) / 2, (screenHeight - 15) / 2});
    printf("%s\n", titulo);
    
    // Lê e mostra os recordes
    FILE *arquivo = fopen("recordes.txt", "r");
    if(arquivo != NULL) {
        char nome[50];
        int pontos, nivel;
        int posicao = 1;
        
        while(fscanf(arquivo, "%s %d %d", nome, &pontos, &nivel) != EOF && posicao <= MAX_RECORDES) {
            char linha[100];
            sprintf(linha, "%d Lugar: %s - %d pontos (Nivel %d)", 
                    posicao, nome, pontos, nivel);
            
            SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
                (COORD){(screenWidth - strlen(linha)) / 2, (screenHeight - 13) / 2 + posicao * 2});
            printf("%s", linha);
            posicao++;
        }
        fclose(arquivo);
        
        // Mensagem para voltar (agora após o último recorde)
        char voltar[] = "Pressione qualquer tecla para voltar...";
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
            (COORD){(screenWidth - strlen(voltar)) / 2, (screenHeight - 13) / 2 + (posicao + 2) * 2});
        printf("%s", voltar);
        
    } else {
        char msg[] = "Nenhum recorde encontrado!";
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
            (COORD){(screenWidth - strlen(msg)) / 2, screenHeight / 2});
        printf("%s", msg);
        
        // Mensagem para voltar quando não há recordes
        char voltar[] = "Pressione qualquer tecla para voltar...";
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
            (COORD){(screenWidth - strlen(voltar)) / 2, screenHeight / 2 + 4});
        printf("%s", voltar);
    }
    
    getch();
    system("cls");
    generateMenu(1);
}

void creditos() {
    system("cls");
    
    // Calcula o centro da tela
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int screenWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int screenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    
    // Título dos créditos
    char titulo[] = "=== CREDITOS ===";
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
        (COORD){(screenWidth - strlen(titulo)) / 2, (screenHeight - 10) / 2});
    printf("%s\n", titulo);
    
    // Array com os nomes
    char *nomes[] = {
        "Pedro Miguel De Lima Oliveira",
        "Renan Hiroshi Gomes Watanabe",
        "Victor Caldas Nery",
        "Vitor Augusto Cunha de Sousa"
    };
    
    // Imprime cada nome centralizado
    for(int i = 0; i < 4; i++) {
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
            (COORD){(screenWidth - strlen(nomes[i])) / 2, (screenHeight - 6) / 2 + i * 2});
        printf("%s", nomes[i]);
    }
    
    // Instrução para voltar
    char voltar[] = "Pressione qualquer tecla para voltar...";
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
        (COORD){(screenWidth - strlen(voltar)) / 2, (screenHeight + 6) / 2});
    printf("%s", voltar);
    
    getch();
    system("cls");
    generateMenu(2);  // Volta para o menu na opção de créditos
}

int get_code() {
    int ch = getch();
    if (ch == 0 || ch == 224)
        ch = 256 + getch();
    return ch;
}

void generateMenu(int choice) {
    system("cls");
    
    // Calcula o centro real da tela
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi);
    int screenWidth = csbi.srWindow.Right - csbi.srWindow.Left + 1;
    int screenHeight = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    
    // Título do jogo
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
        (COORD){(screenWidth - 40) / 2, (screenHeight - 15) / 2});
    printf("||=====================================||");
    
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
        (COORD){(screenWidth - 40) / 2, (screenHeight - 15) / 2 + 1});
    printf("||          JOGO DA COBRINHA           ||");
    
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
        (COORD){(screenWidth - 40) / 2, (screenHeight - 15) / 2 + 2});
    printf("||=====================================||");

    // Opções do menu centralizadas
    for (int count = 0; count < 3; count++) {
        int textLength = strlen(menus[count]);
        if(choice == count) {
            textLength += 4; // Adiciona o tamanho dos símbolos [ ]
        } else {
            textLength += 6; // Adiciona o espaço das margens
        }
        
        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
            (COORD){(screenWidth - textLength) / 2, (screenHeight - 8) / 2 + count * 2});
            
        if(choice == count) {
            printf("[ %s ]", menus[count]);
        } else {
            printf("   %s   ", menus[count]);
        }
    }

    // Instruções
    char instrucoes[] = "Use as setas cima e baixo para selecionar e ENTER para confirmar";
    int instrLength = strlen(instrucoes);
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
        (COORD){(screenWidth - instrLength) / 2, (screenHeight + 8) / 2});
    printf("%s", instrucoes);

    int ch;
    while ((ch = get_code()) != KEY_ESC) {
        switch (ch) {
            case ARROW_UP:
                if(choice > 0) {
                    choice = choice - 1;
                    generateMenu(choice);
                }
                break;
            case ARROW_DOWN:
                if(choice < 3) {
                    choice = choice + 1;
                    generateMenu(choice);
                }
                break;
            case KEY_ENTER:
                switch(choice) {
                    case 0:
                        SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), 
                            (COORD){(screenWidth - 30) / 2, (screenHeight + 4) / 2});
                        printf("Digite seu nome: ");
                        scanf(" %[^\n]s", jogador);
                        jogarCobrinha(1);
                        break;
                    case 1:
                        verRecordes();
                        break;
                    case 2:
                        creditos();
                        break;
                    }
                break;
        }
    }
}
int main() {
    setlocale(LC_ALL, "Portuguese");
    srand(time(NULL));
    initializeConsole();
    generateMenu(0);
    return 0;
}