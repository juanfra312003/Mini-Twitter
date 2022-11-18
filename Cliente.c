#include "solicitud.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

// Cliente:
int u_id, fd;
char nom_pipe[TAMNOMPIPE];
char nom_pipe_gestor[TAMNOMPIPE];

mode_t fifo_mode = S_IRUSR | S_IWUSR;

void validarArgumentos(int argc, char **argv);
void enviarMensaje(Mensaje mensaje);

int main(int argc, char **argv) {
  int pipe_cliente, cuantos, pipe_gestor, creado = 0;
  Mensaje mensaje, mensaje_recibido;
  Solicitud_conexion sol;
  
  // Validacion de los argumentos
  validarArgumentos(argc, argv);

  // Creacion del pipe con el que el cliente se comunica con el gestor
  sprintf(nom_pipe, "pipeUsr%d-proc%d", u_id, getpid());
  unlink(nom_pipe);
  if (mkfifo(nom_pipe, fifo_mode) == -1) {
    perror("Cliente  mkfifo");
    exit(1);
  }

  //Se arma la solicitud de conexion
  sol.solicitud = CONNECT;
  sol.id_usuario = u_id;
  sol.p_id = getpid();
  strcpy(sol.nom_pipe, nom_pipe);

  mensaje.id = CONEXION;
  mensaje.solicitud.sol_conexion = sol;

  //Apertura del pipe_gestor
  do {
    if ((pipe_gestor = open(nom_pipe_gestor, O_WRONLY)) == -1) {
      perror("Cliente abriendo pipe de conexion con el gestor");
      printf("Se volvera a intentar despues\n");
      sleep(5);
    } else
      creado = 1;
  } while (creado == 0);
  
  if (write(pipe_gestor, &mensaje, sizeof(Mensaje)) == -1) {
    perror("Escribir pipe gestor");
    exit(1);
  }
  
  // Apertura del pipe
  if ((pipe_cliente = open(nom_pipe, O_RDONLY)) == -1) {
    perror("Apertura pipe cliente");
  };

  //Lectura del pipe
  if ((cuantos = read(pipe_cliente, &mensaje_recibido, sizeof(Mensaje))) <= 0) {
    perror("Lectura pipe cliente");
    exit(0);
  }

  //
  if (mensaje_recibido.solicitud.sol_conexion.success == 0) {
    printf("El usuario ya esta registrado\n");
    unlink(nom_pipe);
    return 0;
  } else {
    printf("Usuario registrado\n");
  };
  
  printf("Menu\n");
  
  return 0;
}

void validarArgumentos(int argc, char **argv) {
  if (argc != 5) {
    printf("USAGE: Cliente -i ID -p pipeNom\n");
    exit(1);
  }
  for (int i = 1; i < argc; i += 2) {
    // numero de usuarios
    if (strcmp(argv[i], "-i") == 0) {
      u_id = atoi(argv[i + 1]);
    }
    // pipe
    else if (strcmp(argv[i], "-p") == 0) {
      strcpy(nom_pipe_gestor, argv[i + 1]);
    }

    else {
      printf("USAGE: Cliente -i ID -p pipeNom\n");
      exit(1);
    }
  }
}