
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>



//Estrutura que representa cada user,nó da lista
typedef struct NoUsuario {
    char nome[51];        
    struct NoUsuario* anterior;     //ponteiro para o vizinho da esquerda 
    struct NoUsuario* proximo;      //ponteiro para o vizinho da direita 
} NoUsuario;

//remove caracteres de quebra de linha do buffer
void limpar_quebra_de_linha(char* texto) {
    int tamanho = strlen(texto);
    for(int i = 0; i < tamanho; i++) {
        if(texto[i] == '\n' || texto[i] == '\r') {
            texto[i] = '\0';
            break;
        }
    }
}

//substitui espacos em branco por _
void formatar_nome_com_sublinhado(char* nome) {
    for(int i = 0; nome[i] != '\0'; i++) {
        if(nome[i] == ' ') {
            nome[i] = '_';
        }
    }
}

//busca um user pelo nome na lista circular
//retorna o ponteiro para o no se encontrar ou NULL
NoUsuario* buscar_usuario_na_lista(NoUsuario* cabeca_da_lista, const char* nome_buscado) {
    if (cabeca_da_lista == NULL) {
        return NULL;
    }
    
    NoUsuario* no_atual = cabeca_da_lista;
    
    do {
        if (strcmp(no_atual->nome, nome_buscado) == 0) {
            return no_atual;
        }
        no_atual = no_atual->proximo;
    } while (no_atual != cabeca_da_lista);
    
    return NULL;
}


//adiciona um novo user no fim da lista
void adicionar_usuario(NoUsuario** cabeca_da_lista, const char* nome_novo_usuario, FILE* arquivo_saida) {
    // 1. Validação: verificar se o usuário já existe
    if (buscar_usuario_na_lista(*cabeca_da_lista, nome_novo_usuario) != NULL) {
        fprintf(arquivo_saida, "[FAILURE]ADD=%s\n", nome_novo_usuario);
        return;
    }

    //cria novo no
    NoUsuario* novo_no = (NoUsuario*)malloc(sizeof(NoUsuario));
    strcpy(novo_no->nome, nome_novo_usuario);
    
    //inserir na lista
    if (*cabeca_da_lista == NULL) {
   
        novo_no->anterior = novo_no;
        novo_no->proximo = novo_no;
        *cabeca_da_lista = novo_no;
    } else {

        NoUsuario* ultimo_no = (*cabeca_da_lista)->anterior;
        
        novo_no->anterior = ultimo_no;
        novo_no->proximo = *cabeca_da_lista;
        
        ultimo_no->proximo = novo_no;
        (*cabeca_da_lista)->anterior = novo_no;
    }
    
    fprintf(arquivo_saida, "[SUCCESS]ADD=%s\n", nome_novo_usuario);
}

//remove um user da lista e junta os vizinhos
void remover_usuario(NoUsuario** cabeca_da_lista, const char* nome_para_remover, FILE* arquivo_saida) {

    NoUsuario* alvo_da_remocao = buscar_usuario_na_lista(*cabeca_da_lista, nome_para_remover);
    
    if (alvo_da_remocao == NULL) {
        fprintf(arquivo_saida, "[FAILURE]REMOVE=%s\n", nome_para_remover);
        return;
    }
    
    if (alvo_da_remocao->proximo == alvo_da_remocao) {
        *cabeca_da_lista = NULL;
    } else {
        alvo_da_remocao->anterior->proximo = alvo_da_remocao->proximo;
        alvo_da_remocao->proximo->anterior = alvo_da_remocao->anterior;
        
        if (*cabeca_da_lista == alvo_da_remocao) {
            *cabeca_da_lista = alvo_da_remocao->proximo;
        }
    }
    
    free(alvo_da_remocao); 
    fprintf(arquivo_saida, "[SUCCESS]REMOVE=%s\n", nome_para_remover);
}

//mostra o vizinho anterior e o proximo de um user
void mostrar_usuario(NoUsuario* cabeca_da_lista, const char* nome_buscado, FILE* arquivo_saida) {
    NoUsuario* usuario = buscar_usuario_na_lista(cabeca_da_lista, nome_buscado);
    
    if (usuario == NULL) {
        fprintf(arquivo_saida, "[FAILURE]SHOW=?<-%s->?\n", nome_buscado);
    } else {
        fprintf(arquivo_saida, "[SUCCESS]SHOW=%s<-%s->%s\n", 
                usuario->anterior->nome, 
                usuario->nome, 
                usuario->proximo->nome);
    }
}

//le o prefixo da string, identifica o comando e chama a func correta
void processar_comando_da_linha(char* linha_lida, NoUsuario** cabeca_da_lista, FILE* arquivo_saida) {
    char nome_extraido[100] = {0}; 
    
    //add
    if (strncmp(linha_lida, "ADD ", 4) == 0) {
        strcpy(nome_extraido, linha_lida + 4);
        formatar_nome_com_sublinhado(nome_extraido);
        adicionar_usuario(cabeca_da_lista, nome_extraido, arquivo_saida);
    }
    //remove
    else if (strncmp(linha_lida, "REMOVE ", 7) == 0) {
        strcpy(nome_extraido, linha_lida + 7);
        formatar_nome_com_sublinhado(nome_extraido);
        remover_usuario(cabeca_da_lista, nome_extraido, arquivo_saida);
    }
    //show
    else if (strncmp(linha_lida, "SHOW ", 5) == 0) {
        strcpy(nome_extraido, linha_lida + 5);
        formatar_nome_com_sublinhado(nome_extraido);
        mostrar_usuario(*cabeca_da_lista, nome_extraido, arquivo_saida);
    }
}



//libera nos da lista ao finalizar o programa
void liberar_memoria_da_lista(NoUsuario** cabeca_da_lista) {
    if (*cabeca_da_lista == NULL) return;
    
    NoUsuario* no_atual = *cabeca_da_lista;
    NoUsuario* proximo_no = NULL;
    
    //quebra circulo
    (*cabeca_da_lista)->anterior->proximo = NULL; 
    
    //libera no por no
    while (no_atual != NULL) {
        proximo_no = no_atual->proximo;
        free(no_atual);
        no_atual = proximo_no;
    }
    
    *cabeca_da_lista = NULL;
}

//MAIN

int main(int argc, char* argv[]) {
    //valida passagem de argumentos
    if (argc < 3) {
        printf("Quantidade de argumentos invalida. Use: %s <entrada> <saida>\n", argv[0]);
        return 1;
    }

    //abre os arquivos
    FILE* arquivo_entrada = fopen(argv[1], "r");
    FILE* arquivo_saida = fopen(argv[2], "w");

    //validacao arquivos
    if (arquivo_entrada == NULL || arquivo_saida == NULL) {
        printf("Erro ao abrir os arquivos.\n");
        if(arquivo_entrada) fclose(arquivo_entrada);
        if(arquivo_saida) fclose(arquivo_saida);
        return 1;
    }

    //inicia a lista circular vazia
    NoUsuario* cabeca_da_lista = NULL;
    
    //buffer pra ler o texto
    char linha_lida[256];

    //iterando linha por linha ate o fim do arquivo
    while (fgets(linha_lida, sizeof(linha_lida), arquivo_entrada) != NULL) {
        limpar_quebra_de_linha(linha_lida);
        
        //ignora linhas vazias
        if (strlen(linha_lida) == 0) continue; 

        //pega linha lida e envia pra funcao
        processar_comando_da_linha(linha_lida, &cabeca_da_lista, arquivo_saida);
    }


    liberar_memoria_da_lista(&cabeca_da_lista);
    fclose(arquivo_entrada);
    fclose(arquivo_saida);
    
    return 0;
}
