#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
/* ------------------------------------------------------------------- */
/* ------------------------GLOBAL VARIABLES--------------------------- */
/* ------------------------------------------------------------------- */

/* Structure to contain all our information, so we can pass it around */

#define SERVER_PORT 6666 
#define RESPONSE_MODULE 5678

enum socket_status
{
    SOCKET_CLOSE = 1,
    SOCKET_CREATE,
    SOCKET_BIND,
    SOCKET_LISTEN,
    SOCKET_ACCEPT,
}   status_for_socket;

int main (int argc, char **argv)
{
    static struct sockaddr_in server_imx8;
    static struct sockaddr_in client_device;
    static int server_Socket;
    static int client_Socket;
    int client_len = sizeof(client_device);
    int returnStatus  = 0;
    int connect_times = 0; //count how many circles
    int target;
    char send_buffer[] = "1234";
    char recv_buffer[100];    
    status_for_socket = SOCKET_CLOSE;
    while(1)
    {
    	    switch(status_for_socket)
	    {
		case SOCKET_CLOSE:
		    if ((server_Socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)  //| SOCK_NONBLOCK
		    {
		        printf ("Socket can not created!\n");
		        return 1;
		    }
		    else
		    {
		        printf ("Socket created!\n");
		        status_for_socket = SOCKET_CREATE;
		    }
		    break;

		case SOCKET_CREATE:
		    bzero(&server_imx8, sizeof(server_imx8));
		    server_imx8.sin_family = AF_INET;
		    server_imx8.sin_addr.s_addr = htonl(INADDR_ANY);
		    server_imx8.sin_port = htons(SERVER_PORT);
		    returnStatus = bind(server_Socket, (struct sockaddr *)&server_imx8, sizeof(server_imx8));
		    if (returnStatus == 0)
		    {
		        printf ("Bind completed!\n");
		        status_for_socket = SOCKET_BIND;
		    }
		    else
		    {
		        printf ("Could not bind to address!\n");
		        close(server_Socket);
		        return 1;
		    }
		    break;

		case SOCKET_BIND:
		    returnStatus = listen(server_Socket, 5);
		    if (returnStatus == -1)
		    {
		        printf ("Cannot listen on socket!\n");
		        close(server_Socket);
		        return 1;
		    }
		    else
		    {
		        printf ("listen port: %d\n", SERVER_PORT);
		        status_for_socket = SOCKET_LISTEN;
		    }
		    break;

		case SOCKET_LISTEN:
		    client_Socket = accept(server_Socket, (struct sockaddr*)&client_device, (socklen_t*)&client_len);
		    if(client_Socket < 0)
		    {
		        //printf ("waiting devices to connect\n");
		        status_for_socket = SOCKET_LISTEN;
		    }
		    else
		    {
		        //printf ("devices connected\n");
		        status_for_socket = SOCKET_ACCEPT;
		    }
		    break;

		case SOCKET_ACCEPT: //status 5 means device connected, prepare to send and receive
		    connect_times ++;
		    send(client_Socket, send_buffer, strlen(send_buffer), 0);//continue send 1234 message until find device
		    printf ("sent\n");
		    recv(client_Socket, recv_buffer, 100, 0);
		    target=atoi(recv_buffer);
		    if ( target== RESPONSE_MODULE )
		    {
		        //wait 5678 key from target device
		        printf (" verified, cycle: %d \n", connect_times);
		        //printf("connected device IP is %s, Port is %d\n", inet_ntoa(client_device.sin_addr), htons(client_device.sin_port));
		        status_for_socket = SOCKET_LISTEN;
		    }
		    else if (target == 0)
		    {
		        printf ("not module, waiting reconnect, cycle: %d\n", connect_times);
		        status_for_socket = SOCKET_LISTEN;
		    }
		    else
		    {
		        printf ("%s recieved , error situation, cycle: %d \n",recv_buffer,connect_times);
		        status_for_socket = SOCKET_LISTEN;
		    }
		    /*reset recv buffer and clear connect_times*/
		    if (connect_times == 30000)
		    {
		        connect_times=0;
		    }
		    bzero(recv_buffer,100);
		    break;

		default:
		    status_for_socket = SOCKET_CLOSE;
		    break;
    	}
    }
    close(server_Socket); 
	
    return 0;
}












