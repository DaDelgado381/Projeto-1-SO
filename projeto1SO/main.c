#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h> 
#include "constants.h"
#include "operations.h"
#include "parser.h"
#include <dirent.h>
#include <sys/fcntl.h>


void generate(int fp, int fpout){

  while (1) {
      unsigned int event_id, delay;
      size_t num_rows, num_columns, num_coords;
      size_t xs[MAX_RESERVATION_SIZE], ys[MAX_RESERVATION_SIZE];


      switch (get_next(fp)) {
        case CMD_CREATE:
          if (parse_create(fp, &event_id, &num_rows, &num_columns) != 0) {
            fprintf(stderr, "Invalid command. See HELP for usage\n");
            continue; 
          }
          

          if (ems_create(event_id, num_rows, num_columns)) {
            fprintf(stderr, "Failed to create event\n");
          }
          

          break;

        case CMD_RESERVE:
          num_coords = parse_reserve(fp, MAX_RESERVATION_SIZE, &event_id, xs, ys);

          if (num_coords == 0) {
            fprintf(stderr, "Invalid command. See HELP for usage\n");
            continue;
          }

          if (ems_reserve(event_id, num_coords, xs, ys)) {
            fprintf(stderr, "Failed to reserve seats\n");
          }

          break;

        case CMD_SHOW:
          if (parse_show(fp, &event_id) != 0) {
            fprintf(stderr, "Invalid command. See HELP for usage\n");
            continue;
          }

          if (ems_show(event_id, fpout)) {
            fprintf(stderr, "Failed to show event\n");
          }close(fpout);

          break;

        case CMD_LIST_EVENTS:
          if (ems_list_events(fpout)) {
            fprintf(stderr, "Failed to list events\n");
          }close(fpout);

          break;

        case CMD_WAIT:
          if (parse_wait(fp, &delay, NULL) == -1) {  // thread_id is not implemented
            fprintf(stderr, "Invalid command. See HELP for usage\n");
            continue;
          }

          if (delay > 0) {
            printf("Waiting...\n");
            ems_wait(delay);
          }

          break;

        case CMD_INVALID:
          fprintf(stderr, "Invalid command. See HELP for usage\n");
          break;

        case CMD_HELP:
          printf(
              "Available commands:\n"
              "  CREATE <event_id> <num_rows> <num_columns>\n"
              "  RESERVE <event_id> [(<x1>,<y1>) (<x2>,<y2>) ...]\n"
              "  SHOW <event_id>\n"
              "  LIST\n"
              "  WAIT <delay_ms> [thread_id]\n"  // thread_id is not implemented
              "  BARRIER\n"                      // Not implemented
              "  HELP\n");

          break;

        case CMD_BARRIER:  // Not implemented
        case CMD_EMPTY:
          break;

        case EOC:
        return;
      }
    }  
}

  


char *removerExtensao(const char *nomeArquivo) {
    // Encontra a última ocorrência do ponto (.) no nome do arquivo
    const char *ponto = strrchr(nomeArquivo, '.');

    // Se encontrou o ponto, cria uma cópia do nome do arquivo sem a extensão
    
      size_t tamanhoNomeSemExtensao = (size_t)(ponto - nomeArquivo);
      char *nomeSemExtensao = (char *)malloc(tamanhoNomeSemExtensao + 1); // +1 para o caractere nulo
      strncpy(nomeSemExtensao, nomeArquivo, tamanhoNomeSemExtensao);
      nomeSemExtensao[tamanhoNomeSemExtensao] = '\0'; // Adiciona o caractere nulo ao final
      return nomeSemExtensao;
    
}
int main(int argc, char *argv[]) {
  unsigned int state_access_delay_ms = STATE_ACCESS_DELAY_MS;
  struct dirent *dp;
 

  if (argc < 2) {
    perror("armguments to open very small");
    return 1;
  }
  if (argc > 3) {
    char *endptr;
    unsigned long int delay = strtoul(argv[2], &endptr, 10);

   if (*endptr != '\0' || delay > UINT_MAX) {
      fprintf(stderr, "Invalid delay value or value too large\n");
      return 1;
    }

    state_access_delay_ms = (unsigned int)delay;
  }

  if (ems_init(state_access_delay_ms)) {
    fprintf(stderr, "Failed to initialize EMS\n");
    return 1;
  } 
  char *dir = argv[1]; //way of dirpath
  DIR *diretorio = opendir(dir); //pointer to dir
  if (diretorio == NULL) {
    perror("opendir failed");
    return 1;
  }

  while(1){
    
    dp = readdir(diretorio);
    
    if (dp == NULL)
      break;
  
    
    if (strstr(dp->d_name, ".jobs") != 0){
      char caminhoCompleto[PATH_MAX];
      snprintf(caminhoCompleto, sizeof(caminhoCompleto), "%s/%s", dir,dp->d_name);
      char *nome= removerExtensao(dp->d_name);
      int fp = open(caminhoCompleto, O_RDONLY);
      char nomeArquivo[256];
      snprintf(nomeArquivo, sizeof(nomeArquivo), "%s.out", nome);
      int fpout = open(nomeArquivo, O_WRONLY | O_CREAT | O_TRUNC);
      
      
        if (fp == -1) {
          perror("Não foi possível abrir o arquivo ");
          return 1; // Código de erro
        }
        generate(fp, fpout);
        
        close(fp);
       
      
      }
    }
    ems_terminate();
}