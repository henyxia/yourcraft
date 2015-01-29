#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <pthread.h>
#include <stdbool.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)
#define CRLF			"\r\n"
#define PORT	 		1977
#define MAX_CLIENTS 	500
#define BUF_SIZE		1024

static pthread_mutex_t sock_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
typedef struct in_addr IN_ADDR;
typedef struct
{
	bool		online;
	pthread_t	thread;
	SOCKET		sock;
}client;

int init_connection(void);
void end_connection(int sock);
int read_client(SOCKET sock, char *buffer);
void write_client(SOCKET sock, const char *buffer);

void initClients(client* myClients)
{
	int i;
	for(i=0; i<MAX_CLIENTS; i++)
	{
		myClients[i].online = false;
		myClients[i].sock = INVALID_SOCKET;
		myClients[i].thread = 0;
	}
}

int getNewClient(client* myClients)
{
	int i=0;
	while(i<MAX_CLIENTS && myClients[i].online)
		i++;

	if(!(i<MAX_CLIENTS))
		return -1;

	return i;
}

void * processClient(void* arg)
{
	printf("Incoming client !\n");

	int ret;
	char	buffer[BUF_SIZE];
	SOCKADDR_IN csin = { 0 };
	size_t sinsize = sizeof csin;
	client* mClient = arg;
	int csock = accept(mClient->sock, (SOCKADDR *)&csin, &sinsize);
	pthread_mutex_unlock(&sock_mutex);
	if(csock == SOCKET_ERROR)
	{
		printf("accept failed");
	}

	ret=read_client(csock, buffer);
	if(read_client(csock, buffer) == -1)
		printf("Client read failed\n");
	else
	{
		int i=0;
		printf("Bytes bytes recieved : %d\n", ret);
		while(buffer[i] != '\0')
		{
			printf("%d ", buffer[i]);
			i++;
		}
		printf("\n");
	}

	return NULL;
}

void startBind(void)
{
	client clients[MAX_CLIENTS];
	initClients(clients);

	SOCKET	sock = init_connection();
	int		client=0;
	int		max = sock;
	int		ret=0;

	fd_set rdfs;

	while(1)
	{
		FD_ZERO(&rdfs);
		FD_SET(sock, &rdfs);

		// Dirty way
		while(pthread_mutex_lock(&sock_mutex) == EDEADLK)
		{
		}

		if(select(max + 1, &rdfs, NULL, NULL, NULL) == -1)
		{
			perror("select()");
			exit(errno);
		}
		else if(FD_ISSET(sock, &rdfs))
		{
			client = getNewClient(clients);
			if(client == -1)
				printf("Can't accept more clients\n");
			else
			{
				clients[client].sock=sock;
				printf("Connection %d is actually free, allowing it to client\n", client);
				ret = pthread_create(&clients[client].thread, NULL, processClient, (void*) &clients[client]);
				if(ret == 0)
				{
					printf("Thread created successfully\n");
				}
				else
				{
					printf("Thread creation failed, err %d\n", ret);
				}
			}
		}
		else
		{
		}
	}
	end_connection(sock);
}

int init_connection(void)
{
   SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
   SOCKADDR_IN sin = { 0 };

   if(sock == INVALID_SOCKET)
   {
      perror("socket()");
      exit(errno);
   }

   sin.sin_addr.s_addr = htonl(INADDR_ANY);
   sin.sin_port = htons(PORT);
   sin.sin_family = AF_INET;

   if(bind(sock,(SOCKADDR *) &sin, sizeof sin) == SOCKET_ERROR)
   {
      perror("bind()");
      exit(errno);
   }

   if(listen(sock, MAX_CLIENTS) == SOCKET_ERROR)
   {
      perror("listen()");
      exit(errno);
   }

   return sock;
}

void end_connection(int sock)
{
   closesocket(sock);
}

int read_client(SOCKET sock, char *buffer)
{
   int n = 0;

   if((n = recv(sock, buffer, BUF_SIZE - 1, 0)) < 0)
   {
      perror("recv()");
      /* if recv error we disonnect the client */
      n = 0;
   }

   buffer[n] = 0;

   return n;
}

void write_client(SOCKET sock, const char *buffer)
{
   if(send(sock, buffer, strlen(buffer), 0) < 0)
   {
      perror("send()");
      exit(errno);
   }
}
