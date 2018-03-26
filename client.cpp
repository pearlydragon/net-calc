#include <stdio.h> //printf
#include <string>
#include <cstring>   //strlen
#include <sys/socket.h>    //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //

using namespace std;

char server_addr[16]= "127.0.0.1";
int server_port = 6666;


int main(int argc , char *argv[])
{
    int sock;
    struct sockaddr_in server;
    char message[2000] , server_reply[2000];

    //Create socket
    sock = socket(AF_INET , SOCK_STREAM , 0);
    if (sock == -1)
    {
        printf("Could not create socket");
    }
    printf("Socket created\n");

    server.sin_addr.s_addr = inet_addr(server_addr);
    server.sin_family = AF_INET;
    server.sin_port = htons( server_port );

    //Connect to remote server
    if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
    {
        perror("connect failed. Error");
        return 1;
    }

    printf("Connected\n");

    //keep communicating with server
    while(1)
    {
        memset(message, 0, sizeof(message)); //clean message before use
        printf("Enter command : ");
        scanf("%s" , message);
        printf("\n");
        if (strstr(message, "quit")){
            printf ("Bye-bye!\n");
            close(sock);
            return 0;
        }

        //Send some data
        if( send(sock , message , strlen(message) , 0) < 0)
        {
            printf("Send failed\n");
            return 1;
        }

        //Receive a reply from the server
        if( recv(sock , server_reply , 2000 , 0) < 0)
        {
            printf("recv failed\n");
            break;
        }

        printf("Server reply : %s\n", server_reply);
        for (int i = 0; i < 2000; i++){
            server_reply[i] = '\0';
        }

    }

    close(sock);
    return 0;
}
