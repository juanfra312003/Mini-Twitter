/* 
  PROCESO CLIENTE
    ./Cliente
    -Parámetros:
      1: pipe nominal para comunicacion con el gestor
      2: id del usuario que se conecta
    
    -Ejemplo para ejecucion
      ./Cliente -p pipeGestor -i 1
*/

#include "solicitud.h"
#include "colors.h"
#include <fcntl.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

int u_id, fd;
char nom_pipe[TAMNOMPIPE];
char nom_pipe_gestor[TAMNOMPIPE];

//fd
int pipe_cliente, pipe_gestor;
mode_t fifo_mode = S_IRUSR | S_IWUSR;

//Funciones de hilos
void *menu();
void *leerRespuestas();

//Funciones para cumplir requisitos del programa
void enviarSolicitudConexion(int conexion); //Envia una solicitud de conexion/desconexion al gestor
void enviarSolicitudFollow(int follow, int id); //Envia una solicitud de follow/unfollow al gestor
void enviarTweet(char * txt); //Envia un tweet al gestor
void enviarSolTweets(); //Envia una solicitud para recibir los tweets en modo desacoplado

//Funciones complementarias
void validarArgumentos(int argc, char **argv);
Mensaje leerMensaje();

int main(int argc, char **argv) {
  int creado = 0, conectado, error, salida;
  Mensaje mensaje_recibido;
  pthread_t hilo_menu, hilo_respuestas;
  
  // Validacion de los argumentos
  validarArgumentos(argc, argv);

  // Creacion del pipe con el que el cliente se comunica con el gestor
  sprintf(nom_pipe, "pipeUsr%d-proc%d", u_id, getpid());
  unlink(nom_pipe);
  if (mkfifo(nom_pipe, fifo_mode) == -1) {
    perror("Cliente  mkfifo");
    exit(1);
  }

  //Apertura del pipe_gestor
  do {
    if ((pipe_gestor = open(nom_pipe_gestor, O_WRONLY)) == -1) {
      perror("Cliente abriendo pipe de conexion con el gestor");
      printf("Se volvera a intentar despues\n");
      sleep(5);
    } else
      creado = 1;
  } while (creado == 0);

  enviarSolicitudConexion(CONNECT);
  
  // Apertura del pipe
  if ((pipe_cliente = open(nom_pipe, O_RDONLY)) == -1) {
    perror("Apertura pipe cliente");
  };
  
  mensaje_recibido = leerMensaje();
  if (mensaje_recibido.solicitud.sol_conexion.success) {
    printf("Usuario registrado\n");
    conectado = 1;
  } else {
    printf("El usuario ya esta registrado\n");
    unlink(nom_pipe);
    conectado = 0;
  };

  //El proceso termina si el usuario ya esta conectado
  if(!conectado){
    unlink(nom_pipe);
    exit(0);
  }

  if(pthread_create(&hilo_menu, NULL, menu, NULL)){
    perror("Hilo menu");
  }

  if(pthread_create(&hilo_menu, NULL, leerRespuestas, NULL)){
    perror("Hilo respuestas gestor");
  }

  error = pthread_join(hilo_menu, (void **)&salida);
  error = pthread_join(hilo_respuestas, (void **)&salida);
}

void *menu(){
  int opcion, id_objetivo;
  char * tweet;
  size_t tam = TAMMSJ;
  
  while(1){
    printf(CYAN_T"------------------------------------------------\n");
    printf("            BIENVENIDO A MINI-TWITTER           \n");
    printf("------------------------------------------------\n"RESET_COLOR);
    printf("\t[1] Follow\n");
    printf("\t[2] UnFollow\n");
    printf("\t[3] Tweet\n");
    printf("\t[4] RecuperarTweets\n");
    printf("\t[5] Desconectar\n");
    printf("------------------------------------------------\n\t> ");
    if(scanf("%d", &opcion) <= 0) exit(0);
    printf("\n");
    
    switch(opcion){
      case 1: //Follow
        printf("------------------------------------------------\n");
        printf("\tIngrese el ID del usuario a seguir: ");
        if(scanf("%d", &id_objetivo) <= 0) exit(0);
        printf("------------------------------------------------\n\n");
        if(id_objetivo == u_id)
          printf("No te puedes seguir a ti mismo\n\n");
        else enviarSolicitudFollow(FOLLOW, id_objetivo);
        break;
      case 2:
        printf("------------------------------------------------\n");
        printf("\tIngrese el ID del usuario a dejar de seguir: ");
        if(scanf("%d", &id_objetivo) <= 0) exit(0);
        printf("------------------------------------------------\n\n");
        if(id_objetivo == u_id)
          printf("No te puedes dejar de seguir a ti mismo\n\n");
        else enviarSolicitudFollow(UNFOLLOW, id_objetivo);
        break;
      case 3:
        printf("------------------------------------------------\n");
        printf("Tweet:\n\t");
        getchar();
        if(getline(&tweet, &tam, stdin) <= 0) exit(0);
        enviarTweet(tweet);
        printf("------------------------------------------------\n\n");
        break;
      case 4:
        printf("------------------------------------------------\n");
        printf("...Solicitud para recibir tweets pendientes enviada!\n");
        printf("------------------------------------------------\n\n");
        enviarSolTweets();
        break;
      case 5:
        printf("Gracias por usar Mini-Twitter!\n\t\tSaliendo...\n");
        enviarSolicitudConexion(DISCONNECT);
        unlink(nom_pipe);
        exit(1);
        break;
      default:
        printf("La opcion digitada no es valida...\n");
        break;
    }
  }
}
void *leerRespuestas(){
  Mensaje mensaje;
  while(1){
    mensaje = leerMensaje();
    switch(mensaje.id){
      
      case SEGUIR: //En caso de follow/unfollow
        if(mensaje.solicitud.sol_follow.solicitud == FOLLOW){
          printf("\n------------------------------------------------\n");
          if(mensaje.solicitud.sol_follow.success){
            printf("Has empezado a seguir a %d\n", mensaje.solicitud.sol_follow.id_seg);
          }
          else{
            printf("No puedes seguir a %d porque ya lo sigues o no existe\n", mensaje.solicitud.sol_follow.id_seg);
          }
          printf("------------------------------------------------\n\t>");
        }
        else{
          printf("\n------------------------------------------------\n");
          if(mensaje.solicitud.sol_follow.success){
            printf("Has dejado de seguir a %d\n", mensaje.solicitud.sol_follow.id_seg);
          }
          else{
            printf("No puedes dejar de seguir a %d porque no lo sigues o no existe\n", mensaje.solicitud.sol_follow.id_seg);
          }
          printf("------------------------------------------------\n\t>");
        }
        break;

      case TWEET:
        printf("\n------------------------------------------------\n");
        printf("Nuevo Tweet de %d!\n", mensaje.solicitud.tweet.id_origen);
        printf("\t%s", mensaje.solicitud.tweet.mensaje);
        printf("------------------------------------------------\n\t>");
        break;
      
      default:
        break;
    }
  }
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
Mensaje leerMensaje(){
  Mensaje mensaje_recibido;
  int cuantos;
  
  //Lectura del pipe
  if ((cuantos = read(pipe_cliente, &mensaje_recibido, sizeof(Mensaje))) <= 0) {
    exit(0);
  }

  return mensaje_recibido;
}
void enviarSolicitudConexion(int conexion){
  Mensaje mensaje;
  Solicitud_conexion sol;
  
  //Se arma la solicitud de conexion
  sol.solicitud = conexion;
  sol.id_usuario = u_id;
  sol.p_id = getpid();
  strcpy(sol.nom_pipe, nom_pipe);
  mensaje.id = CONEXION;
  mensaje.solicitud.sol_conexion = sol;
  
  if (write(pipe_gestor, &mensaje, sizeof(Mensaje)) == -1) {
    perror("Escribir pipe gestor");
    exit(1);
  }
}
void enviarSolicitudFollow(int follow, int id){
  Mensaje mensaje;
  Solicitud_follow sol;
  
  //Se arma la solicitud de follow
  sol.solicitud = follow;
  sol.id_sol = u_id;
  sol.id_seg = id;
  mensaje.id = SEGUIR;
  mensaje.solicitud.sol_follow = sol;
  
  if (write(pipe_gestor, &mensaje, sizeof(Mensaje)) == -1) {
    perror("Escribir pipe gestor");
    exit(1);
  }
}
void enviarTweet(char * txt){
  Mensaje mensaje;
  Tweet tweet;
  tweet.id_origen = u_id;
  strcpy(tweet.mensaje, txt);
  mensaje.id = TWEET;
  mensaje.solicitud.tweet = tweet;

  if (write(pipe_gestor, &mensaje, sizeof(Mensaje)) == -1) {
    perror("Escribir pipe gestor");
    exit(1);
  }

  printf("...Tweet enviado!\n");
}
void enviarSolTweets(){
  Mensaje mensaje;
  mensaje.id = SOLTWEET;
  mensaje.solicitud.id_solicitud_Tweets = u_id;
  
  if (write(pipe_gestor, &mensaje, sizeof(Mensaje)) == -1) {
    perror("Escribir pipe gestor");
    exit(1);
  }
}