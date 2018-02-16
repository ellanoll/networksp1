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

struct message_s{
    char protocol[6];
    char type;      /* type (1 byte) */
    char status;        /* status (1 byte) */
    int length;     /* length (header + payload) (4 bytes) */
    char payload[1024]; /* payload */
} __attribute__ ((packed));


// Global variables
int serverPortNumber = 0;
int socketDescriptor = -1;
struct sockaddr_in serverAddr,clientAddr;
char one;

/* SELECT() ------------------------------------------------------------------------ THEO */
int connectlist[5];  /* Array of connected sockets so we know who
                      we are talking to */
fd_set socks;        /* Socket file descriptors we want to wake
                      up for, using select() */
int highsock;         /* Highest #'d file descriptor, needed for select() */
/* SELECT() ------------------------------------------------------------------------ THEO */


int send_open_conn_reply(int newSocket){
	struct message_s reply_message;

	reply_message.protocol[0] = 0xe3;
	strcat(reply_message.protocol, "myftp");
	reply_message.type = 0xA2;
	reply_message.status = 1;
	reply_message.length = 12;

	send(newSocket, (char*)(&reply_message),reply_message.length, 0);
	return 0;
}

int check_auth(char loginInfo[1024]){
	FILE *fp;
	char usrnameAndPw[1024];

	if(fp = fopen("access.txt","rb"), fp == NULL)
		return 0;
	else{
		while( fgets(usrnameAndPw, 1024, fp) != NULL ){
			usrnameAndPw[strlen(usrnameAndPw)-1] = '\0';
			printf("login info: %s\n", loginInfo);
			printf("login info length: %d\n", (int)strlen(loginInfo));
			printf("cmp info: %s\n", usrnameAndPw);
			printf("cmp info length: %d\n", (int)strlen(usrnameAndPw));
			printf("cmp result: %d\n", strcmp(loginInfo, usrnameAndPw));
			if( strcmp(loginInfo, usrnameAndPw) == 0 )
				// login info verified
				return 1;
		}
	}
	return 0;
}

int send_auth_reply(int newSocket, int status){
	struct message_s reply_message;
	reply_message.protocol[0] = 0xe3;
	strcat(reply_message.protocol, "myftp");
	reply_message.type = 0xA4;
	reply_message.status = status;
	reply_message.length = 12;
	send(newSocket, (char*)(&reply_message),reply_message.length, 0);
	return 0;
}

int list_dir_send_list_reply(int newSocket){

	return 0;
}

int send_get_reply(int newSocket, int status){

	return 0;
}

int sendn(int newSocket, const void* buf, int buf_len)
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


int recvn(int newSocket, void* buf, int buf_len){
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

int recv_file_data(int newSocket, char directory[]){

	return 0;
}

/*
 * This will handle connection for each client
 * */
/* SELECT() ------------------------------------------------------------------------ THEO */
void deal_with_data(int listnum) {
    // Get the socket descriptor
    int newSocket = connectlist[listnum];

/* SELECT() ------------------------------------------------------------------------ THEO */
    int readSize;
    int quit;
    struct message_s recv_message, send_message;
    int connected, authenticated;

    FILE *fb;
	char target_filename[1024];
	char directory[1024 + 10];

    authenticated = 0;
    connected = 0;
    quit = 0;


    // Get message from client
    memset(recv_message.payload, '\0', 1024);
    readSize = recv(newSocket, (char *)&recv_message, sizeof(struct message_s), 0);
    // Print type, for debugging
    printf("recv_message.type is %x\n", recv_message.type);
    // Analyze message
    switch( recv_message.type ){
        /********************************
         *	Handle OPEN_CONN_REQUEST	*
         ********************************/
        case (char) 0xA1:
            printf("Server - OPEN_CONN_REQUEST received\n");
            send_open_conn_reply(newSocket);
            printf("Server - OPEN_CONN_REPLY sent\n");
            connected = 1;
            FD_SET(connectlist[listnum],&socks);
            if (connectlist[listnum] > highsock)
                highsock = connectlist[listnum];

        break;

        /********************************
         *	Handle AUTH_REQUEST	*
         ********************************/
        case (char) 0xA3:
            printf("Server - AUTH_REQUEST received\n");
            recv_message.payload[recv_message.length-5] = '\0';
            printf("Server - Authenticating: %s\n",recv_message.payload);
            authenticated = check_auth(recv_message.payload);
            send_auth_reply(newSocket, check_auth(recv_message.payload));
            printf("Server - AUTH_REPLY sent\n");

            if( authenticated == 1)
                printf("\t(Server - Access Granted)\n");
            else
                printf("\t(Server - Access Failed)\n");

        break;

        /********************************
         *	Handle LIST_REQUEST	*
         ********************************/
        case (char) 0xA5:
            printf("Server - LIST_REQUEST received\n");
            if (authenticated) {
                list_dir_send_list_reply(newSocket);
                printf("Server - LIST_REPLY sent\n");
            } else {
                strcpy(send_message.payload, "Server - You are not authenticated!\n");
                send_message.length = 12 + strlen(send_message.payload);
                while( send(newSocket, (char*)(&send_message),send_message.length, 0) != send_message.length );
            }



        break;

        /********************************
         *	Handle GET_REQUEST	*
         ********************************/
        case (char) 0xA7:
            if (authenticated) {
                strncpy(target_filename, recv_message.payload,recv_message.length-12);
                target_filename[recv_message.length-12] = '\0';

                printf("target filename: %s\n", target_filename);
                printf("GET_REQUEST: %s\n", recv_message.payload);

                strcpy(directory, "./filedir/");
                strcat(directory, target_filename);
                if(fb = fopen(directory, "rb"), fb != NULL){
                    printf("Server - File found.\n");
                                printf("directory: %s\n", directory);

                    send_get_reply(newSocket, 1);
                    printf("GET_REPLY Sent!\n");

                            sleep(1);

                    send_file_data(newSocket, fb);

                    fclose(fb);

                } else {
                    printf("Error: Server - File does not exist.\n");
                    send_get_reply(newSocket, 0);
                    printf("GET_REPLY Sent\n");
                }
            } else {
                strcpy(send_message.payload, "Server - You are not authenticated!\n");
                send_message.length = 12 + strlen(send_message.payload);
                while( send(newSocket, (char*)(&send_message),send_message.length, 0) != send_message.length );
            }
        break;

        /********************************
         *	Handle PUT_REQUEST	*
         ********************************/
        case (char) 0xA9:
            if (authenticated){
                strncpy(target_filename, recv_message.payload,recv_message.length-12);
                target_filename[recv_message.length-12] = '\0';
                printf("target filename: %s\n", target_filename);
                printf("PUT_REQUEST: %s\n", recv_message.payload);

                printf("PUT_REQUEST Received\n");
                strcpy(directory, "./filedir/");
                strcat(directory, target_filename);
                memset(send_message.payload, '\0', 1024);
                send_message.protocol[0] = 0xe3;
                strcat(send_message.protocol, "myftp");
                send_message.type = 0xAA;
                send_message.length = 12;
                send(newSocket, (char*)(&send_message), send_message.length, 0);
                printf("PUT_REPLY Sent\n");
                recv_file_data(newSocket, directory);
            } else {
                strcpy(send_message.payload, "Server - You are not authenticated!\n");
                send_message.length = 12 + strlen(send_message.payload);
                while( send(newSocket, (char*)(&send_message),send_message.length, 0) != send_message.length );
            }
        break;

        /********************************
         *	Handle QUIT_REQUEST	*
         ********************************/
        case (char) 0xAB:
            printf("Disconnecting by request. (socket: %d)\n", newSocket);

            send_message.protocol[0] = 0xe3;
            strcat(send_message.protocol, "myftp");
            send_message.type = 0xAC;
            send_message.length = 12;
            while( send(newSocket, (char*)(&send_message),send_message.length, 0) !=12 );
            close(newSocket);
            quit = 1;
        break;

        /* Invalid Protocol */
        default:
            printf("Error: Server - Invalid Protocol Received");
            quit = 1;
        break;
    }


}
/* SELECT() ------------------------------------------------------------------------ THEO */
void setnonblocking(socketDescriptor) int socketDescriptor; {
    int opts;

    opts = fcntl(socketDescriptor,F_GETFL);
    if (opts < 0) {
        perror("fcntl(F_GETFL)");
        exit(EXIT_FAILURE);
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(socketDescriptor,F_SETFL,opts) < 0) {
        perror("fcntl(F_SETFL)");
        exit(EXIT_FAILURE);
    }
    return;
}
/* SELECT() ------------------------------------------------------------------------ THEO */
void handle_new_connection() {
    int listnum;         /* Current item in connectlist for for loops */
    int connection; /* Socket file descriptor for incoming connections */

    /* We have a new connection coming in!  We'll
     try to find a spot for it in connectlist. */
    int c = sizeof(struct sockaddr_in);
    connection = accept(socketDescriptor, (struct sockaddr *)&clientAddr, (socklen_t*)&c);
    if (connection < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    setnonblocking(connection);
    for (listnum = 0; (listnum < 5) && (connection != -1); listnum ++)
        if (connectlist[listnum] == 0) {
            printf("\nConnection accepted:   FD=%d; Slot=%d\n",
                   connection,listnum);
            connectlist[listnum] = connection;
            connection = -1;
        }
    if (connection != -1) {
        /* No room left in the queue! */
        printf("\nNo room left for new client.\n");
                  close(connection);
    }
}

/* SELECT() ------------------------------------------------------------------------ THEO */
void build_select_list() {
    int listnum;         /* Current item in connectlist for for loops */

    /* First put together fd_set for select(), which will
     consist of the sock veriable in case a new connection
     is coming in, plus all the sockets we have already
     accepted. */


    /* FD_ZERO() clears out the fd_set called socks, so that
     it doesn't contain any file descriptors. */

    FD_ZERO(&socks);

    /* FD_SET() adds the file descriptor "sock" to the fd_set,
     so that select() will return if a connection comes in
     on that socket (which means you have to do accept(), etc. */

    FD_SET(socketDescriptor,&socks);

    /* Loops through all the possible connections and adds
     those sockets to the fd_set */

    for (listnum = 0; listnum < 5; listnum++) {
        if (connectlist[listnum] != 0) {
            FD_SET(connectlist[listnum],&socks);
            if (connectlist[listnum] > highsock)
                highsock = connectlist[listnum];
        }
    }
}
/* SELECT() ------------------------------------------------------------------------ THEO */
void read_socks() {
    int listnum;         /* Current item in connectlist for for loops */

    /* OK, now socks will be set with whatever socket(s)
     are ready for reading.  Lets first check our
     "listening" socket, and then check the sockets
     in connectlist. */

    /* If a client is trying to connect() to our listening
     socket, select() will consider that as the socket
     being 'readable'. Thus, if the listening socket is
     part of the fd_set, we need to accept a new connection. */

    if (FD_ISSET(socketDescriptor,&socks))
        handle_new_connection();
    /* Now check connectlist for available data */

    /* Run through our sockets and check to see if anything
     happened with them, if so 'service' them. */

    for (listnum = 0; listnum < 5; listnum++) {
        if (FD_ISSET(connectlist[listnum],&socks))
            deal_with_data(listnum);
    } /* for (all entries in queue) */
}


int main(int argc, char* argv[]){

	DIR *dir;
/* SELECT() ------------------------------------------------------------------------ THEO */
    int readsocks;
    struct timeval timeout;  /* Timeout for select */
/* SELECT() ------------------------------------------------------------------------ THEO */

	if( argc != 2 )
		printf("\n\tUsage: %s [PORT_NUMBER]\n\n", argv[0]);
	else{

		if( serverPortNumber = atoi(argv[1]), serverPortNumber!=-1 && serverPortNumber>0 )
			printf("\n# Server Started: Awaiting connections from Port %d #\n\n", serverPortNumber);
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

		//serverPortNumber = 8888;
		printf("\n# Server Started: Awaiting connections from Port %d #\n\n", serverPortNumber);

		// 1. Create a socket
	    socketDescriptor = socket(AF_INET, SOCK_STREAM, 0);
		if ( socketDescriptor == -1){
			printf("Error: Server - Failed to create a socket\n");
			exit(-1);
		} else {
			printf("Server - Socket created %d \n", socketDescriptor);
		}

		// 2. Binding
		setsockopt(socketDescriptor, SOL_SOCKET, SO_REUSEADDR, (char*)&one, sizeof(one));
        /* SELECT() ------------------------------------------------------------------------ THEO */
        setnonblocking(socketDescriptor);
        /* SELECT() ------------------------------------------------------------------------ THEO */
		serverAddr.sin_family = AF_INET;
		serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
		serverAddr.sin_port = htons(serverPortNumber);
		if (bind(socketDescriptor, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < -1) {
			printf("\n\tError: Failed to bind\n\n");
			exit(-1);
		} else {
			printf("Server - Binding succeeded\n");
		}

		// 3. Put server in listening mode

		if (listen(socketDescriptor, 3) == -1){
			printf("Error: Server - Failed to be put in listening mode");
			exit(-1);
		} else {
			printf("Server - Listening\n");
		}

/* SELECT() ------------------------------------------------------------------------ THEO */
        highsock = socketDescriptor;
        memset((char *) &connectlist, 0, sizeof(connectlist));
/* SELECT() ------------------------------------------------------------------------ THEO */
		// 4. Accept connections

		while (1){

/* SELECT() ------------------------------------------------------------------------ THEO */
            build_select_list();
            timeout.tv_sec = 1;
            timeout.tv_usec = 0;

            readsocks = select(highsock+1, &socks, NULL , NULL , NULL);

            if (readsocks < 0) {
                perror("select");
                exit(EXIT_FAILURE);
            }
            if (readsocks == 0) {
                /* Nothing ready to read, just show that
                 we're alive */
                printf(".");
                fflush(stdout);
            } else
                read_socks();
        }
	}
/* SELECT() ------------------------------------------------------------------------ THEO */
}
