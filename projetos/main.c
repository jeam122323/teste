// #include <stdio.h>
// #include <string.h>
// #include <stdlib.h>
// #include <openssl/sha.h>
// #include <openssl/evp.h>
// #include <openssl/bio.h>
// #include <openssl/buffer.h>
// #import hashlib, base64
// #define MAX_PALAVRAS 100
// #define MAX_LINHA 100

// int ler_palavras(const char* nome_arquivo, char palavras[][MAX_LINHA], int max_palavras) {
//     FILE *arquivo = fopen(nome_arquivo, "r");
//     if (!arquivo) {
//         perror("Erro ao abrir o arquivo");
//         return -1;
//     }

//     int i = 0;
//     while (i < max_palavras && fgets(palavras[i], MAX_LINHA, arquivo)) {
//         palavras[i][strcspn(palavras[i], "\n")] = '\0';  
//         i++;
//     }
//     fclose(arquivo);
//     return i;
// }

// void codificar_senha(const char *senha, char *output) {
//     unsigned char hash[SHA512_DIGEST_LENGTH];
//     SHA512((unsigned char*)senha, strlen(senha), hash);

//     BIO *bio, *b64;
//     BUF_MEM *buffer_ptr;
//     b64 = BIO_new(BIO_f_base64());
//     bio = BIO_new(BIO_s_mem());
//     bio = BIO_push(b64, bio);
//     BIO_write(bio, hash, SHA512_DIGEST_LENGTH);
//     BIO_flush(bio);
//     BIO_get_mem_ptr(bio, &buffer_ptr);

//     memcpy(output, buffer_ptr->data, buffer_ptr->length);
//     output[buffer_ptr->length] = '\0';

//     BIO_free_all(bio);
// }

// void gerar_combinacoes(char palavras[][MAX_LINHA], int num_palavras, char senha[], int indice_palavra, int tamanho_senha) {
//     if (tamanho_senha > 0) {
//         char senha_codificada[512];
//         codificar_senha(senha, senha_codificada);
//         printf("Senha: %s\nSHA-512 Codificado: %s\n\n", senha, senha_codificada);
//     }

//     if (indice_palavra >= num_palavras || tamanho_senha == 5) {
//         return;
//     }

//     for (int i = indice_palavra; i < num_palavras; i++) {
//         int comprimento = strlen(senha);
//         if (comprimento > 0) {
//             strcat(senha, " ");
//         }
//         strcat(senha, palavras[i]);
//         gerar_combinacoes(palavras, num_palavras, senha, i + 1, tamanho_senha + 1);
//         senha[comprimento] = '\0'; 
//     }
// }

// int main() {
//     char palavras[MAX_PALAVRAS][MAX_LINHA];
//     int num_palavras = ler_palavras("palavras.txt", palavras, MAX_PALAVRAS);

//     if (num_palavras == -1) {
//         return 1;
//     }

//     char senha[MAX_LINHA * 5];
//     senha[0] = '\0';

//     gerar_combinacoes(palavras, num_palavras, senha, 0, 0);

//     return 0;
// }

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <openssl/bio.h>
#include <openssl/evp.h>
#include <openssl/buffer.h>

// Função para codificar dados em Base64
char* base64_encode(const unsigned char* buffer, size_t length) {
    BIO *bio, *b64;
    BUF_MEM *buffer_ptr;

    b64 = BIO_new(BIO_f_base64());
    bio = BIO_new(BIO_s_mem());
    bio = BIO_push(b64, bio);

    BIO_set_flags(bio, BIO_FLAGS_BASE64_NO_NL); // Remove quebras de linha
    BIO_write(bio, buffer, length);
    BIO_flush(bio);
    BIO_get_mem_ptr(bio, &buffer_ptr);

    char* encoded = (char*)malloc((buffer_ptr->length + 1) * sizeof(char));
    memcpy(encoded, buffer_ptr->data, buffer_ptr->length);
    encoded[buffer_ptr->length] = '\0';

    BIO_free_all(bio);
    return encoded;
}

// Função para codificar uma senha usando SHA-512 e Base64
char* codificar_senha(const char* senha) {
    unsigned char hash[SHA512_DIGEST_LENGTH];

    // Gerar hash SHA-512
    SHA512((unsigned char*)senha, strlen(senha), hash);

    // Codificar o hash em Base64
    char* senha_codificada = base64_encode(hash, SHA512_DIGEST_LENGTH);
    return senha_codificada;
}

// Função para gerar combinações recursivamente
void gerar_combinacoes(char* palavras[], int n, int profundidade, char* combinacao_atual, FILE* saida) {
    if (profundidade == 0) {
        fprintf(saida, "%s\n", combinacao_atual);
        return;
    }

    for (int i = 0; i < n; i++) {
        char nova_combinacao[1024];
        if (strlen(combinacao_atual) > 0) {
            snprintf(nova_combinacao, sizeof(nova_combinacao), "%s %s", combinacao_atual, palavras[i]);
        } else {
            snprintf(nova_combinacao, sizeof(nova_combinacao), "%s", palavras[i]);
        }
        gerar_combinacoes(palavras, n, profundidade - 1, nova_combinacao, saida);
    }
}

// Função principal
int main() {
    // Arquivos de entrada
    FILE *arquivo_palavras = fopen("palavras.txt", "r");
    FILE *arquivo_senhas = fopen("usuarios_senhacodificadas.txt", "r");

    if (!arquivo_palavras || !arquivo_senhas) {
        fprintf(stderr, "Erro ao abrir arquivos de entrada.\n");
        return 1;
    }

    // Ler palavras do arquivo
    char* palavras[1000];
    char buffer[256];
    int num_palavras = 0;

    while (fgets(buffer, sizeof(buffer), arquivo_palavras)) {
        buffer[strcspn(buffer, "\n")] = '\0';  // Remover nova linha
        palavras[num_palavras] = strdup(buffer);
        num_palavras++;
    }
    fclose(arquivo_palavras);

    // Ler senhas codificadas
    char usuarios[1000][256];
    char senhas_codificadas[1000][256];
    int num_usuarios = 0;

    while (fscanf(arquivo_senhas, "%s %s", usuarios[num_usuarios], senhas_codificadas[num_usuarios]) == 2) {
        num_usuarios++;
    }
    fclose(arquivo_senhas);

    // Arquivos de saída
    FILE *arquivo_quebradas = fopen("senhas_quebradas.txt", "w");
    FILE *arquivo_nao_quebradas = fopen("senhas_nao_quebradas.txt", "w");

    if (!arquivo_quebradas || !arquivo_nao_quebradas) {
        fprintf(stderr, "Erro ao abrir arquivos de saída.\n");
        return 1;
    }

    // Gerar combinações e verificar senhas
    for (int profundidade = 1; profundidade <= 5; profundidade++) {
        FILE* combinacoes = tmpfile(); // Arquivo temporário para armazenar combinações
        gerar_combinacoes(palavras, num_palavras, profundidade, "", combinacoes);
        rewind(combinacoes);

        while (fgets(buffer, sizeof(buffer), combinacoes)) {
            buffer[strcspn(buffer, "\n")] = '\0'; // Remover nova linha
            char* senha_codificada = codificar_senha(buffer);

            for (int i = 0; i < num_usuarios; i++) {
                if (strcmp(senha_codificada, senhas_codificadas[i]) == 0) {
                    fprintf(arquivo_quebradas, "%s %s\n", usuarios[i], buffer);
                    strcpy(senhas_codificadas[i], ""); // Marcar como quebrada
                }
            }
            free(senha_codificada);
        }
        fclose(combinacoes);
    }

    // Registrar senhas não quebradas
    for (int i = 0; i < num_usuarios; i++) {
        if (strlen(senhas_codificadas[i]) > 0) {
            fprintf(arquivo_nao_quebradas, "%s\n", usuarios[i]);
        }
    }

    fclose(arquivo_quebradas);
    fclose(arquivo_nao_quebradas);

    // Liberar memória
    for (int i = 0; i < num_palavras; i++) {
        free(palavras[i]);
    }

    printf("Processamento concluído.\n");
    return 0;
}



