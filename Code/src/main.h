#include <iostream>
#include <fstream>
#include <istream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <signal.h>


using namespace std;

int returnStatus;			// Variable para recibir el resultado de las funciones.
FILE* debug_file;
char* nombre_archivo_debug;
char* cadena_a_escribir;
time_t tiempo_actual;
struct tm* p_tiempo;
char* ano;
char* mes;
char* dia;
char* hora;
char* minuto;
char* segundo;
char* nombre_archivo_configuracion;
char* nombre_archivo_LDID;

int listener_general;	      		// socket descriptor para escuchar en el puerto 1190 desde cualquier direccion de internet

sockaddr_in server_addr_general;	// direccion IP en la cual se va a escuchar por comandos desde cualquier lado (INADDR_ANY)

sockaddr_in remoteaddr;				// Aqui se guarda la direccion de quien envio el comando que se recibe en el puerto 1890.

socklen_t addrlen;					// Esta variable se usa para almacenar el tamanno de remoteaddr, es necesaria pues se debe
									// pasar como puntero su direccion a recvfrom.
int puerto_escucha;					// El puerto donde se va a escuchar, se lee desde el archivo de configuracion, debe ser el 1890.
char* Message;						// Variable donde se lee toda la informacion que llego desde un terminal en el mensaje.
char* comando;						// Variable donde se almacena la parte del mensaje correspondiente al comando (23 primeros caracteres)
char* datos;						// Variable donde se almacena la parte del mensaje correspondiente a datos (1024 caracteres)
size_t tamanno_mensaje;				// Tamanno del mensaje recibido, incluye parte de comando y parte de datos.
size_t tamanno_datos;				// Esto me da el tamanno del texto en la parte de datos del mensaje recibido.
int comp_string;
class terminal* terminal_temporal;
int cantidad_terminales;

int socket_envio;

class terminal
{
	public:
	
	char* Nombre;
	int Status;
	struct sockaddr_in direccion;
	//int tiempo_actual;
	//time_t tiempo_ultima_actualizacion;
	struct timeval tiempo_ultima_actualizacion;
	int socket_envio;
		
	terminal();
};

class terminal** tabla_LDID;

struct sigaction sa;
struct itimerval timer;
bool semaforo;
bool retorno_timer;

//int lee_parametros(char* archivo,int* puerto_c,struct rango_addr* arreglo);
//int lee_tabla(char* archivo,class terminal** tabla_LDID);
int enviar_msg(class terminal* terminal,char* Mensaje);
int escribir_debug(char* nombre_arch,FILE* debug_file1,char* cadena,int order);
void timer_handler(int signum);
