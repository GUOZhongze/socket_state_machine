#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h> 
#include <string.h> 
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <linux/fb.h>


int go_ping(const char *cmdstring)
{
    pid_t pid;
    if ((pid = vfork()) < 0)
    {
        printf("\nvfork error\n");
       	return 0;
    }
    else if (pid == 0)
    {       
		if ( execl("/bin/sh", "sh", "-c", cmdstring, (char *)0) < 0)
        {
            printf("\nexeclp error\n");
            return 0;
        }
    }
	int ret;
    int stat;
    waitpid(pid, &stat, 0);
    if (stat == 0)
    {
        ret=0;
    }
    else if (stat>0)
    {
        ret=1;
    }  
    return ret;    
}

enum socket_status
{
	SOCKET_CLOSE = 1,
	SOCKET_CREATE,
	SOCKET_CONNECT,
	SOCKET_PLAYING,
}	status_for_socket;

struct sockaddr_in server_imx8;

int main(int argc, char *argv[])
{ 
	int client_Socket;
	char send_buffer[] = "5678"; 
	char recv_buffer[20];
	int returnStatus ;
    	int socket_client_port = 6666;												
    	status_for_socket = SOCKET_CLOSE;	
    	//------------start of state machine---------------------------//
	while(1)
	{
		switch( status_for_socket )
		{
			case SOCKET_CLOSE:
				/* create a streaming socket */
				client_Socket = socket(AF_INET, SOCK_STREAM , 0);//| SOCK_NONBLOCK
				if (client_Socket == -1) 
				{
					printf( "Could not create a socket!\n");
					status_for_socket=SOCKET_CLOSE;
				}
				else 
				{
					/* setup the address structure */
					/* use the IP address sent as an argument for the server address  */ 
					server_imx8.sin_family = AF_INET; 
					server_imx8.sin_port = htons(socket_client_port);
					server_imx8.sin_addr.s_addr = inet_addr("127.0.0.1");
					printf( "Socket created! port set!\n");
					status_for_socket=SOCKET_CREATE;
				} 
				break;  

			case SOCKET_CREATE:
				returnStatus = connect(client_Socket, (struct sockaddr *)&server_imx8, sizeof(server_imx8));
				if (returnStatus == 0)
				{
					printf("connected \n");
					sleep(1);
					status_for_socket=SOCKET_CONNECT;
				}	 
				else
				{
					printf("socket not connected \n");
					sleep(1);
					status_for_socket=SOCKET_CREATE;
				}
				break;

			case SOCKET_CONNECT:
				send(client_Socket, send_buffer, strlen(send_buffer), 0);
				printf("sent \n"); 
				recv(client_Socket, recv_buffer, 20, 0);
				for(int i=4;i<10;i++)
				{
					recv_buffer[i]=0;
				}
				if( strcmp(recv_buffer,"1234")== 0 ) //wait 1234 key from server
				{ 
					printf("get the key %s, verified\n", recv_buffer);				
					status_for_socket=SOCKET_PLAYING;
				}
				else
				{
					printf("get the error %s\n",recv_buffer);
					status_for_socket=SOCKET_CREATE;
				} 
				bzero(&recv_buffer,20) ;      
				break;
				
			case SOCKET_PLAYING:
				close(client_Socket);
	    			while(1)
	    			{   	
	    				int status=go_ping("ping -c 1 -s 1 127.0.0.1 >/dev/null 2>&1");
	    				if(status==0)
	    				{
	    					printf("ping succeed\n");
	    					sleep(2);
	    				}
	    				else if(status==1)
	    				{
	    					printf("ping failed\n");
	    					sleep(1);
	    					break;
	    				}
				}				
				status_for_socket = SOCKET_CLOSE;
											
				break;

			default:
				status_for_socket = SOCKET_CLOSE;
				break;

		}
	}
    	printf("break the STATE MACHINE\n");
 	/* end of program */	
	close(client_Socket); 
	status_for_socket = SOCKET_CLOSE;
	return 0;
}




