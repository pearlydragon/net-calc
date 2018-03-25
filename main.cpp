#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include "mysql_driver.h"
#include "mysql_connection.h"
#include <string>       //for string type
#include <sys/socket.h> //for socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h>    //write
#include <cstring>      //for strlen

#include <cppconn/driver.h>
#include <cppconn/exception.h>
#include <cppconn/resultset.h>
#include <cppconn/statement.h>

using namespace std;

void open_connect();

string db_address = "tcp://127.0.0.1:3306";
string db_user = "net_calc";
string db_pass = "net_calc";
string db_schema = "net_calc";


sql::mysql::MySQL_Driver *driver;
sql::Connection *con;
sql::Statement *stmt;
sql::ResultSet *res;

int server_port = 6666;

int main(int argc, char *argv[])
{
    map<string,int> client_actions; //map for convert chars.....
    client_actions.insert(pair<string,int>("login", 0));
    client_actions.insert(pair<string,int>("logout", 1));
    client_actions.insert(pair<string,int>("password", 2));
    client_actions.insert(pair<string,int>("calc", 3));

    if (string(argv[1]) == "--create_db"){
        open_connect();
    }
    if (string(argv[1]) == "--start"){
        int socket_desc , client_sock , c , read_size;
            struct sockaddr_in server , client;
            char client_message[2000];

            //Create socket
            socket_desc = socket(AF_INET , SOCK_STREAM , 0);
            if (socket_desc == -1)
            {
                printf("Could not create socket");
            }
            puts("Socket created");

            //Prepare the sockaddr_in structure
            server.sin_family = AF_INET;
            server.sin_addr.s_addr = INADDR_ANY;
            server.sin_port = htons( server_port );

            //Bind
            if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
            {
                //print the error message
                perror("bind failed. Error");
                return 1;
            }
            puts("bind done");

            //Listen
            listen(socket_desc , 3);

            //Accept and incoming connection
            puts("Waiting for incoming connections...");
            c = sizeof(struct sockaddr_in);

            //accept connection from an incoming client
            client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c);
            if (client_sock < 0)
            {
                perror("accept failed");
                return 1;
            }
            puts("Connection accepted");

            //Receive a message from client
            while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
            {
                //Send the message back to client
                //write(client_sock , client_message , strlen(client_message))
                printf ("Client send: %s\n", client_message);
                if (client_actions.count(client_message) == 1){

                    switch (client_actions[client_message]) {
                    case 0:
                        write(client_sock , "Login checking" , strlen("Login checking"));
                        break;
                    case 2:
                        write(client_sock , "Password checking..." , strlen("Password checking..."));
                        break;
                    case 1:
                        write(client_sock , "Bye-bye" , strlen("Bye-bye"));
                        break;
                    case 3:
                        write(client_sock , "Calculating..." , strlen("Calculating..."));
                        break;
                    }
                }else write(client_sock , "Wrong command! Try again." , strlen("Wrong command! Try again."));

            }

            if(read_size == 0)
            {
                puts("Client disconnected");
                fflush(stdout);
            }
            else if(read_size == -1)
            {
                perror("recv failed");
            }
    }

    return 0;
}


void open_connect(){
    try {

        /* Create a connection */
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(db_address, db_user, db_pass);
        /* Connect to the MySQL test database */
        con->setSchema(db_schema);

        stmt = con->createStatement();
        stmt->execute("USE net_calc");
        stmt->execute("DROP TABLE IF EXISTS users");
        stmt->execute("CREATE TABLE users(id INT, user TEXT, pass TEXT, count INT)");
        stmt->execute("INSERT INTO users(id, user, pass, count) VALUES (0, 'test0', 'test0', 10)");
        stmt->execute("DROP TABLE IF EXISTS log");
        stmt->execute("CREATE TABLE log(user_id INT, date DATE, action TEXT, resault TEXT)");
        delete res;
        delete stmt;

    } catch (sql::SQLException &e) {
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
}
