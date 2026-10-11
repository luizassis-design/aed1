
#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>

#define LARGURA 1100
#define ALTURA 700
#define MAX_ELEMENTOS 10000
#define ARQUIVO_PADRAO "dados/entrada.txt"
#define NUM_TESTES 6
#define NUM_TIPOS 4
#define REPETICOES 100

#define COR_FUNDO ((Color){20, 24, 36, 255})
#define COR_PAINEL ((Color){31, 37, 53, 255})
#define COR_DESTAQUE GOLD
#define COR_ENCONTRADO LIME
#define COR_ATUAL ORANGE

typedef enum {
    PARADO,
    EXECUTANDO,
    PAUSADO,
    ENCONTRADO,
    NAO_ENCONTRADO
} EstadoBusca;

typedef enum {
    ALEATORIO,
    ORDENADO,
    INVERSO,
    REPETIDO
} TipoEntrada;

typedef struct {
    int tamanho;
    long long comparacoes;
    double tempoMs;
    int indiceEncontrado;
    int alvo;
    TipoEntrada tipo;
} ResultadoTeste;

static const int tamanhosTeste[NUM_TESTES] = {
    10, 100, 500, 1000, 5000, 10000
};

static int valores[MAX_ELEMENTOS];
static int tamanho = 100;
static int alvo = 42;
static int indiceAtual = 0;
static int indiceEncontrado = -1;
static long long comparacoesBusca = 0;
static EstadoBusca estado = PARADO;
static bool mostrarGrafico = false;
static ResultadoTeste resultados[NUM_TESTES * NUM_TIPOS];
static int totalResultados = 0;
static int velocidadeMs = 70;
static double acumuladoMs = 0.0;

static char mensagem[120] =
    "Pronto. Pressione ENTER para iniciar a busca.";

static const char *nomeTipo(TipoEntrada tipo) {
    switch (tipo) {
        case ALEATORIO: return "Aleatorio";
        case ORDENADO: return "Ordenado";
        case INVERSO: return "Inverso";
        case REPETIDO: return "Repetido";
        default: return "Desconhecido";
    }
}

static void reiniciarBusca(void) {
    indiceAtual = 0;
    indiceEncontrado = -1;
    comparacoesBusca = 0;
    acumuladoMs = 0.0;
    estado = PARADO;
}

static void gerarVetor(int n, TipoEntrada tipo) {
    if (n < 1) n = 1;
    if (n > MAX_ELEMENTOS) n = MAX_ELEMENTOS;
    tamanho = n;

    for (int i = 0; i < tamanho; i++) {
        switch (tipo) {
            case ALEATORIO:
                valores[i] = GetRandomValue(0, 9999);
                break;
            case ORDENADO:
                valores[i] = i + 1;
                break;
            case INVERSO:
                valores[i] = tamanho - i;
                break;
            case REPETIDO:
                valores[i] = 7;
                break;
        }
    }

    if (tipo == REPETIDO) alvo = 7;
    else if (tipo == ORDENADO) alvo = tamanho;
    else if (tipo == INVERSO) alvo = 1;
    else alvo = valores[tamanho / 2];

    reiniciarBusca();

    snprintf(mensagem, sizeof(mensagem), "Vetor %s gerado.",
             nomeTipo(tipo));
}

static bool carregarArquivo(const char *caminho) {
    FILE *arquivo = fopen(caminho, "r");

    if (arquivo == NULL) {
        snprintf(mensagem, sizeof(mensagem),
                 "Nao foi possivel abrir %s", caminho);
        return false;
    }

    int n = 0;

    while (n < MAX_ELEMENTOS &&
           fscanf(arquivo, "%d", &valores[n]) == 1) {
        n++;
    }

    fclose(arquivo);

    if (n == 0) {
        snprintf(mensagem, sizeof(mensagem),
                 "Arquivo vazio ou sem numeros inteiros.");
        return false;
    }

    tamanho = n;
    alvo = valores[tamanho - 1];
    reiniciarBusca();

    snprintf(mensagem, sizeof(mensagem),
             "Arquivo carregado: %d elementos. Alvo = ultimo valor.",
             tamanho);

    return true;
}

static int buscaLinear(const int *vetor, int n, int valor,
                       long long *comparacoes) {
    *comparacoes = 0;

    for (int i = 0; i < n; i++) {
        (*comparacoes)++;

        if (vetor[i] == valor) return i;
    }

    return -1;
}

static ResultadoTeste executarTeste(int n, TipoEntrada tipo) {
    ResultadoTeste r = {0};

    int *vetor = malloc((size_t)n * sizeof(int));

    if (vetor == NULL) {
        r.tamanho = n;
        r.tipo = tipo;
        r.indiceEncontrado = -1;
        return r;
    }

    for (int i = 0; i < n; i++) {
        switch (tipo) {
            case ALEATORIO:
                vetor[i] = GetRandomValue(0, 9999);
                break;
            case ORDENADO:
                vetor[i] = i + 1;
                break;
            case INVERSO:
                vetor[i] = n - i;
                break;
            case REPETIDO:
                vetor[i] = 7;
                break;
        }
    }

    int valorAlvo;

    if (tipo == REPETIDO) valorAlvo = 7;
    else if (tipo == ORDENADO) valorAlvo = n;
    else if (tipo == INVERSO) valorAlvo = 1;
    else valorAlvo = vetor[n / 2];

    long long totalComparacoes = 0;
    int indice = -1;

    clock_t inicio = clock();

    for (int repeticao = 0; repeticao < REPETICOES; repeticao++) {
        long long comps = 0;

        indice = buscaLinear(vetor, n, valorAlvo, &comps);
        totalComparacoes += comps;
    }

    clock_t fim = clock();

    r.tamanho = n;
    r.tipo = tipo;
    r.alvo = valorAlvo;
    r.comparacoes = totalComparacoes / REPETICOES;
    r.tempoMs = ((double)(fim - inicio) / CLOCKS_PER_SEC)
                * 1000.0 / REPETICOES;
    r.indiceEncontrado = indice;

    free(vetor);

    return r;
}

static void salvarResultadosCSV(void) {
    FILE *arquivo = fopen("resultados_busca_linear.csv", "w");

    if (arquivo == NULL) {
        snprintf(mensagem, sizeof(mensagem),
                 "Nao foi possivel salvar o CSV.");
        return;
    }

    fprintf(arquivo,
        "tipo,tamanho,alvo,comparacoes_medias,tempo_medio_ms,indice_encontrado\n");

    for (int i = 0; i < totalResultados; i++) {
        ResultadoTeste r = resultados[i];

        fprintf(arquivo, "%s,%d,%d,%lld,%.8f,%d\n",
                nomeTipo(r.tipo), r.tamanho, r.alvo,
                r.comparacoes, r.tempoMs, r.indiceEncontrado);
    }

    fclose(arquivo);

    snprintf(mensagem, sizeof(mensagem),
             "Experimentos concluidos. CSV salvo na pasta do programa.");
}

static void executarExperimentos(void) {
    totalResultados = 0;

    TipoEntrada tipos[NUM_TIPOS] = {
        ALEATORIO, ORDENADO, INVERSO, REPETIDO
    };

    for (int t = 0; t < NUM_TIPOS; t++) {
        for (int i = 0; i < NUM_TESTES; i++) {
            resultados[totalResultados++] =
                executarTeste(tamanhosTeste[i], tipos[t]);
        }
    }

    salvarResultadosCSV();
}

static void iniciarBusca(void) {
    reiniciarBusca();
    estado = EXECUTANDO;

    snprintf(mensagem, sizeof(mensagem), "Busca iniciada.");
}

static void executarUmPasso(void) {
    if (estado == ENCONTRADO || estado == NAO_ENCONTRADO) {
        snprintf(mensagem, sizeof(mensagem),
                 "A busca terminou. Pressione R para reiniciar.");
        return;
    }

    if (estado == PARADO) {
        indiceAtual = 0;
        indiceEncontrado = -1;
        comparacoesBusca = 0;
        estado = PAUSADO;
    }

    if (indiceAtual >= tamanho) {
        estado = NAO_ENCONTRADO;

        snprintf(mensagem, sizeof(mensagem),
                 "Alvo nao encontrado.");
        return;
    }

    comparacoesBusca++;

    if (valores[indiceAtual] == alvo) {
        indiceEncontrado = indiceAtual;
        estado = ENCONTRADO;

        snprintf(mensagem, sizeof(mensagem),
                 "Alvo encontrado no indice %d.",
                 indiceEncontrado);
    } else {
        indiceAtual++;
        estado = PAUSADO;

        if (indiceAtual >= tamanho) {
            estado = NAO_ENCONTRADO;

            snprintf(mensagem, sizeof(mensagem),
                     "Alvo nao encontrado apos verificar todos os elementos.");
        } else {
            snprintf(mensagem, sizeof(mensagem),
                     "Passo executado. Pressione N para avancar novamente.");
        }
    }
}

static void atualizarBusca(void) {
    if (estado != EXECUTANDO) return;

    acumuladoMs += GetFrameTime() * 1000.0;

    if (acumuladoMs < velocidadeMs) return;

    acumuladoMs = 0.0;

    executarUmPasso();

    if (estado == PAUSADO) estado = EXECUTANDO;
}

static void desenharVetor(void) {
    const int colunas = 50;
    const int margemX = 30;
    const int inicioY = 205;
    const int larguraCelula = (LARGURA - 2 * margemX) / colunas;
    const int alturaCelula = 25;

    int linhas = (tamanho + colunas - 1) / colunas;

    if (linhas > 16) linhas = 16;

    for (int i = 0; i < tamanho && i < colunas * linhas; i++) {
        int linha = i / colunas;
        int coluna = i % colunas;

        Rectangle celula = {
            (float)(margemX + coluna * larguraCelula),
            (float)(inicioY + linha * alturaCelula),
            (float)(larguraCelula - 2),
            (float)(alturaCelula - 2)
        };

        Color cor = (Color){65, 75, 98, 255};

        if (i == indiceAtual && estado == EXECUTANDO)
            cor = COR_ATUAL;

        if (i == indiceEncontrado)
            cor = COR_ENCONTRADO;

        DrawRectangleRec(celula, cor);

        if (tamanho <= 500) {
            char texto[16];

            snprintf(texto, sizeof(texto), "%d", valores[i]);

            DrawText(texto, (int)celula.x + 2,
                     (int)celula.y + 5, 10, RAYWHITE);
        }
    }

    if (tamanho > colunas * linhas) {
        DrawText(TextFormat(
            "Visualizando indices 0 a %d de %d elementos.",
            colunas * linhas - 1, tamanho),
            30, 620, 16, LIGHTGRAY);

        if (indiceAtual >= colunas * linhas) {
            DrawText(TextFormat(
                "Indice atual %d esta fora da area visivel.",
                indiceAtual), 30, 642, 16, ORANGE);
        }
    } else if (tamanho > 500) {
        DrawText(
            "Valores ocultos para manter a visualizacao legivel.",
            30, 620, 16, LIGHTGRAY);
    }
}

static void desenharGrafico(void) {
    DrawRectangle(20, 155, LARGURA - 40, 520, COR_PAINEL);

    DrawText("GRAFICO: NUMERO MEDIO DE COMPARACOES",
             40, 168, 22, RAYWHITE);

    if (totalResultados == 0) {
        DrawText("Pressione B para executar os experimentos.",
                 40, 220, 20, LIGHTGRAY);
        return;
    }

    int x0 = 90, y0 = 570, largura = 930, altura = 300;

    DrawLine(x0, y0, x0 + largura, y0, RAYWHITE);
    DrawLine(x0, y0, x0, y0 - altura, RAYWHITE);

    DrawText("Comparacoes", 35, 215, 16, LIGHTGRAY);
    DrawText("Tamanho do vetor", 455, 595, 16, LIGHTGRAY);

    long long maxComp = 1;

    for (int i = 0; i < totalResultados; i++) {
        if (resultados[i].comparacoes > maxComp)
            maxComp = resultados[i].comparacoes;
    }

    Color cores[NUM_TIPOS] = {
        SKYBLUE, LIME, ORANGE, PINK
    };

    for (int tipo = 0; tipo < NUM_TIPOS; tipo++) {
        for (int i = 0; i < NUM_TESTES; i++) {
            ResultadoTeste r =
                resultados[tipo * NUM_TESTES + i];

            int x = x0 + (int)(
                (double)i / (NUM_TESTES - 1) * largura);

            int h = (int)(
                (double)r.comparacoes / (double)maxComp * altura);

            DrawRectangle(x - 22 + tipo * 11,
                          y0 - h, 10, h, cores[tipo]);

            if (tipo == 0) {
                char rotulo[16];

                snprintf(rotulo, sizeof(rotulo), "%d", r.tamanho);

                DrawText(rotulo, x - 20, y0 + 8, 12, LIGHTGRAY);
            }
        }
    }

    const char *legendas[NUM_TIPOS] = {
        "Aleatorio", "Ordenado", "Inverso", "Repetido"
    };

    for (int i = 0; i < NUM_TIPOS; i++) {
        int x = 100 + i * 190;

        DrawRectangle(x, 635, 14, 14, cores[i]);
        DrawText(legendas[i], x + 22, 634, 16, RAYWHITE);
    }

    DrawText("Os dados completos, incluindo tempo medio, estao no CSV.",
             40, 657, 14, LIGHTGRAY);
}

static void desenharInterface(void) {
    ClearBackground(COR_FUNDO);

    DrawText("BUSCA LINEAR - VISUALIZADOR E ANALISE DE DESEMPENHO",
             25, 20, 25, RAYWHITE);

    DrawText(TextFormat("Alvo: %d", alvo),
             30, 65, 18, COR_DESTAQUE);

    DrawText(TextFormat("Elementos: %d", tamanho),
             170, 65, 18, LIGHTGRAY);

    DrawText(TextFormat("Comparacoes: %lld", comparacoesBusca),
             350, 65, 18, LIGHTGRAY);

    DrawText(TextFormat("Indice atual: %d", indiceAtual),
             570, 65, 18, LIGHTGRAY);

    const char *status = "Parado";
    Color corStatus = LIGHTGRAY;

    if (estado == EXECUTANDO) {
        status = "Executando";
        corStatus = COR_ATUAL;
    } else if (estado == PAUSADO) {
        status = "Pausado";
        corStatus = COR_DESTAQUE;
    } else if (estado == ENCONTRADO) {
        status = "Encontrado!";
        corStatus = COR_ENCONTRADO;
    } else if (estado == NAO_ENCONTRADO) {
        status = "Nao encontrado";
        corStatus = RED;
    }

    DrawText(TextFormat("Estado: %s", status),
             790, 65, 18, corStatus);

    DrawText(
        "ENTER iniciar | ESPACO pausar/continuar | N proximo passo | R reiniciar",
        30, 105, 17, RAYWHITE);

    DrawText(
        "1 aleatorio | 2 ordenado | 3 inverso | 4 repetido | setas mudam alvo",
        30, 130, 16, LIGHTGRAY);

    DrawText(
        "O carregar arquivo | G grafico | B experimentos | [ ] velocidade",
        30, 150, 16, LIGHTGRAY);

    DrawText(mensagem, 30, 180, 15, COR_DESTAQUE);

    if (mostrarGrafico)
        desenharGrafico();
    else
        desenharVetor();
}

int main(void) {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE);

    InitWindow(LARGURA, ALTURA, "Busca Linear - Raylib");

    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));

    gerarVetor(100, ALEATORIO);

    while (!WindowShouldClose()) {
        if (IsKeyPressed(KEY_ENTER))
            iniciarBusca();

        if (IsKeyPressed(KEY_SPACE)) {
            if (estado == EXECUTANDO) {
                estado = PAUSADO;
                snprintf(mensagem, sizeof(mensagem), "Busca pausada.");
            } else if (estado == PAUSADO) {
                estado = EXECUTANDO;
                snprintf(mensagem, sizeof(mensagem), "Busca continuada.");
            }
        }

        if (IsKeyPressed(KEY_N))
            executarUmPasso();

        if (IsKeyPressed(KEY_R)) {
            reiniciarBusca();
            snprintf(mensagem, sizeof(mensagem), "Busca reiniciada.");
        }

        if (IsKeyPressed(KEY_G))
            mostrarGrafico = !mostrarGrafico;

        if (IsKeyPressed(KEY_B)) {
            executarExperimentos();
            mostrarGrafico = true;
        }

        if (IsKeyPressed(KEY_ONE)) {
            gerarVetor(100, ALEATORIO);
            mostrarGrafico = false;
        }

        if (IsKeyPressed(KEY_TWO)) {
            gerarVetor(100, ORDENADO);
            mostrarGrafico = false;
        }

        if (IsKeyPressed(KEY_THREE)) {
            gerarVetor(100, INVERSO);
            mostrarGrafico = false;
        }

        if (IsKeyPressed(KEY_FOUR)) {
            gerarVetor(100, REPETIDO);
            mostrarGrafico = false;
        }

        if (IsKeyPressed(KEY_UP)) alvo++;
        if (IsKeyPressed(KEY_DOWN)) alvo--;

        if (IsKeyPressed(KEY_LEFT_BRACKET) && velocidadeMs > 5)
            velocidadeMs -= 5;

        if (IsKeyPressed(KEY_RIGHT_BRACKET) && velocidadeMs < 500)
            velocidadeMs += 5;

        if (IsKeyPressed(KEY_O)) {
            carregarArquivo(ARQUIVO_PADRAO);
            mostrarGrafico = false;
        }

        atualizarBusca();

        BeginDrawing();
        desenharInterface();
        EndDrawing();
    }

    CloseWindow();
    return 0;
}
