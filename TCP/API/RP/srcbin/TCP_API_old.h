#ifndef TCP_API_H
#define TCP_API_H

#include<stdio.h> //for printf
#include<stdlib.h> //for exit()
#include<sys/socket.h> //for socket
#include<sys/types.h> //utile?
#include<arpa/inet.h> //for inet_addr
#include<errno.h> //for error
#include<unistd.h> //for close()
#include<netdb.h> //for gethostbyname
#include<pthread.h> //for thread

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

pthread_t TCP_server_thread;

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADiDR;
typedef struct client client; //structure that contained client informations
typedef struct server_info server_info; //structure need to make thread with server routinr

void init_struct_client(client* client_list,unsigned int Nmax); //initialise client structure
void clear_struct_client(client* client_list); //free malloc on struct client
void add_client(client* client_list, SOCKET sock_server); //TCP server is initialised it add new client informations in structure client
void clear_client(client* client_list,unsigned int id); //clear client with id_client==id form structure client and reorganize the structure
void init_TCP_client(SOCKET* sock, const char* IP, int Port); //initialise TCP client
void init_TCP_server(SOCKET* sock, int Port, client* client_list,unsigned int MaxClient); //initialise TCP server
void *TCP_server_routine(void* p_data); //server routine function for thread, server turn in parallel to main
void launch_server(SOCKET* sock, client* client_list); //function that launch the server in parallel to main
void close_TCP_server(SOCKET* sock, client* client_list); //close client connexion and TCP server (for server)
void close_TCP_client(SOCKET* sock); //close client connexion (for client)
void send_TCP_server(client* client_list, char* buffer, int buff_length, int target); // send buffer of size buff_length to client with id target, if target<0 buffer is sent to all clients for server
void send_TCP_client(SOCKET* sock, char* buffer, int buff_length); //send buffer of size buff_length to server for client
void receive_TCP_server(client* client_list, char* buffer, int buff_length, int target); //receive buffer of size buff_length from client with id target for server
void reveive_TCP_client(SOCKET* sock, char* buffer, int buff_length); //receive buffer of size buff from server for client

struct client
{
	unsigned int Nmax;
	unsigned int NbClient;
	unsigned int* id_client;
	SOCKET* sock_client;
	SOCKADDR_IN* sin_client;
};

struct server_info
{
	SOCKET sock;
	client* client_list;
};

void init_struct_client(client* client_list, unsigned int Nmax)
{
	int i=0;
	client_list->Nmax=Nmax;
	client_list->NbClient=0;
	client_list->sock_client=(SOCKET *)malloc(Nmax*sizeof(SOCKET));
	client_list->sin_client=(SOCKADDR_IN *)malloc(Nmax*sizeof(SOCKADDR_IN));
	client_list->id_client=(unsigned int *)malloc(Nmax*sizeof(unsigned int));
	for (i=0 ; i<Nmax ; i++){client_list->id_client[i]=0;}
}

void clear_struct_client(client* client_list)
{
	free(client_list->sock_client);
	free(client_list->sin_client);
	free(client_list->id_client);
}

void add_client(client* client_list, SOCKET sock_server)
{
		unsigned int socklen=sizeof(SOCKADDR_IN);
		if (client_list->NbClient<client_list->Nmax-1)
		{
			client_list->sock_client[client_list->NbClient]=accept(sock_server,(SOCKADDR *)&client_list->sin_client[client_list->NbClient],&socklen);
		if (client_list->sock_client[client_list->NbClient]==INVALID_SOCKET)
		{
			perror("accept()");
			exit(errno);
		}
		else
		{
			client_list->id_client[client_list->NbClient]=client_list->NbClient;
			client_list->NbClient++;
			printf("New client connected\n");
		}
	}
	else{printf("too many clients\n");}
}

void clear_client(client* client_list, unsigned int id)
{
	int i;
	client* client_temp=NULL;
	init_struct_client(client_temp,client_list->Nmax);
	if (client_list->NbClient>1)
	{
		client_temp->NbClient=client_list->NbClient-1;
		if (id==client_list->NbClient-1)
		{
			for (i=0 ; i<id-1 ; i++)
			{
				client_temp->sock_client[i]=client_list->sock_client[i];
				client_temp->sin_client[i]=client_list->sin_client[i];
				client_temp->id_client[i]=client_list->id_client[i];
			}
		}
		else
		{
			for (i=0 ; i<id-1 ; i++)
			{
				client_temp->sock_client[i]=client_list->sock_client[i];
				client_temp->sin_client[i]=client_list->sin_client[i];
				client_temp->id_client[i]=client_list->id_client[i];
			}
			for (i=id+1 ; i<client_list->NbClient ; i++)
			{
				client_temp->sock_client[i-1]=client_list->sock_client[i];
				client_temp->sin_client[i-1]=client_list->sin_client[i];
				client_temp->id_client[i-1]=client_list->id_client[i];
			}
		}
	}
	(*client_list)=(*client_temp);
}

void init_TCP_client(SOCKET* sock, const char* IP, int Port)
{
	SOCKADDR_IN sclient={0};
	
	//Create socket
	(*sock)=socket(AF_INET, SOCK_STREAM, 0);
	if ((*sock)==INVALID_SOCKET)
	{
		perror("socket()");
		exit(errno);
	}

	//Socket info
	sclient.sin_addr.s_addr=inet_addr(IP);
	sclient.sin_family=AF_INET;
	sclient.sin_port=htons(Port);

	//Connection to server
	if (connect((*sock), (SOCKADDR *) &sclient, sizeof(SOCKADDR))==SOCKET_ERROR)
	{
		perror("connect()");
		exit(errno);
	}
	printf("Connected\n");
}

void init_TCP_server(SOCKET* sock, int Port,client* client_list, unsigned int MaxClient)
{
	SOCKADDR_IN server={0};

	//Create socket
	int i=56;
	i=socket(AF_INET, SOCK_STREAM,0);
	printf("socket return %i\n",i);
	*sock=i;
	if ((*sock)==INVALID_SOCKET)
	{
		perror("socket()");
		exit(errno);
	}

	//Socket info
	server.sin_family=AF_INET;
	server.sin_port=htons(Port);
	server.sin_addr.s_addr=INADDR_ANY; //allow all IP to access to the server

	//Link communication point
	if (bind((*sock), (SOCKADDR *)&server, sizeof(SOCKADDR))==SOCKET_ERROR)
	{
		perror("bind()");
		exit(errno);
	}

	//Connexion
	if (listen((*sock), MaxClient)==SOCKET_ERROR)
	{
		perror("listen()");
		exit(errno);
	}

	init_struct_client(client_list, MaxClient);

}

void *TCP_server_routine(void* p_data)
{
	server_info* serv_info=p_data;

	printf("socket = %i\n",serv_info->sock);
	
	while (1)
	{
		add_client(serv_info->client_list,serv_info->sock);
		printf("after add\n");
	}

	return NULL;
}

void launch_server(SOCKET* sock, client* client_list)
{
	server_info* serv_info=(server_info *)malloc(sizeof(server_info));
	serv_info->sock=(*sock);
	serv_info->client_list=client_list;

	if(pthread_create(&TCP_server_thread, NULL, TCP_server_routine, serv_info)!=0)
	{
		perror("pthread_create()");
		exit(errno);
	}
}

void close_TCP_server(SOCKET* sock, client* client_list)
{
	int i=0;

	pthread_cancel(TCP_server_thread);//close thread

	for(i=0 ; i<client_list->NbClient ; i++){close(client_list->sock_client[i]);}
	close((*sock));
	clear_struct_client(client_list);
}

void close_TCP_client(SOCKET* sock)
{
	close((*sock));
}

void send_TCP_server(client* client_list, char* buffer, int buff_length, int target)
{
	int i=0;
	if (target<0)
	{
		for (i=0 ; i<client_list->NbClient ; i++)
		{
			if(send(client_list->sock_client[i], buffer, buff_length, 0)==0)
			{
				clear_client(client_list, client_list->id_client[i]);
				printf("client %u disconnected\n",client_list->id_client[i]+1);
			}
		}
	}
	else
	{
		if(send(client_list->sock_client[target], buffer, buff_length, 0)<0)
		{
			perror("send()");
			exit(errno);
		}
	}
}

void send_TCP_client(SOCKET* sock, char* buffer, int buff_length)
{
	if(send((*sock), buffer, buff_length, 0)<0)
	{
		perror("send()");
		exit(errno);
	}
}

void receive_TCP_server(client* client_list, char* buffer, int buff_length, int target)
{
	if (recv(client_list->sock_client[target], buffer, buff_length+1, MSG_WAITALL)==0)
	{
		printf("client disconnected\n");
	}
}

void receive_TCP_client(SOCKET* sock, char* buffer, int buff_length)
{
	if (recv((*sock), buffer, buff_length+1, MSG_WAITALL)==0)
	{
		printf("server closed\n");
	}
}

#endif
