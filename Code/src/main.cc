
#include <iostream>
#include "main.h"

//************************************************************************************************************************************************
//	Metodos de la clase terminal
//************************************************************************************************************************************************

terminal::terminal()
{
//*******************************************************************************************************************
// Constructor
//*******************************************************************************************************************

	Nombre = (char*) calloc (1,51);
	Status = 1;
	bzero(&direccion, sizeof(struct sockaddr_in));
	direccion.sin_family = AF_INET;
	//tiempo_actual = 0;
	bzero(&tiempo_ultima_actualizacion, sizeof(struct timeval));
	socket_envio = 0;
	/*socket_envio = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (socket_envio == -1) 
	{
        fprintf(stdout, "Could not create a socket!\n");
		//fflush(debug_file);
		//fclose(debug_file);
    }
    else 
	{
        fprintf(stdout, "Socket created!\n");
		//fflush(debug_file);
    }
	setsockopt(socket_envio, IPPROTO_TCP, SO_REUSEADDR, (char*) &Status,sizeof(int));*/
}

int main()
{
//*******************************************************************************************************************
// Inicializaci'on del archivo debug
//*******************************************************************************************************************
	
	nombre_archivo_debug = (char*) calloc (1,100);
	ano = (char*) calloc (1,100);
	mes = (char*) calloc (1,100);
	dia = (char*) calloc (1,100);
	hora = (char*) calloc (1,100);
	minuto = (char*) calloc (1,100);
	segundo = (char*) calloc (1,100);
	cadena_a_escribir = (char*) calloc (1,4096);
	tiempo_actual = time(NULL);
	p_tiempo = localtime (&tiempo_actual);

	sprintf(ano,"%d",p_tiempo->tm_year+1900);
	sprintf(mes,"%d",p_tiempo->tm_mon+1);
	sprintf(dia,"%d",p_tiempo->tm_mday);
	sprintf(hora,"%d",p_tiempo->tm_hour);
	sprintf(minuto,"%d",p_tiempo->tm_min);
	sprintf(segundo,"%d",p_tiempo->tm_sec);
	
	strcpy(nombre_archivo_debug,"/var/log/");
	strcat(nombre_archivo_debug,ano);
	strcat(nombre_archivo_debug,"_");
	strcat(nombre_archivo_debug,mes);
	strcat(nombre_archivo_debug,"_");
	strcat(nombre_archivo_debug,dia);
	strcat(nombre_archivo_debug,"_");
	strcat(nombre_archivo_debug,hora);
	strcat(nombre_archivo_debug,":");
	strcat(nombre_archivo_debug,minuto);
	strcat(nombre_archivo_debug,":");
	strcat(nombre_archivo_debug,segundo);
	strcat(nombre_archivo_debug,"_");
	strcat(nombre_archivo_debug,"ldid.log");
	
	debug_file = fopen(nombre_archivo_debug,"a+");
	if (debug_file == NULL)
	{
  		fprintf(stderr, "error abriendo el archivo de debug de la aplicación LDID!\n");
  		return(1);
	}
	fprintf(stderr, "archivo debug de la aplicación LDID abierto correctamente!\n");
	
	free(ano);
	free(mes);
	free(dia);
	free(hora);
	free(minuto);
	free(segundo);

	cantidad_terminales = 20;
	tabla_LDID = new class terminal*[cantidad_terminales];
	
	for (int i=0;i<cantidad_terminales;i++)
	{
		tabla_LDID[i] = new class terminal;
	}
	
	listener_general = 0;
	addrlen = 0;
	puerto_escucha = 1190;
	returnStatus = 0;

	Message = (char*)malloc(224);		// 200 caracteres de datos + 23 caracteres del comando + \0.
	bzero((char*)Message, 224);
	comando = (char*)malloc(24);		// 23 caracteres del comando + espacio para el \0.
	bzero((char*)comando, 24);
	datos = (char*)malloc(201);		// 2000 caracteres de datos + espacio para el \0.
	bzero((char*)datos, 201);
	tamanno_mensaje = 0;
	tamanno_datos = 0;
	
	listener_general = socket(AF_INET, SOCK_DGRAM, 0);
    if (listener_general == -1) 
	{
		bzero((char*)cadena_a_escribir, 4096);
		strcpy(cadena_a_escribir,"Could not create a socket!\n");
		escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,0);
		fflush(debug_file);
		fclose(debug_file);
        return(1);
    }
    else 
	{
		bzero((char*)cadena_a_escribir, 4096);
		strcpy(cadena_a_escribir,"Socket created!\n");
		escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,0);
		fflush(debug_file);
	}
	// lose the pesky "address already in use" error message
	int yes=1;         // for setsockopt() SO_REUSEADDR, below
    if (setsockopt(listener_general, SOL_SOCKET, SO_REUSEADDR, &yes,sizeof(int)) == -1) 
	{
        perror("setsockopt");
        return(1);
    }
	bzero(&remoteaddr, sizeof(remoteaddr));
	bzero(&server_addr_general, sizeof(server_addr_general));
	server_addr_general.sin_family = AF_INET;
	server_addr_general.sin_addr.s_addr = htonl(INADDR_ANY);
	server_addr_general.sin_port = htons(puerto_escucha);
	
	returnStatus = bind(listener_general,(struct sockaddr*)&server_addr_general,sizeof(server_addr_general));
	if (returnStatus == 0)
	{
		bzero((char*)cadena_a_escribir, 4096);
		strcpy(cadena_a_escribir,"Bind completed!\n");
		escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,0);
		fflush(debug_file);
	}
	else 
	{
		bzero((char*)cadena_a_escribir, 4096);
		strcpy(cadena_a_escribir,"Could not bind to address!\n");
		escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,0);
		fflush(debug_file);
		fclose(debug_file);
		close(listener_general);
		return(1);
	}
	
//*******************************************************************************************************************
// Creacion del timer que interrumpe cada X segundos para enviar los mensajes de registro de direccion.
//*******************************************************************************************************************
	semaforo = true;
	retorno_timer = false;
	/* Install timer_handler as the signal handler for SIGVTALRM. */
	memset (&sa, 0, sizeof (sa));
	sa.sa_handler = &timer_handler;
	sigaction (  SIGALRM, &sa, NULL);
	
	/* Configure the timer to expire after 1 s */
	timer.it_value.tv_sec = 1;
	timer.it_value.tv_usec = 0;
	/* ... and every 1 s after that. */
	timer.it_interval.tv_sec = 1;
	timer.it_interval.tv_usec = 0;
	setitimer (ITIMER_REAL, &timer, NULL);

//*******************************************************************************************************************

	while (1)
	{
		addrlen = sizeof(remoteaddr);
		bzero((char*)Message, 224);
		returnStatus = recvfrom(listener_general, (char*)Message, 224, 0,(struct sockaddr*)&remoteaddr, &addrlen);
		if ((returnStatus == -1) && (!retorno_timer))
		{
			retorno_timer = false;
		}
		else if (returnStatus != -1)
		{
			semaforo = false;	// Si el timer interrumpe a partir de este momento no modificará las listas.
		
			char* final = Message + returnStatus - 1;
			*final = 0;
			tamanno_datos = returnStatus - 24;
			
			// Las dos líneas de arriba son para garantizar que el último caracter que está en el buffer Message es un \0
			
			bzero((char*)comando, 24);
			strncpy((char*)comando,(char*)Message,23);

			//Extraer la parte de datos del mensaje recibido.
			
			bzero((char*)datos, 201);
			strncpy((char*)datos,(char*)(Message+23),tamanno_datos);
			
			comp_string = strcmp((char*)comando,"###REGISTRO_SERVIDOR&&:\0");
			if ((comp_string == 0) && (strlen(datos) > 5))
			{
				int contador_final = 0;
				for (int i=0;i<cantidad_terminales;i++)
				{
					comp_string = strcmp((char*)tabla_LDID[i]->Nombre,"");	// Si esta entrada de la tabla no est'a vac'ia
					if (comp_string != 0)	
					{
						comp_string = strcmp((char*)datos,tabla_LDID[i]->Nombre);	// Compara el nombre del terminal con el que lleg'o
						if (comp_string == 0)										// Si es el mismo nombre entonces actualiza la direcci'on y env'ia
						{															// el ack al servidor que se registr'o
							tabla_LDID[i]->direccion = remoteaddr;
							
							gettimeofday(&tabla_LDID[i]->tiempo_ultima_actualizacion,NULL);
						
							bzero((char*)cadena_a_escribir, 4096);
							strcpy(cadena_a_escribir,"Actualizado el servidor ");
							escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
							escribir_debug(nombre_archivo_debug,debug_file,(char*)tabla_LDID[i]->Nombre,2);
							bzero((char*)cadena_a_escribir, 4096);
							strcpy(cadena_a_escribir,"\n");
							escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
							fflush(debug_file);
							bzero((char*)Message, 224);
							strcpy((char*)Message,"###RECIBIDO_OK&&&&&&&&&");
							if (enviar_msg(tabla_LDID[i],(char*)Message))
							{
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"Enviado el mensaje: ###RECIBIDO_OK&&&&&&&&&  al servidor: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
								escribir_debug(nombre_archivo_debug,debug_file,tabla_LDID[i]->Nombre,2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,", cuya direccion es: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,2);
								escribir_debug(nombre_archivo_debug,debug_file,inet_ntoa(remoteaddr.sin_addr),2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"\n");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
								fflush(debug_file);
							}
							else
							{
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"No se pudo enviar el mensaje: ###RECIBIDO_OK&&&&&&&&&  al servidor: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
								escribir_debug(nombre_archivo_debug,debug_file,tabla_LDID[i]->Nombre,2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,", cuya direccion es: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,2);
								escribir_debug(nombre_archivo_debug,debug_file,inet_ntoa(remoteaddr.sin_addr),2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"\n");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
								fflush(debug_file);
							}
							break;
						}
					}
					contador_final++;
				}
				if (contador_final == 20)
				{
					for (int i=0;i<cantidad_terminales;i++)
					{
						//if (*(tabla_LDID[i]->Nombre) == NULL) // Si la entrada de la tabla est'a vac'ia
						comp_string = strcmp((char*)tabla_LDID[i]->Nombre,"");
						if (comp_string == 0)
						{
							strcpy((char*)tabla_LDID[i]->Nombre,(char*)datos);
							tabla_LDID[i]->direccion = remoteaddr;
							gettimeofday(&tabla_LDID[i]->tiempo_ultima_actualizacion,NULL);
						
							bzero((char*)cadena_a_escribir, 4096);
							strcpy(cadena_a_escribir,"Registrando el servidor ");
							escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
							escribir_debug(nombre_archivo_debug,debug_file,(char*)tabla_LDID[i]->Nombre,2);
							bzero((char*)cadena_a_escribir, 4096);
							strcpy(cadena_a_escribir,"\n");
							escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
							fflush(debug_file);							
							bzero((char*)Message, 224);
							strcpy((char*)Message,"###RECIBIDO_OK&&&&&&&&&");
							if(enviar_msg(tabla_LDID[i],(char*)Message))
							{
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"Enviado el mensaje: ###RECIBIDO_OK&&&&&&&&&  al servidor: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
								escribir_debug(nombre_archivo_debug,debug_file,tabla_LDID[i]->Nombre,2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,", cuya direccion es: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,2);
								escribir_debug(nombre_archivo_debug,debug_file,inet_ntoa(remoteaddr.sin_addr),2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"\n");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
								fflush(debug_file);
							}
							else
							{
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"no se pudo enviar el mensaje: ###RECIBIDO_OK&&&&&&&&&  al servidor: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
								escribir_debug(nombre_archivo_debug,debug_file,tabla_LDID[i]->Nombre,2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,", cuya direccion es: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,2);
								escribir_debug(nombre_archivo_debug,debug_file,inet_ntoa(remoteaddr.sin_addr),2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"\n");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
								fflush(debug_file);
							}
							break;
						}
					}
				}
			}
			else
			{
				comp_string = strcmp((char*)comando,"###SOLICITUD_DIRECCION:");
				if ((comp_string == 0) && (strlen(datos) > 5))
				{
					//for (int i=0;i<cantidad_terminales;i++)
					int contaor = 0;
					while (contaor < cantidad_terminales)
					{
						comp_string = strcmp((char*)datos,tabla_LDID[contaor]->Nombre);
						if (comp_string == 0)
						{
							terminal_temporal = new class terminal;
							bzero(terminal_temporal, sizeof(terminal_temporal));
							terminal_temporal->direccion = remoteaddr;
							bzero((char*)Message, 224);
							strcpy((char*)Message,"###DIRECCION_SERVIDOR&:");
							strcpy((char*)(Message+23),inet_ntoa(tabla_LDID[contaor]->direccion.sin_addr));
							if (enviar_msg(terminal_temporal,(char*)Message))
							{
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"Enviada la direccion: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
								escribir_debug(nombre_archivo_debug,debug_file,(char*)inet_ntoa(tabla_LDID[contaor]->direccion.sin_addr),2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir," del servidor: ");
								strcat(cadena_a_escribir,tabla_LDID[contaor]->Nombre);
								strcat(cadena_a_escribir,", al terminal con direccion: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,2);
								escribir_debug(nombre_archivo_debug,debug_file,inet_ntoa(remoteaddr.sin_addr),2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"\n");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
								fflush(debug_file);
							}
							else
							{
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"No se pudo enviar la direccion: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
								escribir_debug(nombre_archivo_debug,debug_file,(char*)inet_ntoa(tabla_LDID[contaor]->direccion.sin_addr),2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir," del servidor: ");
								strcat(cadena_a_escribir,tabla_LDID[contaor]->Nombre);
								strcat(cadena_a_escribir,", al terminal con direccion: ");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,2);
								escribir_debug(nombre_archivo_debug,debug_file,inet_ntoa(remoteaddr.sin_addr),2);
								bzero((char*)cadena_a_escribir, 4096);
								strcpy(cadena_a_escribir,"\n");
								escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
								fflush(debug_file);
							}
							delete terminal_temporal;
							break;
						}
						contaor++;
					}
					if (contaor == cantidad_terminales)
					{
						terminal_temporal = new class terminal;
						bzero(terminal_temporal, sizeof(terminal_temporal));
						terminal_temporal->direccion = remoteaddr;
						bzero((char*)Message, 224);
						strcpy((char*)Message,"###SERV_NO_REGISTRADO&&");
						strcpy((char*)(Message+23),(char*)datos);
						if (enviar_msg(terminal_temporal,(char*)Message))
						{
							bzero((char*)cadena_a_escribir, 4096);
							strcpy(cadena_a_escribir,"Enviado el mensaje de que el servidor: ");
							escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
							escribir_debug(nombre_archivo_debug,debug_file,(char*)datos,2);
							bzero((char*)cadena_a_escribir, 4096);
							strcpy(cadena_a_escribir," no esta registrado, al terminal con direccion: ");
							escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,2);
							escribir_debug(nombre_archivo_debug,debug_file,inet_ntoa(remoteaddr.sin_addr),2);
							bzero((char*)cadena_a_escribir, 4096);
							strcpy(cadena_a_escribir,"\n");
							escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
							fflush(debug_file);
						}
						else
						{
							bzero((char*)cadena_a_escribir, 4096);
							strcpy(cadena_a_escribir,"No se pudo enviar el mensaje de que el servidor: ");
							escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
							escribir_debug(nombre_archivo_debug,debug_file,(char*)datos,2);
							bzero((char*)cadena_a_escribir, 4096);
							strcpy(cadena_a_escribir," no esta registrado, al terminal con direccion: ");
							escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,2);
							escribir_debug(nombre_archivo_debug,debug_file,inet_ntoa(remoteaddr.sin_addr),2);
							bzero((char*)cadena_a_escribir, 4096);
							strcpy(cadena_a_escribir,"\n");
							escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
							fflush(debug_file);
						}
						
						delete terminal_temporal;
						//break;
					}	
				}
			}
		}
		semaforo = true;
	}
	fflush(debug_file);
	fclose(debug_file);
	return 0;
}

void timer_handler(int signum)
{
//******************************************************************************************************************
// Esta funci'on interrumpe cada x segundos para chequear que no haya pasado el timeout de desregistro de los servidores
// de aplicaci'on, si un servidor no se actualiza en 10 minutos su entrada en la tabla se invalida. 
//******************************************************************************************************************

	if (semaforo)	// El sem'aforo debe estar en verde para poder entrar en esta funcion
	{
		int comp_string_timer;
		struct timeval tiempo_actual_timer;
		gettimeofday(&tiempo_actual_timer,NULL);
		struct timeval elapsed_timer;
		
		for (int i=0;i<cantidad_terminales;i++)
		{
			comp_string_timer = strcmp((char*)tabla_LDID[i]->Nombre,"");	// Si esta entrada de la tabla no est'a vac'ia
			if (comp_string_timer != 0)	
			{
				elapsed_timer.tv_sec = (tiempo_actual_timer.tv_sec - tabla_LDID[i]->tiempo_ultima_actualizacion.tv_sec);
				if (elapsed_timer.tv_sec > 600)	// timeout en 10 minutos.
				{
					// el servidor previamente registrado no da sennales de vida, por lo tanto darle el bate.
					
					bzero((char*)cadena_a_escribir, 4096);
					strcpy(cadena_a_escribir,"Desregistrando el servidor: ");
					escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,1);
					escribir_debug(nombre_archivo_debug,debug_file,(char*)tabla_LDID[i]->Nombre,2);
					bzero((char*)cadena_a_escribir, 4096);
					strcpy(cadena_a_escribir,"\n");
					escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,3);
					fflush(debug_file);
					strcpy((char*)tabla_LDID[i]->Nombre,"");
					bzero(&(tabla_LDID[i]->direccion), sizeof(tabla_LDID[i]->direccion));
					bzero(&tabla_LDID[i]->tiempo_ultima_actualizacion, sizeof(struct timeval));
				}
			}
		}
		retorno_timer = true;
	}
}

int enviar_msg(class terminal* terminal,char* Mensaje)
{
//******************************************************************************************************************
// Funcion de envio de mensaje a un terminal conectado, si se envia el mensaje correctamente devuelve 1 sino devuelve 0
//******************************************************************************************************************
	
	int returnStatus = 0;
	int retorno;
	
	returnStatus = sendto(listener_general, (char*)Mensaje, strlen((char*)Mensaje)+1
						  , 0,(struct sockaddr*)&terminal->direccion,sizeof((struct sockaddr_in)terminal->direccion));
	if (returnStatus == -1)
	{
		retorno = 0;
	}
	else
	{
		retorno = 1;
	}
	return(retorno);
}

int escribir_debug(char* nombre_arch,FILE* debug_file1,char* cadena,int order)
{

	if (order == 0 || order == 1 || order == 3)	// order 0 es que es mensaje unico (lleva encabezamiento de fecha y adem'as se puede comenzar un nuevo archivo log con el)
												// order 1 es que es el primer mensaje de un grupo (lleva encabezamiento de fecha pero no se puede comenzar un nuevo archivo)
												// order 2 es que es parte intermedia de un grupo de mensajes (no lleva encabezamiento de fecha ni se puede comenzar un nuevo archivo)
												// order 3 es que es el 'ultimo mensaje de un grupo (no lleva encabezamiento de fecha pero se puede comenzar un nuevo archivo)
	{
		char* ano_new = (char*) calloc(100,1);
		char* mes_new = (char*) calloc(100,1);
		char* dia_new = (char*) calloc(100,1);
		char* hora_new = (char*) calloc(100,1);
		char* minuto_new = (char*) calloc(100,1);
		char* segundo_new = (char*) calloc(100,1);
		char* cadena_a_escribir_new = (char*) calloc(100,1);
		
		time_t tiempo_actual_new;
		struct tm* p_tiempo_new;
		struct stat* debug_attributes = new struct stat;
	
		tiempo_actual_new = time(NULL);
		p_tiempo_new = localtime (&tiempo_actual_new);
		
		sprintf(ano_new,"%d",p_tiempo_new->tm_year+1900);
		sprintf(mes_new,"%d",p_tiempo_new->tm_mon+1);
		sprintf(dia_new,"%d",p_tiempo_new->tm_mday);
		sprintf(hora_new,"%d",p_tiempo_new->tm_hour);
		sprintf(minuto_new,"%d",p_tiempo_new->tm_min);
		sprintf(segundo_new,"%d",p_tiempo_new->tm_sec);
	
		strcpy(cadena_a_escribir_new,ano_new);
		strcat(cadena_a_escribir_new,"_");
		strcat(cadena_a_escribir_new,mes_new);
		strcat(cadena_a_escribir_new,"_");
		strcat(cadena_a_escribir_new,dia_new);
		strcat(cadena_a_escribir_new,"_");
		strcat(cadena_a_escribir_new,hora_new);
		strcat(cadena_a_escribir_new,":");
		strcat(cadena_a_escribir_new,minuto_new);
		strcat(cadena_a_escribir_new,":");
		strcat(cadena_a_escribir_new,segundo_new);
	
		if (order == 0 || order == 3)
		{
			stat (nombre_arch, debug_attributes);
	
			if ((debug_attributes->st_size/1048576) > 5)
			{
				fclose(debug_file1);
		
				bzero((char*)nombre_arch, 100);
				strcpy(nombre_arch,"/var/log");
				strcat(nombre_arch,cadena_a_escribir_new);
				strcat(nombre_arch,"_");
				strcat(nombre_arch,"ldid.log");
	
				debug_file1 = fopen(nombre_arch,"a");
				if (debug_file1 == NULL)
				{
  					fprintf(stderr, "error creando el archivo de debug de la aplicacion ldid! \n");
  					return(1);
				}
				fprintf(stderr, "archivo debug de la aplicacion ldid creado correctamente!\n");
			}
			delete debug_attributes;
		}
		if (order != 3)
		{
			strcat(cadena_a_escribir_new,": ");
			fprintf(debug_file1, cadena_a_escribir_new);
		}
		free(ano_new);
		free(mes_new);
		free(dia_new);
		free(hora_new);
		free(minuto_new);
		free(segundo_new);
		free(cadena_a_escribir_new);
	}
	fprintf(debug_file1, cadena);
	fflush(debug_file1);
	return(0);
}
/*int lee_tabla(char* archivo,class terminal** tabla_LDID)
{
	// Esta funci'on lee la tabla LDID que est'a guardada en el disco duro. Retorna la cantidad de records que tiene la tabla. Se debe invocar una primera vez con
	// el segundo par'ametro (tabla_LDID) con valor NULL, de tal manera que solo retorne la cantidad de records que tiene la tabla. De esta manera se puede crear el
	// arreglo tabla_LDID que se le pasar'a como referencia en la segunda invocaci'on, en la cual se llena esta.
	
	int a = 0;
	
	FILE* file = fopen(archivo, "r+ t");
	if (file == NULL)
	{

  		//fprintf(debug_file, "error abriendo el archivo de configuracion!\n");
		//fflush(debug_file);
		
		//fprintf(debug_file, "error abriendo la tabla LDID!\n");
		bzero((char*)cadena_a_escribir, 4096);
		strcpy(cadena_a_escribir,"error abriendo la tabla LDID!\n");
		escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,0);
		fclose(debug_file);
  		return(-1);
	}
	else
	{
		//fprintf(debug_file, "tabla LDID abierta correctamente...!\n");
		bzero((char*)cadena_a_escribir, 4096);
		strcpy(cadena_a_escribir,"tabla LDID abierta correctamente...!\n");
		escribir_debug(nombre_archivo_debug,debug_file,cadena_a_escribir,0);
		//fclose(debug_file);
	
		ssize_t bytes_read = 0;
		char* linea = new char[200];
		size_t cantidad = 0;
		int comp_string = -1;
		char* status_str = new char[2];
		char* address_str = new char[16];
		char* puerto_str = new char[7]; //65536;
		char* tiempo_str = new char[11];
		
		while(comp_string != 0)
		{
			bzero((char*)linea, 200);
			bytes_read = getline (&linea,&cantidad,file);
			escribir_debug(nombre_archivo_debug,debug_file,linea,0);
			fflush(debug_file);
			comp_string = strcmp((char*)linea,"FIN");
			if (comp_string != 0)
			{
				//escribir_debug(nombre_archivo_debug,debug_file,linea,0);
				if (tabla_LDID != NULL)
				{
					strcpy(tabla_LDID[a]->Nombre,strtok(linea,";"));
					//fprintf(debug_file, "%s : ",tabla_LDID[a]->Nombre);
						   
					bzero((char*)status_str, 2);
					strcpy(status_str,strtok(NULL,";"));
					tabla_LDID[a]->Status = atoi(status_str);
					//fprintf(debug_file, "%d : ",tabla_LDID[a]->Status);
					
					bzero((char*)address_str,16);
					strcpy(address_str,strtok(NULL,";"));
					inet_aton(address_str, &(tabla_LDID[a]->direccion.sin_addr));
					//fprintf(debug_file, "%d : ",tabla_LDID[a]->direccion.sin_addr);
						   
					bzero((char*)puerto_str,7);
					strcpy(puerto_str,strtok(NULL,";"));
					tabla_LDID[a]->direccion.sin_port = atoi(puerto_str);
					//fprintf(debug_file, "%d : ",tabla_LDID[a]->direccion.sin_port);
						   
					bzero((char*)tiempo_str, 11);
					strcpy(tiempo_str,strtok(NULL,";"));
					tabla_LDID[a]->tiempo_actual = atoi(tiempo_str);
					//fprintf(debug_file, "%d\n",tabla_LDID[a]->tiempo_actual);
				}
				a++;
			}
			else break;
		}
		//delete[] status_str;
		//delete[] address_str;
		//delete[] puerto_str;
		//delete[] tiempo_str;
	}
	//escribir_debug(nombre_archivo_debug,debug_file,"ok",0);
	//fclose(file);
	//escribir_debug(nombre_archivo_debug,debug_file,"ok",0);
	//return(a);
}*/



