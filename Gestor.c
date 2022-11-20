/* 
  PROCESO GESTOR
    ./Gestor
    -Parámetros:
      1: número de usuarios máximo
      2: relaciones en archivo de texto
      3: modo del gestor
      4: tiempo impresión
      5: nombre del pipe
    
    -Ejemplo para ejecucion
      ./Gestor -n 10 -r ArchivoEntrada.txt -m A -t 20 -p pipeGestor
*/

#include "Usuario.h"
#include "solicitud.h"
#include "colors.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

mode_t fifo_mode = S_IRUSR | S_IWUSR;

//Informacion para logica de negocio
Usuario usuarios[MAXUSRS]; //Arreglo de usuarios de la plataforma
int Num = 0; // Numero de usuarios de la platarforma
int acoplado; // 1: Acoplado, 0: Desacoplado

// Informacion a imprimir cada t tiempo
int tiempo;
int conectados = 0;
int tweets_enviados = 0;
int tweets_recibidos = 0;


// valida que los argumentos ingresados al momento de ejecutar el programa sean correctos.
// Inicializa a su vez los valores iniciales que se ingresan en los argumentos.
void validarArgumentos(int argc, char **argv, char *nom_archivo, char *nom_pipe, int *tiempo);
// Inicializa la estructura de usuarios a partir del archivo de entrada
int iniciarUsuarios(Usuario *usuarios, char *nom_archivo);
// Imprime los usuarios en su estado actual (conexion, id, seguidores, etc)
void printUsers();

void signalHandler(){
  printf(AMARILLO_T"\n----------------------------\n");
  printf("      Estadisticas     \n");
  printf("----------------------------\n");
  printf("Usuarios conectados: %d\n", conectados);
  printf("Tweets enviados: %d\n", tweets_enviados);
  printf("Tweets recidos: %d\n", tweets_recibidos);
  printf("----------------------------\n\n"RESET_COLOR);
  signal(SIGALRM, signalHandler);
  alarm(tiempo);
}

// Atiende una solicitud de conexion
void atenderConexion(Mensaje sol);
void atenderDesconexion(Mensaje mensaje);
void atenderFollow(Mensaje mensaje);
void atenderUnfollow(Mensaje mensaje); 
void atenderTweet(Mensaje mensaje); 
void enviarTweetsPendientes(int id_destino);
void atenderSolicitudTweet(Mensaje mensaje);

int main(int argc, char **argv) { 
  Mensaje mensaje;
  char nom_archivo[TAMNOMPIPE], nom_pipe[TAMNOMPIPE];
  int pipe_gestor;
  
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

  //Alarma para imprimir estadisticas del gestor
  signal(SIGALRM, signalHandler);
  alarm(tiempo);
  
  printf("\nAtendiendo solicitudes...\n\n");

  // Apertura del pipe
  if ((pipe_gestor = open(nom_pipe, O_RDONLY)) == -1) {
    perror("pipe");
    exit(0);
  };
  
  // Lectura del pipe
  printf("Empezando lectura del pipe...\n\n");
  while (1) {
    //Lectura del pipe con el que los procesos cliente mandan solicitudes al gestor
    if (read(pipe_gestor, &mensaje, sizeof(Mensaje)) > 0) {
      switch(mensaje.id){
        
        case CONEXION: //Solicitud de conexion/desconexion de un cliente
          if(mensaje.solicitud.sol_conexion.solicitud == CONNECT)
            atenderConexion(mensaje);
          else
            atenderDesconexion(mensaje);
          break;

        case SEGUIR: //Solicitud de follow/unfollow de un cliente
          if(mensaje.solicitud.sol_follow.solicitud == FOLLOW)
            atenderFollow(mensaje);
          else
            atenderUnfollow(mensaje);
          break;

        case TWEET:
          atenderTweet(mensaje);
          break;

        case SOLTWEET:
          atenderSolicitudTweet(mensaje);
      }
    }
  }
}

void validarArgumentos(int argc, char **argv, char *nom_archivo, char *nom_pipe, int *tiempo) {
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
    usuarios[i].recibidos = 0;
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
void atenderConexion(Mensaje mensaje) {
  char nom_pipe_cliente[TAMNOMPIPE];
  Solicitud_conexion sol = mensaje.solicitud.sol_conexion;
  int id_cliente = sol.id_usuario - 1;
  int fdCliente;

  strcpy(nom_pipe_cliente, sol.nom_pipe);
  //Apertura del pipe con el cliente que solicita la conexion
  if ((fdCliente = open(nom_pipe_cliente, O_WRONLY)) < 0){
    perror("Error abriendo pipe cliente");
    return;
  }

  printf("------------------------------------------------\n");
  printf("Usuario %d solicitando conexion...\n", sol.id_usuario);
  if (usuarios[id_cliente].online == 1) { // Si el usuario ya esta conectado
    sol.success = 0;
    printf("...El usuario %d ya esta conectado!\n\n", sol.id_usuario);
  } else {
    usuarios[id_cliente].online = 1;
    usuarios[id_cliente].p_id = sol.p_id;
    usuarios[id_cliente].fd = fdCliente;
    strcpy(usuarios[id_cliente].pipe, sol.nom_pipe);
    sol.success = 1;
    conectados++;
    printf("...El usuario %d se ha conectado!\n\n", sol.id_usuario);
  }
  mensaje.solicitud.sol_conexion = sol; // Arma el mensaje de respuesta para el cliente  

  printf("\tEnviando respuesta al cliente...\n");
  if(write(fdCliente, &mensaje, sizeof(Mensaje)) == -1){
    printf("...No se pudo enviar la respuesta al cliente!\n");
    return;
  }
  printf("\t...Respuesta enviada!\n");
  printf("------------------------------------------------\n\n");

  if (usuarios[id_cliente].online && acoplado)
    enviarTweetsPendientes(id_cliente);
}
void atenderDesconexion(Mensaje mensaje) {
  Solicitud_conexion sol = mensaje.solicitud.sol_conexion;
  int id_cliente = sol.id_usuario - 1;

  printf("------------------------------------------------\n");
  printf("Usuario %d solicitando desconexion...\n", sol.id_usuario);
  if (usuarios[id_cliente].online == 1) { // Si el usuario ya esta conectado
    usuarios[id_cliente].online = 0;
    close(usuarios[id_cliente].fd);
    printf("...El usuario %d se ha desconectado!\n", sol.id_usuario);
    conectados--;
  } else {
    printf("...El usuario %d no estaba conectado!\n", sol.id_usuario);
  }
  printf("------------------------------------------------\n\n");
}
void atenderFollow(Mensaje mensaje){
  Solicitud_follow sol = mensaje.solicitud.sol_follow;
  int id_solicitante = sol.id_sol, id_seguido = sol.id_seg - 1;
  int ya_sigue = 0;

  printf("------------------------------------------------\n");
  printf("Usuario %d solicitando seguir a %d...\n", id_solicitante, id_seguido+1);
  
  for(int i = 0 ; i < usuarios[id_seguido].num_seguidores ; i++){
    if(usuarios[id_seguido].id_seguidores[i] == id_solicitante) ya_sigue = 1;
  }
  if(ya_sigue){
    sol.success = 0;
    printf("...El usuario %d ya sigue a %d!\n\n", id_solicitante, id_seguido+1);
  }
  else{
    sol.success = 1;
    usuarios[id_seguido].id_seguidores[usuarios[id_seguido].num_seguidores] = id_solicitante;
    usuarios[id_seguido].num_seguidores++;
    printf("...El usuario %d ha empezado a seguir a %d!\n\n", id_solicitante, id_seguido+1);
  }
  mensaje.solicitud.sol_follow = sol;

  printf("\tEnviando respuesta al cliente...\n");
  if(write(usuarios[id_solicitante-1].fd, &mensaje, sizeof(Mensaje)) == -1){
    printf("...No se pudo enviar la respuesta al cliente!\n");
    return;
  }
  printf("\t...Respuesta enviada!\n");
  printf("------------------------------------------------\n\n");
}
void atenderUnfollow(Mensaje mensaje){
  Solicitud_follow sol = mensaje.solicitud.sol_follow;
  int id_solicitante = sol.id_sol, id_seguido = sol.id_seg - 1;
  int indice_seguidor = -1, aux;

  printf("------------------------------------------------\n");
  printf("Usuario %d solicitando dejar de seguir a %d...\n", id_solicitante, id_seguido+1);
  
  for(int i = 0 ; i < usuarios[id_seguido].num_seguidores ; i++){
    if(usuarios[id_seguido].id_seguidores[i] == id_solicitante) indice_seguidor = i;
  }
  if(indice_seguidor == -1){
    sol.success = 0;
    printf("...El usuario %d no sigue a %d!\n\n", id_solicitante, id_seguido+1);
  }
  else{
    sol.success = 1;
    for(int i = indice_seguidor; i < usuarios[id_seguido].num_seguidores-1; i++){
      usuarios[id_seguido].id_seguidores[i] = usuarios[id_seguido].id_seguidores[i+1];
    }
    usuarios[id_seguido].num_seguidores--;
    printf("...El usuario %d ha dejado de seguir a %d!\n\n", id_solicitante, id_seguido+1);
  }
  mensaje.solicitud.sol_follow = sol;

  printf("\tEnviando respuesta al cliente...\n");
  if(write(usuarios[id_solicitante-1].fd, &mensaje, sizeof(Mensaje)) == -1){
    printf("...No se pudo enviar la respuesta al cliente!\n");
    return;
  }
  printf("\t...Respuesta enviada!\n");
  printf("------------------------------------------------\n\n");
}
void atenderTweet(Mensaje mensaje){
  Tweet tweet = mensaje.solicitud.tweet;
  int id_remitente = tweet.id_origen-1, id_destino;

  printf("------------------------------------------------\n");
  printf("Tweet recibido...\n");
  printf("\tMensaje: %s\n", tweet.mensaje);
  printf("------------------------------------------------\n\n");
  for(int i = 0 ; i < usuarios[id_remitente].num_seguidores ; i++){
    id_destino = usuarios[id_remitente].id_seguidores[i] - 1;
    usuarios[id_destino].tweets_recibidos[usuarios[id_destino].recibidos] = tweet;
    usuarios[id_destino].recibidos++;
    if(acoplado && usuarios[id_destino].online) enviarTweetsPendientes(id_destino);
  }
  tweets_recibidos++;
  
}
void enviarTweetsPendientes(int id_destino){
  Mensaje mensaje;
  mensaje.id = TWEET;
  printf("------------------------------------------------\n");
  printf("Enviando Tweets pendientes a %d... \n", id_destino+1);
  for(; usuarios[id_destino].ultimo_leido  < usuarios[id_destino].recibidos ; usuarios[id_destino].ultimo_leido++){
    mensaje.solicitud.tweet = usuarios[id_destino].tweets_recibidos[usuarios[id_destino].ultimo_leido];
    if(write(usuarios[id_destino].fd, &mensaje, sizeof(Mensaje)) == -1){
      printf("...No se pudo enviar la tweet al cliente!\n");
    return;
  }
    tweets_enviados++;
  }
  printf("...Tweets enviados! \n");
  printf("------------------------------------------------\n\n");
}
void atenderSolicitudTweet(Mensaje mensaje){
  enviarTweetsPendientes(mensaje.solicitud.id_solicitud_Tweets-1);
}