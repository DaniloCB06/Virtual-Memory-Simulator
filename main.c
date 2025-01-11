// Joao Henrique Roseira Dib - 2210993
// Danilo da Cruz Barbosa - 2211700

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define KB 1024 // 1KB

// Estrutura para página
typedef struct {
    unsigned int frame;  // Frame da memória física
    int R;               // Bit Referenciada
    int M;               // Bit Modificada
    int valid;           // Se a página está válida
    unsigned int ultimoAcesso; // Timestamp para o LRU
} Page;

typedef struct {
    int pagina;      
    int ocupado;     
} Quadro;

unsigned int getIndicePag(unsigned int end_logico, int page_size) {
    return end_logico / page_size;
}

unsigned int calcular_qtd_paginas(unsigned int page_size_kb) {
    if (page_size_kb == 8) {
        return 1048576;
    }
    else if (page_size_kb == 32) {
        return 131072;
    }
    else {
        return -1;
    }
}

// Função principal do simulador
void sim_virtual(const char *algoritmo, const char *arq_path, int page_size_kb, int memory_size_mb) {
    int page_size = page_size_kb * KB; // Tamanho da página em bytes

    int num_frames = (memory_size_mb * KB * KB) / page_size; // Quadros disponíveis na memória física

    Quadro *quadros = (Quadro *)malloc(num_frames * sizeof(Quadro));
    // Inicializa todos os quadros como vazios
    for (int i = 0; i < num_frames; i++) {
        quadros[i].pagina = -1; // Nenhuma página carregada
        quadros[i].ocupado = 0; // Quadro está vazio
    }

    FILE *arq = fopen(arq_path, "r");
    if (!arq) {
        printf("Erro ao abrir o arquivo");
        exit(0);
    }

    Page *tab_pags;
    tab_pags = calloc(calcular_qtd_paginas(page_size_kb), sizeof(Page)); // Tabela de páginas
    unsigned int end_logico;
    char rw;
    int page_faults = 0;
    int qtd_pag_suja = 0;

    if (strcmp(algoritmo, "NRU") == 0) {
    int contador = 0;
    while (fscanf(arq, "%x %c", &end_logico, &rw) != EOF) {
        unsigned int indicePag = getIndicePag(end_logico, page_size);
        int pag_encontrada = 0;

        // Verifica se a página já está carregada
        for (int i = 0; i < num_frames; i++) {
            if (quadros[i].ocupado && quadros[i].pagina == indicePag) {
                tab_pags[indicePag].R = 1; // Página encontrada, atualiza bit R
                if (rw == 'W') {
                    tab_pags[indicePag].M = 1; // Atualiza bit M se for escrita
                }
                pag_encontrada = 1;
                break;
            }
        }

        // Caso seja um page fault
        if (!pag_encontrada) {
            page_faults++;
            int substituida = 0;

            // Procura um quadro vazio
            for (int i = 0; i < num_frames; i++) {
                if (!quadros[i].ocupado) {
                    quadros[i].pagina = indicePag;
                    quadros[i].ocupado = 1;

                    tab_pags[indicePag].frame = i;
                    tab_pags[indicePag].R = 1;
                    tab_pags[indicePag].M = (rw == 'W') ? 1 : 0;
                    tab_pags[indicePag].valid = 1;

                    substituida = 1;
                    break;
                }
            }

            // Se não encontrou um quadro vazio, aplica NRU
            if (!substituida) {
                for (int j = 0; j < 4; j++) {
                    for (int i = 0; i < num_frames; i++) {
                        int pagina_atual = quadros[i].pagina;
                        if (pagina_atual != -1 && tab_pags[pagina_atual].valid) {
                            int r = tab_pags[pagina_atual].R;
                            int m = tab_pags[pagina_atual].M;

                            if ((j == 0 && r == 0 && m == 0) || (j == 1 && r == 0 && m == 1) ||
                                (j == 2 && r == 1 && m == 0) || (j == 3 && r == 1 && m == 1)) {
                                if (tab_pags[pagina_atual].M) {
                                    qtd_pag_suja++;
                                }

                                tab_pags[pagina_atual].valid = 0;
                                quadros[i].pagina = indicePag;

                                tab_pags[indicePag].frame = i;
                                tab_pags[indicePag].R = 1;
                                tab_pags[indicePag].M = (rw == 'W') ? 1 : 0;
                                tab_pags[indicePag].valid = 1;

                                substituida = 1;
                                break;
                            }
                        }
                    }
                    if (substituida) break;
                }
            }
        }

        // Reset dos bits R periodicamente
        contador++;
        if (contador == 1000) {
            for (int i = 0; i < num_frames; i++) {
                if (quadros[i].ocupado) {
                    tab_pags[quadros[i].pagina].R = 0;
                }
            }
            contador = 0;
        }
    }
}

    
    else if (strcmp(algoritmo, "LRU") == 0) {
    unsigned int tempo = 0; // Contador global de tempo para simular o acesso

    while (fscanf(arq, "%x %c", &end_logico, &rw) != EOF) {
        unsigned int indicePag = getIndicePag(end_logico, page_size);
        int pag_encontrada = 0;

        // Verifica se a página já está carregada nos quadros
        for (int i = 0; i < num_frames; i++) {
            if (quadros[i].ocupado && quadros[i].pagina == indicePag) {
                tab_pags[indicePag].ultimoAcesso = tempo++; // Atualiza o tempo de último acesso
                pag_encontrada = 1;
                break;
            }
        }

        // Caso seja um page fault
        if (!pag_encontrada) {
            page_faults++;

            // Encontra o quadro LRU ou um quadro vazio
            int lru_index = -1;
            unsigned int lru_tempo = (unsigned int)-1;

            for (int i = 0; i < num_frames; i++) {
                if (!quadros[i].ocupado) {
                    lru_index = i; // Encontra um quadro vazio
                    break;
                }
                // Encontra o LRU se não houver quadros vazios
                int pagina_no_quadro = quadros[i].pagina;
                if (tab_pags[pagina_no_quadro].ultimoAcesso < lru_tempo) {
                    lru_tempo = tab_pags[pagina_no_quadro].ultimoAcesso;
                    lru_index = i;
                }
            }

            // Substituir a página no quadro encontrado (vazio ou LRU)
            if (quadros[lru_index].ocupado) {
                // Página no quadro será substituída
                int pagina_substituida = quadros[lru_index].pagina;

                if (tab_pags[pagina_substituida].M) {
                    qtd_pag_suja++; // Incrementa se a página substituída estava suja
                }
                tab_pags[pagina_substituida].valid = 0; // Marca como inválida na tabela
            }

            // Atualiza o quadro com a nova página
            quadros[lru_index].pagina = indicePag;
            quadros[lru_index].ocupado = 1;

            // Atualiza a tabela de páginas
            tab_pags[indicePag].frame = lru_index;
            tab_pags[indicePag].R = 1;
            tab_pags[indicePag].M = (rw == 'W') ? 1 : 0;
            tab_pags[indicePag].valid = 1;
            tab_pags[indicePag].ultimoAcesso = tempo++;
        }
    }
}
    
    else if (strcmp(algoritmo, "SEG") == 0) {
        unsigned int ponteiro = 0; // Ponteiro para a fila circular

        while (fscanf(arq, "%x %c", &end_logico, &rw) != EOF) {
            unsigned int indicePag = getIndicePag(end_logico, page_size);
            int pag_encontrada = 0;

            // Verifica se a página já está carregada
            for (int i = 0; i < num_frames; i++) {
                if (quadros[i].ocupado && quadros[i].pagina == indicePag) {
                    // Página encontrada, atualiza o bit R
                    tab_pags[indicePag].R = 1;
                    if (rw == 'W') {
                        tab_pags[indicePag].M = 1;
                    }
                    pag_encontrada = 1;
                    break; // Sai do loop
                }
            }

            // Caso seja um page fault
            if (!pag_encontrada) {
                page_faults++;

                // Substitui página usando a fila circular e segunda chance
                while (1) {
                    int quadro_atual = ponteiro; // Aponta para o quadro atual na fila

                    // Obtém a página associada ao quadro atual
                    int pagina_atual = quadros[quadro_atual].pagina;

                    if (!quadros[quadro_atual].ocupado || tab_pags[pagina_atual].R == 0) {
                        // Página com R=0 ou quadro vazio, substituir
                        if (quadros[quadro_atual].ocupado) {
                            // Página será substituída
                            if (tab_pags[pagina_atual].M) {
                                qtd_pag_suja++; // Incrementa se a página estava suja
                            }
                            tab_pags[pagina_atual].valid = 0; // Marca como inválida na tabela
                        }

                        // Atualiza o quadro com a nova página
                        quadros[quadro_atual].pagina = indicePag;
                        quadros[quadro_atual].ocupado = 1;

                        // Atualiza a tabela de páginas
                        tab_pags[indicePag].frame = quadro_atual;
                        tab_pags[indicePag].R = 1;
                        tab_pags[indicePag].M = (rw == 'W') ? 1 : 0;
                        tab_pags[indicePag].valid = 1;

                        // Move o ponteiro circular para o próximo quadro
                        ponteiro = (ponteiro + 1) % num_frames;
                        break; // Sai do loop de substituição
                    } else {
                        // Página com R=1, dá segunda chance
                        tab_pags[pagina_atual].R = 0; // Reseta o bit R
                        ponteiro = (ponteiro + 1) % num_frames; // Move para o próximo quadro
                    }
                }
            }
        }
    }


    else if (strcmp(algoritmo, "OTM") == 0) {
        unsigned int referencias[2000000]; // Ajustar para o número máximo de referências
        unsigned int total_refs = 0;

        // Preenche a lista de referências lendo o arquivo inteiro
        rewind(arq);
        while (fscanf(arq, "%x %c", &end_logico, &rw) != EOF) {
            referencias[total_refs++] = getIndicePag(end_logico, page_size);
        }

        // Simulação do algoritmo otimo
        rewind(arq);
        unsigned int posicao_atual = 0; // Posição atual na lista de referências
        unsigned int indicePag;
        char rw;

        while (fscanf(arq, "%x %c", &end_logico, &rw) != EOF) {
            indicePag = getIndicePag(end_logico, page_size);
            int pag_encontrada = 0;

            // Verifica se a página já está carregada nos quadros
            for (int i = 0; i < num_frames; i++) {
                if (quadros[i].ocupado && quadros[i].pagina == indicePag) {
                    // Página encontrada
                    tab_pags[indicePag].R = 1;
                    if (rw == 'W') {
                        tab_pags[indicePag].M = 1;
                    }
                    pag_encontrada = 1;
                    break; // Sai do loop
                }
            }

            // Caso seja um page fault
            if (!pag_encontrada) {
                page_faults++;

                // Encontra o quadro para substituir (Ótimo)
                int substituir_index = -1;
                int max_distancia = -1;

                for (int i = 0; i < num_frames; i++) {
                    if (!quadros[i].ocupado) {
                        // Encontra quadro vazio
                        substituir_index = i;
                        break;
                    }

                    // Calcula a distância futura
                    int distancia = total_refs; // Valor máximo como base
                    for (unsigned int j = posicao_atual + 1; j < total_refs; j++) {
                        if (referencias[j] == quadros[i].pagina) {
                            distancia = j - posicao_atual;
                            break;
                        }
                    }

                    // Atualiza o quadro com a maior distância
                    if (distancia > max_distancia) {
                        max_distancia = distancia;
                        substituir_index = i;
                    }
                }

                // Substitui a página
                if (quadros[substituir_index].ocupado) {
                    // Página existente será substituída
                    int pagina_substituida = quadros[substituir_index].pagina;
                    if (tab_pags[pagina_substituida].M) {
                        qtd_pag_suja++; // Incrementa se a página estava suja
                    }
                    tab_pags[pagina_substituida].valid = 0; // Marca como inválida
                }

                // Atualiza o quadro com a nova página
                quadros[substituir_index].pagina = indicePag;
                quadros[substituir_index].ocupado = 1;

                // Atualiza a tabela de páginas
                tab_pags[indicePag].frame = substituir_index;
                tab_pags[indicePag].R = 1;
                tab_pags[indicePag].M = (rw == 'W') ? 1 : 0;
                tab_pags[indicePag].valid = 1;
            }
            posicao_atual++; // Move para a próxima referência
        }
    }

    else {
        printf("Algoritmo %s não existente no programa.\n", algoritmo);
        fclose(arq);
        free(tab_pags);
        free(quadros);
        return;
    }

    fclose(arq);
    free(tab_pags);
    free(quadros);

    printf("\n////////////////////////////////////////////\n\n");
    printf("Algoritmo: %s\n", algoritmo);
    printf("Arquivo de entrada: %s\n", arq_path);
    printf("Tamanho da página: %d KB\n", page_size_kb);
    printf("Tamanho da memória física: %d MB\n", memory_size_mb);
    printf("Número de faltas de página: %d\n", page_faults);
    printf("Número de páginas sujas escritas no disco: %d\n", qtd_pag_suja);
    printf("\n////////////////////////////////////////////\n");
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        printf("Uso: %s <algoritmo> <arquivo.log> <tamanho_pagina_kb> <tamanho_memoria_mb>\n", argv[0]);
        return 1;
    }

    // Obtém os parâmetros da linha de comando
    const char *algoritmo = argv[1];         // Algoritmo (e.g., "lru", "nru", "SegundaChance")
    const char *arquivo_log = argv[2];       // Nome do arquivo de entrada (e.g., "arquivo.log")
    int page_size_kb = atoi(argv[3]);        // Tamanho da página em KB (e.g., 8)
    int memory_size_mb = atoi(argv[4]);      // Tamanho da memória em MB (e.g., 2)

    if (page_size_kb <= 0 || memory_size_mb <= 0) {
        printf("Erro: tamanho da página e memória devem ser maiores que zero.\n");
        return 1;
    }
    sim_virtual(algoritmo, arquivo_log, page_size_kb, memory_size_mb);

    return 0;
}
