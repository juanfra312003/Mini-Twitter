#include "solicitud.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// struct para manejo de usuarios del gestor
struct Usuario {
  bool online; //(0: offline, 1: online)
  int id;
  char pipe[TAMNOMPIPE];
  int ultimo_leido;
  Tweet tweets_recibidos[MAXTWEETS];
  int ultimo_enviado;
  Tweet tweets[MAXTWEETS];
  int num_seguidores;
  int id_seguidores[MAXUSRS];
};

// Gestor:
// var global:
int Num = 0;
int tweets_enviados = 0;
int tweets_recibidos = 0;

void iniciarUsuarios(Usuario *usuarios);
void recibirTweet(Tweet tweet, Usuario *usuarios);
void enviarTweet(Usuario destino, Tweet msj);
void follow(Solicitud_follow solicitud, Usuario *usuarios);

int main(int argc, char **argv) {
  //$ gestor -n Num -r Relaciones -m modo -t time -p pipeNom
  Usuario usuarios[MAXUSRS];
  bool acoplado; // (1: Acoplado, 0: Desacoplado)
  char *nom_archivo, *nom_pipe;
  int tiempo;
  if (argc != 11) {
    printf("USAGE: gestor -n Num -r Relaciones -m modo -t time -p pipeNom");
    return 0;
  }
  for (int i = 1; i < argc; i += 2) {

    if (strcmp(argv[i], "-n") == 0) {
      Num = atoi(argv[i + 1]);
      if (Num > MAXUSRS) {
        printf("El numero de usuarios no es soportado (Max. %d)", MAXUSRS);
        return 0;
      }
    }

    else if (strcmp(argv[i], "-r") == 0)
      strcpy(nom_archivo, argv[i + 1]);

    else if (strcmp(argv[i], "-m") == 0) {
      if (argv[i + 1][0] == 'A')
        acoplado = 1;
      else if (argv[i + 1][0] == 'D')
        acoplado = 0;
      else
        printf("Modo %c no reconocido", argv[i + 1][0]);
    }

    else if (strcmp(argv[i], "-t") == 0)
      tiempo = atoi(argv[i + 1]);

    else if (strcmp(argv[i], "-p") == 0) {
      strcpy(nom_pipe, argv[i + 1]);
    }
  }
}