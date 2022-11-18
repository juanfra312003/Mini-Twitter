#ifndef __USUARIO
#define __USUARIO
#include "solicitud.h"
// struct para manejo de usuarios del gestor, almacena los datos donde se leen los datos del Gestor.
typedef struct Usuario{
  int online; //(0: offline, 1: online)
  int id;
  char pipe[TAMNOMPIPE];
  int p_id;
  int ultimo_leido;
  int leidos;
  Tweet tweets_recibidos[MAXTWEETS];
  int ultimo_enviado;
  int enviados;
  Tweet tweets_enviados[MAXTWEETS];
  int num_seguidores;
  int id_seguidores[MAXUSRS];
} Usuario;

#endif