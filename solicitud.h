#ifndef __SOLICITUD
#define __SOLICITUD
//Tamanio maximo de la cadena de un tweet
#define TAMMSJ 200
//Numero maximo de usuarios de la plataforma
#define MAXUSRS 80
//Tamanio de la cadena del nombre de un pipe
#define TAMNOMPIPE 20
//Maximo de tweets enviados/recibidos por usuario
#define MAXTWEETS 200

//El ID de cada estructura representara al tipo de solictud que se esta manejando en determinado mensaje

//ID de la estructura referente a una solicitud de conexion
#define CONEXION 0
#define CONNECT 1
#define DISCONNECT 0

//ID de la estructura referente a una solicitud de seguimiento
#define SEGUIR 1
#define FOLLOW 1
#define UNFOLLOW 0

//ID de la estructura referente a una solicitud de un Tweet
#define SOLTWEET 2

//ID de la estructura referente a un tweet
#define TWEET 3


// ID: 0 : Solicitud de conexion/desconexion
typedef struct Solicitud_conexion{ 
  int solicitud; //1: conectar, 0: desconectar
  int id_usuario;
  int p_id;
  char nom_pipe[TAMNOMPIPE];
  int success;
} Solicitud_conexion;

// ID: 1 : Solicitud de follow/unfollow
typedef struct Solicitud_follow{
  int solicitud; //1: follow, 0: unfollow
  int id_sol;
  int id_seg;
  int success;
} Solicitud_follow;

// ID: 1 : Tweet
typedef struct Tweet{
  int id_origen;
  char mensaje[TAMMSJ];
} Tweet;

//Estructura generalizada referente a cualquier solicitud
typedef union Solicitud{
  Solicitud_conexion sol_conexion; // 0
  Solicitud_follow sol_follow; // 1
  Tweet tweet; // 2
  int solicitud_Tweets; // 3
} Solicitud;

//Mensaje: Unidad que sera enviada entre cliente y gestor
typedef struct Mensaje {
  int id;
  Solicitud solicitud;
} Mensaje;

#endif