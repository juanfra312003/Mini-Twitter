#define TAMMSJ 200
#define MAXUSRS 80

#include <queue>

//Structs de mensajes entre Gestor y cliente:

/*
Antes de escribir en el pipe la solicitud que representa alguna de estas estructuras
es necesario que se escriba un entero para diferenciar las solicitudes
*/

struct Solicitud_conexion{ // ID: 0
  int solicitud; //(1: conectar, 0: desconectar
  int id_usuario;
  int pid_origen;  
};

struct Solicitud_follow{ // ID: 1
	int solicitud; //(1: follow, 0: unfollow) 
  int id_sol;
	int id_seg;
  bool success;
};

struct Tweet{ // ID: 2
	int id_origen;
	char mensaje[TAMMSJ];
};

//struct para manejo de usuarios del gestor
struct Usuario{
	bool online; //(0: offline, 1: online)
	int id;
  char pipe[20];
	std::queue<Tweet> tweets;
	int num_seguidores;
	int id_seguidores [MAXUSRS];
};

//Gestor:
  //var global: 
  int Num = 0;
  int tweets_enviados = 0;
  int tweets_recibidos = 0;
  bool acoplado; // (1: Acoplado, 0: Desacoplado)

  //var local:
  Usuario usuarios[MAXUSRS];

  //funciones:
  void iniciarUsuarios(Usuario * usuarios);
  void recibirTweet(Tweet tweet, Usuario * usuarios);
  void enviarTweet(Usuario destino, Tweet msj);
  void follow(Solicitud_follow solicitud, Usuario * usuarios);


//Cliente:
  int u_id;
  char pipe[20];

/*PREGUNTAS:
- Colas en C
- Al registrarse un Cliente con el Gestor, debe recibir todos los tweets que ya han sido enviados al usuario que se está representando y estar listo para los siguientes.
*/