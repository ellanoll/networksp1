// gcc sampleServer.c -o sampleServer -lpthread

#include<stdio.h>
#include<string.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<stdlib.h>
#include<dirent.h>
#include<errno.h>

#include <signal.h>
#include <pthread.h>
#include <sys/select.h>
#include <ctype.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>
#include <sys/time.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <wordexp.h>
#include <fcntl.h>
#include <pwd.h>
#include <sys/stat.h>

struct message_struct{
    char protocol[6];
    char type;      //type is 1 byte
    char status;        //status is 1 byte
    int length;     // length is 4 bytes (header + payload)
    char payload[1024]; //payload
} __attribute__ ((packed));


// Global variables
int server_port_num = 0;
int sock_descriptor = -1;
struct sockaddr_in server_address,clientAddr;
char one;

/* SELECT() ------------------------------------------------------------------------ THEO */
int connected_socks[5];  /* Array of connected sockets so we know who
                      we are talking to */
fd_set socks;        /* Socket file descriptors we want to wake
                      up for, using select() */
int highsock;         /* Highest #'d file descriptor, needed for select() */
/* SELECT() ------------------------------------------------------------------------ THEO */

int send_open(int newSocket){ //sends an open connection reply to client
	struct message_struct reply;

	reply.protocol[0] = 0xe3;
	strcat(reply.protocol, "myftp");
  reply.type = 0xA2;
  reply.status = 1;
  reply.length = 12;


	send(newSocket, (char*)(&reply),reply.length, 0); //send message
	return 0;
}



int authenticate_user(char loginInfo[1024]){
	FILE *fp; //filepointer
	char USR[1024]; //username
	int spaces = 0;
	loginInfo[strlen(loginInfo)-1] = '\0';

	if(fp = fopen("access.txt","rb"), fp == NULL) //open file w/ usr/pass combos
		return 0;
	else{
		while( fgets(USR, 1024, fp) != NULL ){
			for(int i=0;i<strlen(USR); i++){
				if(USR[i] == ' '){
					USR[i] = '\0';
				}
			}
			printf("login data: %s\n", loginInfo);
			printf("login data length: %d\n", (int)strlen(loginInfo));
			printf("cmp data: %s\n", USR);
			printf("cmp data length: %d\n", (int)strlen(USR));
			printf("cmp result: %d\n", strcmp(loginInfo, USR));
			if( strcmp(loginInfo, USR) == 0 ) //if login info verified
				return 1;
		}
	}
  //otherwise invalide
	return 0;
}

int authenticate_pass(char loginInfo[1024]){
	FILE *fp; //filepointer
	char USR[1024]; //username

	if(fp = fopen("access.txt","rb"), fp == NULL) //open file w/ usr/pass combos
		return 0;
	else{
		while( fgets(USR, 1024, fp) != NULL ){
			USR[strlen(USR)-1] = '\0';
			printf("login data: %s\n", loginInfo);
			printf("login data length: %d\n", (int)strlen(loginInfo));
			printf("cmp data: %s\n", USR);
			printf("cmp data length: %d\n", (int)strlen(USR));
			printf("cmp result: %d\n", strcmp(loginInfo, USR));
			if( strcmp(loginInfo, USR) == 0 ) //if login info verified
				return 1;
		}
	}
  //otherwise invalide
	return 0;
}




int send_ls_reply(int newSocket){

	return 0;
}

int send_get_reply(int newSocket, int status){

	return 0;
}

int recv_file_data(int newSocket, char directory[]){

	return 0;
}

int send_new(int newSocket, const void* buf, int buf_len)
{
    int n_left = buf_len;         // actual data bytes sent
    int n;
    while (n_left > 0){
        if ((n = send(newSocket, buf + (buf_len - n_left), n_left, 0)) < 0){
                if (errno == EINTR)
                        n = 0;
                else
                        return -1;
        } else if (n == 0) {
                return 0;
        }
        n_left -= n;
    }
    return buf_len;
}

int send_file_data(int newSocket, FILE * fb){

	return 0;
}

int send_auth(int newSocket, int status){ //send authorization reply to client
	struct message_struct reply;
	reply.protocol[0] = 0xe3;
	strcat(reply.protocol, "myftp");
  reply.type = 0xA4;   //type indicates authorization reply message
  reply.status = status;
  reply.length = 12;
	send(newSocket, (char*)(&reply),reply.length, 0); //send message
	return 0;
}

int send_pass(int newSocket, int status){ //send authorization reply to client
	struct message_struct reply;
	reply.protocol[0] = 0xe3;
	strcat(reply.protocol, "myftp");
  reply.type = 0xB2;   //type indicates authorization reply message
  reply.status = status;
  reply.length = 12;
	send(newSocket, (char*)(&reply),reply.length, 0); //send message
	return 0;
}

/*
 * This will handle connection for each client
 * */
/* SELECT() ------------------------------------------------------------------------ THEO */
void handle_data(int listnum) {
    // Get the socket descriptor from list
    int newSocket = connected_socks[listnum];

/* SELECT() ------------------------------------------------------------------------ THEO */

    struct message_struct recv_message, send_message;
    int authenticated, readSize, connected;

    FILE *fb;
	  char filename[1024]; //filename for ls
	  char directory[1024 + 10]; //directory for ls


    authenticated = 0;
    connected = 0;


    // Get message from client and read
    memset(recv_message.payload, '\0', 1024);
    readSize = recv(newSocket, (char *)&recv_message, sizeof(struct message_struct), 0);
    // Print type of message, for debugging
    printf("recv_message.type is %x\n", recv_message.type);
    // Analyze message based on its type
    switch( recv_message.type ){
        /********************************
         *	Handle open connection request	*
         ********************************/
        case (char) 0xA1:
            printf("Server received Open_Connection_Request\n");
            send_open(newSocket); //send open reply to client
            printf("Server sent Open_Connection_Reply\n");
          //  connected = 1; //socket is connected
            FD_SET(connected_socks[listnum],&socks); //set socket in list
            if (connected_socks[listnum] > highsock) //reasses highest socket
                highsock = connected_socks[listnum];

        break;

        /********************************
         *	Handle Authentication Request	*
         ********************************/
        case (char) 0xA3:
            printf("Server received Authentication_Request \n");
            recv_message.payload[recv_message.length-5] = '\0'; //assign payload
            printf("Server is Authenticating: %s\n",recv_message.payload);
            authenticated = authenticate_user(recv_message.payload); //authenticate request
            send_auth(newSocket, authenticated); //send reply
            printf("Server sent Authentication_Reply \n");

            if( authenticated != 1) //print authentication result
                printf("\t(Server: Access Failed)\n");
            else
                printf("\t(Server: Access Granted)\n");

        break;

        /********************************
         *	Handle Authentication Request	*
         ********************************/
        case (char) 0xB1:
            printf("Server received PASS Authentication_Request \n");
            recv_message.payload[recv_message.length-5] = '\0'; //assign payload
            printf("Server is Authenticating PASS: %s\n",recv_message.payload);
            authenticated = authenticate_pass(recv_message.payload); //authenticate request
            send_pass(newSocket, authenticated); //send reply
            printf("Server sent PASS Authentication_Reply \n");

            if( authenticated != 1) //print authentication result
                printf("\t(Server: PASS Access Failed)\n");
            else
                printf("\t(Server: PASS Access Granted)\n");

        break;

        /********************************
         *	Handle PUT_Request	*
         ********************************/
        case (char) 0xA9:
            if (!authenticated){
              strcpy(send_message.payload, "Server: You are not authenticated!\n");
              send_message.length = 12 + strlen(send_message.payload);
              //send authentication error to client until successful
              while( send(newSocket, (char*)(&send_message),send_message.length, 0) != send_message.length );
            }
            else{
                strncpy(filename, recv_message.payload,recv_message.length-12);
                filename[recv_message.length-12] = '\0'; //set filename
                printf("filename: %s\n", filename);
                printf("PUT_Request: %s Received\n", recv_message.payload);
                strcpy(directory, "./filedir/");
                strcat(directory, filename);
                memset(send_message.payload, '\0', 1024); //initialize payload
                send_message.protocol[0] = 0xe3;
                strcat(send_message.protocol, "myftp");
                send_message.type = 0xAA;
                send_message.length = 12;
                //send message to client
                send(newSocket, (char*)(&send_message), send_message.length, 0);
                printf("PUT_Reply Sent\n");
                //recieve file data
                recv_file_data(newSocket, directory);
            }
        break;

        /********************************
         *	Handle GET request	*
         ********************************/
        case (char) 0xA7:
            if (!authenticated) { //handle non authenticated user
              strcpy(send_message.payload, "Server: You are not authenticated!\n");
              send_message.length = 12 + strlen(send_message.payload);
              //send authentication error to client until successful
              while( send(newSocket, (char*)(&send_message),send_message.length, 0) != send_message.length );
            }
            else{
                strncpy(filename, recv_message.payload,recv_message.length-12);
                filename[recv_message.length-12] = '\0';

                printf("filename: %s\n", filename);
                printf("GET_Request: %s\n", recv_message.payload);

                strcpy(directory, "./filedir/");
                strcat(directory, filename);
                //if file found
                if(fb = fopen(directory, "rb"), fb != NULL){
                    printf("Server error: File found.\n");
                                printf("directory: %s\n", directory);

                    //send GET message reply to client as successful
                    send_get_reply(newSocket, 1);
                    printf("GET_Reply sent\n");

                            sleep(1);

                    //send file data
                    send_file_data(newSocket, fb);

                    fclose(fb);
                }
                else{ //if file not found
                  printf("Error from Server: File does not exist.\n");
                  //send GET message reply to client as failure
                  send_get_reply(newSocket, 0);
                  printf("GET_reply sent\n");
                }
            }
        break;

        /********************************
         *	Handle ls	*
         ********************************/
        case (char) 0xA5:
            printf("Server received ls request\n");
            if (!authenticated) { //double check authentication
                strcpy(send_message.payload, "Server - You are not authenticated!\n");
                send_message.length = 12 + strlen(send_message.payload);
                //send authentication error message to client until successful
                while( send(newSocket, (char*)(&send_message),send_message.length, 0) != send_message.length );

            } else { //if already authenticated send ls reply
                send_ls_reply(newSocket);
                printf("Server sent ls reply\n");
            }
        break;

        /********************************
         *	Handle Quit	*
         ********************************/
        case (char) 0xAB:
            printf("Disconnecting by request. (socket: %d)\n", newSocket);

            //create message
            send_message.protocol[0] = 0xe3;
            strcat(send_message.protocol, "myftp");
            send_message.type = 0xAC;
            send_message.length = 12;
            //continue sending until successful
            while( send(newSocket, (char*)(&send_message),send_message.length, 0) !=12 );
            close(newSocket); //close the socket
        break;

        /* Invalid Protocol */
        default:
            printf("Error from server: Invalid Protocol Received");
        break;
    }


}

/* SELECT() ------------------------------------------------------------------------ THEO */
void setnonblocking(sock_descriptor) int sock_descriptor; {
    int opts;

    opts = fcntl(sock_descriptor,F_GETFL);
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(sock_descriptor,F_SETFL,opts) < 0) {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
    return;
}
/* SELECT() ------------------------------------------------------------------------ THEO */
void handle_new_connection() { //handles new socket connection
    int listnum;         //current socket in list
    int connection; //Socket file descriptor for incoming connections


    int c = sizeof(struct sockaddr_in);
    //accept the socket
    connection = accept(sock_descriptor, (struct sockaddr *)&clientAddr, (socklen_t*)&c);
    if (connection < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    setnonblocking(connection); //unblock that socket connection
    //find listnum of newly connected socket
    for (listnum = 0; (listnum < 5) && (connection != -1); listnum ++)
        if (connected_socks[listnum] == 0) {
            printf("\nConnection accepted:   FD=%d; Slot=%d\n",
                   connection,listnum);
            connected_socks[listnum] = connection;
            connection = -1;
        }
    if (connection != -1) { //if no room left
        printf("\nNo room left for new client. Try again later.\n");
                  close(connection);
    }
}

/* SELECT() ------------------------------------------------------------------------ THEO */
void build_select_list() { //builds fd_set for select()
    int listnum;         //current socket in list

    //clear out fd_set called socks
    FD_ZERO(&socks);

    //adds fd socket to the fd_set so select() returns if connection comes in
    FD_SET(sock_descriptor,&socks);

    //loop through possible connections and add sockets to fd_set
    for (listnum = 0; listnum < 5; listnum++) {
        if (connected_socks[listnum] != 0) {
            FD_SET(connected_socks[listnum],&socks);
            if (connected_socks[listnum] > highsock)
                highsock = connected_socks[listnum];
        }
    }
}
/* SELECT() ------------------------------------------------------------------------ THEO */
void read_socks() {
    int listnum;        // Current item in connected_socks for for loops

     //check for listening sockets, accept new connection if socket is readable
    if (FD_ISSET(sock_descriptor,&socks))
        handle_new_connection();

    //run through all sockets to check for requests
    for (listnum = 0; listnum < 5; listnum++) {
        if (FD_ISSET(connected_socks[listnum],&socks)){
            handle_data(listnum);}
    }
}



int recv_new(int newSocket, void* buf, int buf_len){
    int n_left = buf_len;
    int n = 0;
    while (n_left > 0){
        if ((n = recv(newSocket, buf + (buf_len - n_left), n_left, 0)) < 0){
            if (errno == EINTR)
                n = 0;
            else
                return -1;
        } else if (n == 0){
            return 0;
        }
        n_left -= n;
    }
    return buf_len;
}



int main(int argc, char* argv[]){

	DIR *dir;
/* SELECT() ------------------------------------------------------------------------ THEO */
    int readsocks;
    struct timeval timeout;
/* SELECT() ------------------------------------------------------------------------ THEO */

	if( argc != 2 ) //need at least two command line arguments
		printf("\n\tUsage: %s [PORT_NUMBER]\n\n", argv[0]);
	else{

    //set server port number from command line (8888)
		if( server_port_num = atoi(argv[1]), server_port_num!=-1 && server_port_num>0 )
			printf("\nServer Awaiting connections from Port %d \n\n", server_port_num);
		else{
			printf("\n\tError: Port number is out-of-range\n\n");
			exit(-1);
		}

		if (dir = opendir("./filedir"), dir == NULL){
			printf("Error: ./filedir not accessable");
			exit(-1);
		} else {
			closedir(dir);
		}

		//server_port_num = 8888;
		printf("\nServer Awaiting connections from Port %d \n\n", server_port_num);

		// 1. Create a socket
	  sock_descriptor = socket(AF_INET, SOCK_STREAM, 0);
		if ( sock_descriptor == -1){ //check for success of creation
			printf("Server error: socket creation failed\n");
			exit(-1);
		} else {
			printf("Server error: socket creation successful %d \n", sock_descriptor);
		}

		// 2. Bind the socket
		setsockopt(sock_descriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(one));
        /* SELECT() ------------------------------------------------------------------------ THEO */
        setnonblocking(sock_descriptor);
        /* SELECT() ------------------------------------------------------------------------ THEO */
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = htonl(INADDR_ANY);
		server_address.sin_port = htons(server_port_num);
		if (bind(sock_descriptor, (struct sockaddr *)&server_address, sizeof(server_address)) < -1) {
			printf("\n\tError: Failed to bind\n\n");
			exit(-1);
		} else {
			printf("Server: Binding successful\n");
		}

		// 3. Put server in listening mode
		if (listen(sock_descriptor, 3) == -1){
			printf("Server error: failed to be put in listening mode");
			exit(-1);
		} else {
			printf("Server is Listening\n");
		}

/* SELECT() ------------------------------------------------------------------------ THEO */
        highsock = sock_descriptor; //set highest descriptor
        memset((char *) &connected_socks, 0, sizeof(connected_socks));
/* SELECT() ------------------------------------------------------------------------ THEO */
		// 4. Accept connections until quit

		while (1){

/* SELECT() ------------------------------------------------------------------------ THEO */
            build_select_list(); //build list
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            //use Select()
            readsocks = select(highsock+1, &socks, NULL , NULL , NULL);

            if (readsocks < 0) {
                perror("select");
                exit(EXIT_FAILURE);
            }
            if (readsocks == 0) {
                //no requests, just show server still waiting
                printf(".");
                fflush(stdout);
            } else //read and handle socket connections/requests
                read_socks();
        }
	}
}
