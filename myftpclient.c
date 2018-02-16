/*
    C ECHO client example using sockets
*/
#include "myftp.h"

char server_IP[1024];
int server_port_number;

char login_id[1024];
char login_pw[1024];

char target_filename[1024];

struct message_s{
    char protocol[6];
    char type;      /* type (1 byte) */
    char status;        /* status (1 byte) */
    int length;     /* length (header + payload) (4 bytes) */
    char payload[1024]; /* payload */
} __attribute__ ((packed));


int return_option(char str[]){
    if( strcasecmp(str,"open") == 0 )
        return 1;
    if( strcasecmp(str,"USER") == 0 )
        return 2;
    if( strcasecmp(str,"ls") == 0 )
        return 3;
    if( strcasecmp(str,"get") == 0 )
        return 4;
    if( strcasecmp(str,"put") == 0 )
        return 5;
    if( stcasecm(str, "PASS") == 0)
        return 6;

    return 0;
}

void send_open_conn_request(int newSocket){
    struct message_s send_message;
    send_message.protocol[0] = 0xe3;
    strcat(send_message.protocol, "myftp");
    send_message.type = 0xA1;
    send_message.length = 12;
    memset(send_message.payload, '\0', 1024);
    send(newSocket, (char *)&send_message, send_message.length, 0);
}

void send_auth_user_request(int newSocket){
    struct message_s send_message;
    send_message.protocol[0] = 0xe3;
    strcat(send_message.protocol, "myftp");
    send_message.type = 0xA3;
    send_message.length = 12;
    memset(send_message.payload, '\0', 1024);
    send(newSocket, (char *)&send_message, send_message.length, 0);
}

void send_auth_pass_request(int newSocket){
    struct message_s send_message;
    send_message.protocol[0] = 0xe3;
    strcat(send_message.protocol, "myftp");
    send_message.type = 0xA3;
    send_message.length = 12;
    memset(send_message.payload, '\0', 1024);
    send(newSocket, (char *)&send_message, send_message.length, 0);
}

int main(int argc , char *argv[])
{
    char client_command[1024];
    int newSocket = 0;
    struct sockaddr_in server;

    struct message_s send_message, recv_message;
    int quit;
    char* token;

    char temp_server_IP[1024];
    int temp_server_port_number;

    char temp_login_id[1024];
    char temp_login_pw[1024];

    FILE * fb;

    int connected = 0;
    int authenticated = 0;
    char *thread_safety;

    char recv_type;


    quit = 0;
    while( quit==0 ){

        printf("Client > ");
        fgets(client_command, 1024, stdin);
        client_command[strlen(client_command)-1]='\0';

        if (strcasecmp(client_command,"quit") != 0) {

            if( token = strtok_r(client_command," \n", &thread_safety), token == 0 ){
                printf("Error: Client - Invalid command!");
                continue;
            }
            if( connected == 0 && return_option(token)>1 ){
                printf("Error: Client - No connection established. Have you tried \"open\"?\n");
                continue;
            }
            if( return_option(token)>2 && authenticated == 0 ){
                printf("Error: Client - You haven't authenticated yet.\n");
                continue;
            }

            switch( return_option(token) ){
                /***********************
                 * Handle open command *
                 ***********************/
                case 1:
                    connected = 0;
                    // Eliminate first argument to see if there're any arguments left
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token == NULL ){
                        printf("Usage: open SERVER_IP PORT_NUMBER\n");
                        continue;
                    }
                    // Read the second argument, which is supposed to be IP address
                    strcpy(temp_server_IP, token);
                    // Eliminate second argument to if there's the third argument
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token == NULL ){
                        printf("Usage: open SERVER_IP PORT_NUMBER\n");
                        continue;
                    }
                    temp_server_port_number = atoi(token);
                    if( temp_server_port_number <= 0 ){
                        printf("Error: Client - Bad Port Number\n");
                        continue;
                    }
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token != NULL ){
                        printf("Usage: open SERVER_IP PORT_NUMBER\n");
                        continue;
                    }

                    strcpy(server_IP, temp_server_IP);
                    server_port_number = temp_server_port_number;
                    printf("Opening %s at port %d... ", server_IP, server_port_number);

                    // 1. Create socket
                    newSocket = socket(AF_INET , SOCK_STREAM , 0);
                    if (newSocket == -1){
                        printf("Error: Client - Could not create socket\n");
                    } else {
                        puts("Client - Socket created\n");
                    }

                    // 2. Configure Server
                    server.sin_addr.s_addr = inet_addr(server_IP);
                    server.sin_family = AF_INET;
                    server.sin_port = htons( server_port_number );

                    // 3. Connect to remote server
                    if (connect(newSocket , (struct sockaddr *)&server , sizeof(server)) < 0){
                        perror("Error: Client - Connection failed. ");
                        return 1;
                    } else {
                        puts("Client - Connected to server\n");
                    }

                    // Attempt to send OPEN_CONN_REQUEST
                    send_open_conn_request(newSocket);
                    printf("OPEN_CONN_REQUEST sent\n");

                    // Attempt to retrieve OPEN_CONN_REPLY
                    memset(recv_message.payload, '\0', 1024);
                    recv(newSocket, (char *)&recv_message, sizeof(struct message_s), 0);
                    if (recv_message.status == '0'){
                        printf("Error: Server connection failed.\n");
                    } else {
                        printf("Client - Server connection accepted.\n");
                        connected = 1;
                    }


                    break;

                /***********************
                 * Handle auth user command *
                 ***********************/
                case 2:
                    authenticated = 0;
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token == NULL ){
                        printf("Usage: auth USER_ID USER\n");
                        continue;
                    }
                    strcpy(temp_login_id,token);
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token == NULL ){
                        printf("Usage: auth USER_ID USER\n");
                        continue;
                    }
                    strcpy(temp_login_pw,token);
                    token = strtok_r(NULL," \n",&thread_safety);
                    if( token != NULL ){
                        printf("Usage: auth USER_ID USER\n");
                        continue;
                    }
                    strcpy(login_id, temp_login_id);
                    strcpy(login_pw, temp_login_pw);

                    /* Authenciation */
                    /* Prepare AUTH_REQUEST */
                    send_message.protocol[0] = 0xe3;
                    strcat(send_message.protocol, "myftp");
                    send_message.type = 0xA3;
                    send_message.length = 12 + strlen(login_id) + strlen(login_pw) + 1;
                    strcpy(send_message.payload, login_id);
                    strcat(send_message.payload, " ");
                    strcat(send_message.payload, login_pw);
                    while( send(newSocket, (char*)&send_message, send_message.length, 0) != 12 + strlen(login_id)+strlen(login_pw)+1);
                    printf("AUTH_REQUEST sent\n");

                    /* Waiting for AUTH_REPLY */
                    recv(newSocket, (char *)&recv_message,sizeof(struct message_s), 0);
                    printf("AUTH_REPLY received\n");
                    recv_type = 0xA4;
                    if (recv_message.type != recv_type){
                        printf("Error: Client - Wrong header received, terminating connection");
                        exit(-1);
                    }
                    if (recv_message.status == 1)
                    {
                        printf("User Verified.\n");
                    }
                    else
                    {
                        printf("Authentication FAILED.\n");
                        // Return IDLE state
                        close(newSocket);
                    }
                    break;

                /***********************
                 * Handle ls command *
                 ***********************/
                case 3:
                    if( token = strtok_r(NULL," \n", &thread_safety), token != NULL ){
                            printf("Usage: ls (no argument is needed)\n");
                            continue;
                    }
                    /* Sending LIST_REQUEST */
                    send_message.protocol[0] = 0xe3;
                    strcat(send_message.protocol, "myftp");
                    send_message.type = 0xA5;
                    send_message.length = 12;
                    while ( send(newSocket, (char*)&send_message, send_message.length, 0) != 12 );

                    /* Attempt to receive LIST_REPLY */
                    recv(newSocket, (char *)&recv_message,sizeof(struct message_s), 0);
                    if( recv_type = 0xA6, recv_type != recv_message.type){
                        printf("Error: Client - Wrong header received, terminating connection");
                        exit(-1);
                    }
                    printf("----- file list start -----\n");
                    printf("%s", recv_message.payload);
                    printf("----- file list end -----\n");


                    break;

                /***********************
                 * Handle get command *
                 ***********************/
                case 4:
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token == NULL ){
                        printf("Usage: get TARGET_FILENAME\n");
                        continue;
                    }
                    strcpy(target_filename, token);
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token != NULL ){
                        printf("Usage: get TARGET_FILENAME\n");
                        continue;
                    }
                    printf("Downloading \"%s\"...\n",target_filename);

                    // Sending GET_REQUEST
                    send_message.protocol[0] = 0xe3;
                    strcat(send_message.protocol, "myftp");
                    send_message.type = 0xA7;
                    strcpy(send_message.payload, target_filename);
                    send_message.length = 12 + strlen(target_filename);
                    send(newSocket,(char *)(&send_message),send_message.length, 0);

                    // Receiving GET_REPLY

                    recv(newSocket,(char *)(&recv_message),sizeof(struct message_s)+1, 0);
                    // For debug
                    //printf("Showing GET_REPLY info\n");
                    //printf("protocol: %s\n", recv_message.protocol);
                    //printf("type: %x\n", (int)recv_message.type);
                    //printf("status: %d\n", (int)recv_message.status);
                    //printf("length: %d\n", recv_message.length);
                    //printf("payload: %s\n", recv_message.payload);
                    recv_type = 0xA8;
                    if( recv_type != recv_message.type ){
                        printf("Error: Client - Wrong header received, terminating connection");
                        exit(-1);
                    }
                    if (recv_message.status == 1){
                        fb = fopen(target_filename, "wb");
                        if (fb != NULL){
                            download_file(newSocket, fb);
                            printf("File downloaded.\n");
                            fclose(fb);
                        } else
                            printf("Error: Client - Cannot download file.\n");
                    } else {
                        printf("File does not exist.\n");
                    }
                    break;

                /***********************
                 * Handle put command *
                 ***********************/
                case 5:
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token == NULL ){
                        printf("Usage: put SOURCE_FILENAME\n");
                        continue;
                    }
                    strcpy(target_filename, token);
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token != NULL ){
                        printf("Usage: put SOURCE_FILENAME\n");
                        continue;
                    }
                    printf("Uploading \"%s\"...\n",target_filename);

                    fb = fopen(target_filename, "rb");
                    if (fb != NULL){
                        printf("Sending PUT_REQUEST\n");
                        send_message.type = 0xA9;
                        send_message.protocol[0] = 0xe3;
                        strcat(send_message.protocol, "myftp");
                        memset(send_message.payload, '\0', 1024);
                        memcpy(send_message.payload, target_filename, strlen(target_filename));
                        send_message.length = 12 + strlen(target_filename);
                        send(newSocket,(char *)(&send_message),send_message.length, 0);
                        printf("PUT_REQUEST sent\n");

                        printf("Receiving PUT_REPLY\n");
                        memset(recv_message.payload, '\0', 1024);
                        recv(newSocket,(char *)(&recv_message),sizeof(struct message_s), 0);
                        if( recv_type = 0xAA, recv_type != recv_message.type){
                            printf("Error: Client - Wrong header received, terminating connection");
                            exit(-1);
                        }
                        printf("PUT_REPLY received\n");
                        upload_file(newSocket, fb);
                        fclose(fb);
                        printf("File uploaded.\n");
                    }
                    else
                        printf("Error: Client - File does not exist.");

                    break;


                /***********************
                 * Handle auth user command *
                 ***********************/
                case 6:
                    authenticated = 0;
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token == NULL ){
                        printf("Usage: auth PASSWORD\n");
                        continue;
                    }
                    strcpy(temp_login_id,token);
                    token = strtok_r(NULL," \n", &thread_safety);
                    if( token == NULL ){
                        printf("Usage: auth PASSWORD\n");
                        continue;
                    }
                    strcpy(temp_login_pw,token);
                    token = strtok_r(NULL," \n",&thread_safety);
                    if( token != NULL ){
                        printf("Usage: auth PASSWORD\n");
                        continue;
                    }
                    strcpy(login_id, temp_login_id);
                    strcpy(login_pw, temp_login_pw);

                    /* Authenciation */
                    /* Prepare AUTH_REQUEST */
                    send_message.protocol[0] = 0xe3;
                    strcat(send_message.protocol, "myftp");
                    send_message.type = 0xAE;
                    send_message.length = 12 + strlen(login_id) + strlen(login_pw) + 1;
                    strcpy(send_message.payload, login_id);
                    strcat(send_message.payload, " ");
                    strcat(send_message.payload, login_pw);
                    while( send(newSocket, (char*)&send_message, send_message.length, 0) != 12 + strlen(login_id)+strlen(login_pw)+1);
                    printf("PASS_REQUEST sent\n");

                    /* Waiting for AUTH_REPLY */
                    recv(newSocket, (char *)&recv_message,sizeof(struct message_s), 0);
                    printf("AUTH_REPLY received\n");
                    recv_type = 0xAF;
                    if (recv_message.type != recv_type){
                        printf("Error: Client - Wrong header received, terminating connection");
                        exit(-1);
                    }
                    if (recv_message.status == 1)
                    {
                        printf("User Verified.\n");
                        authenticated = 1;
                    }
                    else
                    {
                        printf("Authentication FAILED.\n");
                        // Return IDLE state
                        close(newSocket);
                    }
                    break;
                case 0:
                    /* otherwise */
                    printf("Error: Client - Bad Command\n");
                break;


            }

        } else {
            quit = 1;
        }
    }






    send_quit_request(newSocket, &connected);
    close(newSocket);
    printf("Thank you.\n");


    return 0;
}
