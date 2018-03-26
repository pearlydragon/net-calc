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
int check_login(string user_login);
int check_pass(string user_pass);
int do_calc(string user, string pass, double f_number, double s_number, char operation, double resault);
int check_count(string user, string pass);

string db_address = "tcp://127.0.0.1:3306";
string db_user = "net_calc";
string db_pass = "net_calc";
string db_schema = "net_calc";

sql::mysql::MySQL_Driver *driver;
sql::Connection *con;
sql::Statement *stmt;
sql::PreparedStatement *pstmt;
sql::ResultSet *res;

int server__port = 6666;

int main(int argc, char *argv[])
{
    map<string,int> client_actions; //map for convert chars.....
    client_actions.insert(pair<string,int>("login", 0));
    client_actions.insert(pair<string,int>("logout", 1));
    client_actions.insert(pair<string,int>("password", 2));
    client_actions.insert(pair<string,int>("calc", 3));

    map<char,int> math_operations; //map for convert chars.....
    client_actions.insert(pair<string,int>("+", 0));
    client_actions.insert(pair<string,int>("-", 1));
    client_actions.insert(pair<string,int>("*", 2));
    client_actions.insert(pair<string,int>("/", 3));

    if (string(argv[1]) == "--create_db"){
        open_connect();
    }
    if (string(argv[1]) == "--start"){
        //if (string(argv[2]) == "-p"){
        //    server_port = atoi(argv[3]);
        //}

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
        server.sin_port = htons( server__port );

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
        int command = 10;
        string client_login = "";
        string client_pass = "";
        double first_number = 0;
        double second_number = 0;
        double resault = 0;
        char operation;
        int count = 0;
        while( (read_size = recv(client_sock , client_message , 2000 , 0)) > 0 )
        {
            //Send the message back to client
            //write(client_sock , client_message , strlen(client_message))
            printf ("Client send: %s %d\n", client_message, client_actions.count(client_message)); //for debug and see what client send to server
            printf ("Current command : %d\n", command);
            if (command >= 0 && command <=3){
                switch (command) {
                case 0:
                    client_login = client_message;
                    if (check_login(client_login)){
                        command = 10;
                        write(client_sock , "Login: OK" , strlen("Login: OK"));
                        continue;
                    }else{
                        client_login = "";
                        write(client_sock , "Login: FAIL. Try again." , strlen("Login: FAIL. Try again."));
                    }
                    break;
                case 1:
                    break;
                case 2:
                    client_pass = client_message;
                    if (check_pass(client_pass) == 1){
                        command = 10;
                        write(client_sock , "Password: OK" , strlen("Password: OK"));
                        //continue;
                    }else{
                        client_login = "";
                        write(client_sock , "Password: FAIL. Try again." , strlen("Password: FAIL. Try again."));
                    }
                    break;
                case 3:
                    printf ("%d: first: %f, second: %f, result: %f, %d\n", __LINE__, first_number, second_number, resault, isdigit( client_message[0]));
                    //for (int i = 0; i < strlen(client_message); i++){
                        if (isdigit( client_message[0])){
                            if (first_number == 0){
                                first_number = atof(client_message);
                            }else{
                                if (second_number == 0){
                                    second_number == atof(client_message);
                                    switch (math_operations[operation]) {
                                    case 0:
                                        resault = first_number + second_number;
                                        printf ("%d: first: %f, second: %f, result: %f\n", __LINE__, first_number, second_number, resault);
                                        break;
                                    case 1:
                                        resault = first_number - second_number;
                                        break;
                                    case 2:
                                        resault = first_number * second_number;
                                        break;
                                    case 3:
                                        resault = first_number / second_number;
                                        break;
                                    }

                                    if (check_count(client_login, client_pass) == 1){
                                        if (do_calc(client_login, client_pass, first_number, second_number, operation, resault) == 1){
                                            command = 10;
                                            string temp = "Resault: "+to_string(resault);
                                            write(client_sock , temp.c_str(), temp.length());
                                            continue;
                                        }
                                    }

                                }else{
                                    first_number = resault;
                                    second_number = atof(client_message);
                                    switch (math_operations[operation]) {
                                    case 0:
                                        resault = first_number + second_number;
                                        printf ("%d: first: %f, second: %f, result: %f\n", __LINE__, first_number, second_number, resault);
                                        break;
                                    case 1:
                                        resault = first_number - second_number;
                                        break;
                                    case 2:
                                        resault = first_number * second_number;
                                        break;
                                    case 3:
                                        resault = first_number / second_number;
                                        break;
                                    }

                                    if (check_count(client_login, client_pass) == 1){
                                        if (do_calc(client_login, client_pass, first_number, second_number, operation, resault) == 1){
                                            command = 10;
                                            string temp = "Resault: "+to_string(resault);
                                            write(client_sock , temp.c_str(), temp.length());
                                            continue;
                                        }
                                    }

                                }
                            }
                        }else{
                            if (math_operations.count(client_message[0]) !=1){
                                write(client_sock , "Wrong operation or value - try again", strlen("Wrong operation or value. Try again"));
                            }
                        }
                    //}

                    break;
                }
            }
            if (client_actions.count(client_message) == 1 && command == 10){

                switch (client_actions[client_message]) {
                case 0:
                    command = 0;
                    write(client_sock , "Login checking" , strlen("Login checking"));
                    break;
                case 2:
                    if (client_login == ""){
                        write(client_sock , "Enter your login before!" , strlen("Enter your login before!"));
                    }else{
                        command = 2;
                        write(client_sock , "Password checking..." , strlen("Password checking..."));
                    }
                    break;
                case 1:
                    write(client_sock , "Bye-bye" , strlen("Bye-bye"));
                    close(client_sock);

                    break;
                case 3:
                    if (client_login == "" || client_pass == ""){
                        write(client_sock , "Enter your login/pass before!" , strlen("Enter your login/pass before!"));
                    }else{
                        command = 3;
                        resault = 0;
                        write(client_sock , "Calculating..." , strlen("Calculating..."));

                    }
                    break;
                }
            }else write(client_sock , "Wrong command! Try again." , strlen("Wrong command! Try again."));
            for (int i = 0; i < 20; i++){
                client_message[i] = '\0';
            }
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
        stmt->execute("COMMIT");
        stmt->execute("DROP TABLE IF EXISTS log");
        stmt->execute("CREATE TABLE log(user_id INT, date DATE, action TEXT, resault TEXT)");
        delete res;
        delete stmt;
        delete con;

    } catch (sql::SQLException &e) {
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
    }
}

int check_login(string user_login){
    user_login = '"'+user_login+'"';
    try {

        /* Create a connection */
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(db_address, db_user, db_pass);
        /* Connect to the MySQL test database */
        con->setSchema(db_schema);
        stmt = con->createStatement();
        stmt->execute("USE net_calc");
        res = stmt->executeQuery("SELECT user FROM users WHERE user = "+user_login);
        //
        if (res->rowsCount() == 1){
            delete res;
            delete stmt;
            delete con;
            return 1;
        }else{
            delete res;
            delete stmt;
            delete con;
            return 0;
        }

    } catch (sql::SQLException &e) {
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      return 0;
    }

}

int check_pass(string user_pass){
    user_pass = '"'+user_pass+'"';
    try {

        /* Create a connection */
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(db_address, db_user, db_pass);
        /* Connect to the MySQL test database */
        con->setSchema(db_schema);
        stmt = con->createStatement();
        stmt->execute("USE net_calc");
        res = stmt->executeQuery("SELECT pass FROM users WHERE pass = "+user_pass);
        printf ("%d: Rows return: %d\n", __LINE__, res->rowsCount());
        //
        if (res->rowsCount() == 1){
            delete res;
            delete stmt;
            delete con;
            return 1;
        }else{
            delete res;
            delete stmt;
            delete con;
            return 0;
        }

    } catch (sql::SQLException &e) {
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      return 0;
    }

}

int do_calc(string user, string pass, double f_number, double s_number, char operation , double resault){
    string text_operation = '"'+to_string(f_number)+operation+to_string(s_number)+'"';
    string string_login = '"'+user+'"';
    string string_password = '"'+pass+'"';
    try {

        /* Create a connection */
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(db_address, db_user, db_pass);
        /* Connect to the MySQL test database */
        con->setSchema(db_schema);
        cout << "Enter\n" << endl;
        stmt = con->createStatement();
        stmt->execute("USE net_calc");
        stmt->executeQuery("SELECT id FROM users WHERE user = "+string_login+" AND pass = "+string_password);
        int userid = res->getInt("id");
        stmt->executeQuery("INSERT INTO logs (user_id, date, action, resault) ("+to_string(userid)+",now(),"+text_operation+','+to_string(resault)+")");
        stmt->executeQuery("COMMIT");
        res = stmt->executeQuery("SELECT count FROM users WHERE user = "+string_login+" AND pass = "+string_password);
        int count = res->getInt("count");
        count--;
        stmt->executeQuery("UPDATE users SET count = "+count);
        stmt->executeQuery("COMMIT");
        printf ("%d: Rows return: %d\n", __LINE__, res->rowsCount());
        //
        delete res;
        delete stmt;
        delete con;
        return 0;

    } catch (sql::SQLException &e) {
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      return 0;
    }

}

int check_count(string user, string pass){
    string string_login = '"'+user+'"';
    string string_password = '"'+pass+'"';

    try {

        /* Create a connection */
        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect(db_address, db_user, db_pass);
        /* Connect to the MySQL test database */
        con->setSchema(db_schema);
        stmt = con->createStatement();
        stmt->execute("USE net_calc");
        res = stmt->executeQuery("SELECT count FROM users WHERE user = "+string_login+" AND pass = "+string_password);
        if (res->rowsCount() > 0){
            cout << "All good" << res->getInt("count") << endl;
        }else cout << "Somthink goes wrong(" << endl;
        int count = res->getInt("count");
        printf ("%d: Count return: %d\n", __LINE__, count);
        //
        delete res;
        delete stmt;
        delete con;
        if (count = 0) return 1;
        else return 0;

    } catch (sql::SQLException &e) {
      cout << "# ERR: SQLException in " << __FILE__;
      cout << "(" << __FUNCTION__ << ") on line " << __LINE__ << endl;
      cout << "# ERR: " << e.what();
      cout << " (MySQL error code: " << e.getErrorCode();
      cout << ", SQLState: " << e.getSQLState() << " )" << endl;
      return 0;
    }
}
