// Gestor
#include "Usuario.h"
#include "solicitud.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

Usuario usuarios[MAXUSRS];
int Num = 0;  // Numero de usuarios de la platarforma
int acoplado; // 1: Acoplado, 0: Desacoplado

// Informacion a imprimir cada t tiempo
int conectados = 0;
int tweets_enviados = 0;
int tweets_recibidos = 0;

// valida que los argumentos ingresados al momento de ejecutar el programa sean
// correctos e inicializa los valores que recibe (tiempo, nombre del archivo,
// nombre del pipe nominal)
void validarArgumentos(int argc, char **argv, char *nom_archivo, char *nom_pipe,
                       int *tiempo);
// Inicializa la estructura de usuarios a partir del archivo de entrada
int iniciarUsuarios(Usuario *usuarios, char *nom_archivo);
// Imprime los usuarios en su estado actual (conexion, id, seguidores, etc)
void printUsers();
// Atiende una solicitud de conexion
void atenderConexion(Usuario *usuarios, Mensaje sol);
// Abre un pipe por su nombre
int abrirPipe(char *nom_pipe);
// Envia un mensaje a un cliente
void enviarMensaje(char *nom_pipe, Mensaje mensaje);

int main(int argc, char **argv) {
  //./gestor -n 12 -r ArchivoEntrada.txt -m A -t 5 -p pipeGestor
  Mensaje mensaje;
  char nom_archivo[TAMNOMPIPE], nom_pipe[TAMNOMPIPE];
  int tiempo, pipe_gestor;
  mode_t fifo_mode = S_IRUSR | S_IWUSR;
  // Validacion de los argumentos
  validarArgumentos(argc, argv, nom_archivo, nom_pipe, &tiempo);
  // Inicializacion de la estructura de usuarios
  if (!iniciarUsuarios(usuarios, nom_archivo))
    return 0;
  printf("\nEstructura de datos inicializada...\n");
  // Impresion de usuarios
  printf("Usuarios registrados...\n");
  printUsers();
  // Creacion del pipe con el que los clientes se comunican con el gestor
  unlink(nom_pipe);
  if (mkfifo(nom_pipe, fifo_mode) == -1) {
    perror("Gestor mkfifo");
    exit(1);
  }
  printf("\nAtendiendo solicitudes...\n\n");
  // Apertura del pipe
  if ((pipe_gestor = open(nom_pipe, O_RDONLY)) == -1) {
    perror("pipe");
    exit(0);
  };
  // Lectura del pipe
  printf("Empezando lectura del pipe...\n\n");
  while (1) {
    if (read(pipe_gestor, &mensaje, sizeof(Mensaje)) > 0) {
      if (mensaje.id == CONEXION) { // Mensaje es referente a una conexion
        if (mensaje.solicitud.sol_conexion.solicitud == CONNECT)
          atenderConexion(usuarios, mensaje);
      }
    }
  }
}

void validarArgumentos(int argc, char **argv, char *nom_archivo, char *nom_pipe,
                       int *tiempo) {
  if (argc != 11) {
    printf("USAGE: gestor -n Num -r Relaciones -m modo -t time -p pipeNom\n");
    exit(1);
  }
  for (int i = 1; i < argc; i += 2) {
    // numero de usuarios
    if (strcmp(argv[i], "-n") == 0) {
      Num = atoi(argv[i + 1]);
      if (Num > MAXUSRS) {
        printf("El numero de usuarios no es soportado (Max. %d)\n", MAXUSRS);
        exit(1);
      }
    }
    // archivo de relaciones
    else if (strcmp(argv[i], "-r") == 0)
      strcpy(nom_archivo, argv[i + 1]);
    // modo del gestor
    else if (strcmp(argv[i], "-m") == 0) {
      if (argv[i + 1][0] == 'A')
        acoplado = 1;
      else if (argv[i + 1][0] == 'D')
        acoplado = 0;
      else
        printf("Modo %c no reconocido\n", argv[i + 1][0]);
    }
    // tiempo
    else if (strcmp(argv[i], "-t") == 0)
      *tiempo = atoi(argv[i + 1]);
    // pipe
    else if (strcmp(argv[i], "-p") == 0) {
      strcpy(nom_pipe, argv[i + 1]);
    }

    else {
      printf("USAGE: gestor -n Num -r Relaciones -m modo -t time -p pipeNom\n");
      exit(1);
    }
  }
}

int iniciarUsuarios(Usuario *usuarios, char *nom_archivo) {
  char buffer[20];
  int sigue;
  FILE *fp;

  // Da valores iniciales a cada usuario
  for (int i = 0; i < Num; i++) {
    usuarios[i].online = 0;
    usuarios[i].id = i + 1;
    usuarios[i].ultimo_leido = 0;
    usuarios[i].leidos = 0;
    usuarios[i].ultimo_enviado = 0;
    usuarios[i].enviados = 0;
    usuarios[i].num_seguidores = 0;
  }
  // Apertura del archivo
  fp = fopen(nom_archivo, "r");
  if (!fp) {
    perror("Error en la lectura del archivo");
    return 0;
  }
  // Lectura del archivo y asignacion de seguidores
  for (int i = 0; i < Num; i++) {
    for (int j = 0; j < Num; j++) {
      if (fscanf(fp, "%s", buffer) < 1)
        break;
      sigue = atoi(buffer);
      if (sigue) { // Si lo leido es == 1 (sigue a i usuario)
        usuarios[j].id_seguidores[usuarios[j].num_seguidores] = i + 1;
        usuarios[j].num_seguidores++;
      }
    }
  }
  fclose(fp);
  return 1;
}

void printUsers() {
  printf(" -----------------------\n");
  for (int i = 0; i < Num; i++) {
    printf("| ID: %d \t| Online: %d\t| Seguidores: ", usuarios[i].id,
           usuarios[i].online);
    if (usuarios[i].num_seguidores == 0)
      printf("Nadie lo sigue =(");
    else
      for (int j = 0; j < usuarios[i].num_seguidores; j++)
        printf("%d, ", usuarios[i].id_seguidores[j]);
    printf("\n -----------------------\n");
  }
}

void atenderConexion(Usuario *usuarios, Mensaje mensaje) {
  char nom_pipe_cliente[TAMNOMPIPE];
  Solicitud_conexion sol = mensaje.solicitud.sol_conexion;
  int id_cliente = sol.id_usuario - 1;

  printf("Usuario %d solicitando conexion...\n", sol.id_usuario);
  if (usuarios[id_cliente].online == 1) { // Si el usuario ya esta conectado
    sol.success = 0;
    printf("...El usuario %d ya esta conectado!\n\n", sol.id_usuario);
  } else {
    usuarios[id_cliente].online = 1;
    usuarios[id_cliente].p_id = sol.p_id;
    strcpy(usuarios[id_cliente].pipe, sol.nom_pipe);
    sol.success = 1;
    printf("...El usuario %d se ha conectado!\n\n", sol.id_usuario);
  }
  mensaje.solicitud.sol_conexion = sol; // Arma el mensaje de respuesta para el cliente
  // Abre el pipe de conexion con el cliente
  enviarMensaje(sol.nom_pipe, mensaje);
}

int abrirPipe(char *nom_pipe) {
  int fd, creado = 0;
  do {
    if ((fd = open(nom_pipe, O_WRONLY)) == -1) {
      printf("\tIntentado abrir %s\n", nom_pipe);
      sleep(1);
    } else {
      printf("\tPipe abierto\n");
      creado = 1;
    }
  } while (creado == 0);
  return fd;
}

void enviarMensaje(char *nom_pipe, Mensaje mensaje) {
  printf("Enviando mensaje de respuesta al cliente...\n");
  int pipe_cliente = abrirPipe(nom_pipe);
  if (write(pipe_cliente, &mensaje, sizeof(Mensaje) == -1)) {
    perror("Escritura pipe cliente");
  }
  printf("...Mensaje enviado\n\n");
  //close(pipe_cliente);
}