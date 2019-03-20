/* ---------------------------------------------------------------------------
 * Student Names: Jonathan Stregger
 * Assignment 3: calclient source file
 * Lab Section: X01L, Winter 2019
 * Lab Instructor’s Name: Ron Meleshko
 * CMPT 361, Class Section: AS01, Winter 2019
 * Instructor's Name: Ron Meleshko
 * Purpose: 
 *		Connects to a calserv over TCP on port 12000.  Sends requests to add
 *      appointments, delete appointments, and save calendar of appointments.
 *      Requests are "add", "delete", "save", and "exit".
 *      Appointments are entered as follows:
 *      <request type>  (required)
 *      yy-mm-dd        (not required for save)
 *      hh:mm-hh:mm     (not required for save)
 *      notes           (not required for save/delete, optional for add)
 * 
 *      Requests must be followed by two newlines.
 * ------------------------------------------------------------------------ */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "calclient.h"

#define SERVER_IP "10.0.2.10"
#define SERVER_PORT "12000"

int main(int argc, char **argv){
    //Display use instructions for the client.
    printf("/tWelcome to the Calander Client 4000 Extreme!\n"
    "Appointments can be added with the \"add\" request.\n"
    "Appointments can be deleted with the \"delete\" request.\n"
    "To save appointments on the server use the \"save\" request.\n"
    "Requests must be entered in the following form:\n"
    "<request>\nyy-mm-dd\nhh:mm-hh:mm\n"
    "Any note for the appointment on add, ignored for delete request.\n\n"
    "Press the enter key an additional time after the last line of data.\n"
    "Type \"exit\" to leave program.\n\n");

    return connectToCalServ();
}

/*
 * Recieves a requests from the user for the server.  Checks for validity
 * before sending on to the calendar server.
 * 
 * Return:  A string that can be sent to the server
 */
char * generateRequest(){
    printf("Calserv available, enter a request:\n");
    int requestLine = 0; //track the line of the command for 
    int requestType = 0; //track request type 1:add, 2:delete, 3:save
    int lineIndex = 0; //track the index of the end of last line
    char request[255]; //max size of request is 255 characters
    char getChar = 0, prevChar = 0; //previous char to track double return
    //receive input until double return received
    for(int i = 0; i < 254; i++){
        getChar = fgetc(stdin);
        //if user entered twice then input is complete.
        if(getChar == '\n' && prevChar == '\n'){
            request[i] = getChar;
            request[i + 1] = 0;
            break;
        }
        request[i] = getChar; //save char to request
        //Test the line of input if a newline detected
        if(getChar == '\n'){
            //get the current line of data
            char line[255];
            int j = lineIndex;
            for(; j < 255 && request[j] != '\n'; j++)
                line[j] = request[j];
            line[j] = 0;
            switch(requestLine){
                case 0: //request type
                    if(strcmp(line, "add") == 0)
                        requestType = 1;
                    else if (strcmp(line, "delete") == 0)
                        requestType = 2;
                    else if (strcmp(line, "save") == 0)
                        requestType = 3;
                    //if exit is encountered, check if save is needed and then exit
                    else if(strcmp(line, "exit")){
                        char save[4];
                        printf("Save before exit? (yes/no): ");
                        fgets(save, 3, stdin);
                        if(strcmp(save, "yes")){
                            strcpy(line, "save-exit"); //special double command
                            return line;
                        }
                        return "exit\n\n";
                    }
                    else {
                        printf("Invalid command \"%s\" encountered.\n", line);
                        return generateRequest();
                    }
                case 1: //date
                    int invalidDate = 0;
                    //date is fixed length: "yy-mm-dd\n"
                    for(int k = 0; k < 9; k++){
                        if((k == 2 || k == 5 ) && line[k] != '-')
                            invalidDate = 1;
                        else if(k = 8 && line[k] != '\n')
                            invalidDate = 1;
                        else if(line[k] < 48 && line[k] > 57)
                            invalidDate = 1;
                    }
                    if(invalidDate != 0){
                        printf("Invalid date \"%s\".  Format is yy-mm-dd."
                            " Reenter date.\n", line);
                        i -= j; //set line counter back the length of the line
                        requestLine--; //set back line counter to date
                    }
                    break;
                case 2: //time
                    int invalidTime = 0;
                    //time is fixed length: "hh:mm-hh:mm\n"
                    for(int k = 0; k < 12; k++){
                        if(k == 5 && line[k] != '-')
                            invalidTime = 1;
                        else if(k = 11 && line[k] != '\n')
                            invalidTime = 1;
                        else if((k == 2 || k == 8) && line[k] != ':')
                            invalidTime = 1;
                        else if(line[k] < 48 && line[k] > 57)
                            invalidTime = 1;
                    }
                    if(invalidTime != 0){
                        printf("Invalid time \"%s\".  Format is hh:mm-hh:mm."
                            " Reenter time.\n", line);
                        i -= j; //set line counter back the length of the line
                        requestLine--; //set back line counter to date
                    }
                    break;
                case 3: //note
                    break; //note can be empty
                default:
                    printf("Too many lines of data encountered.\n\n");
                    return generateRequest();
            }
            lineIndex = i + 1;
            requestLine++;
        }
        prevChar = getChar;
    }

}

/*
 * Open a connection to the calendar server.
 */
void connectToCalServ(){
    //status: status of getaddrinfo
    //sockfd: the socket file descriptor
    //connfd: the connect fil descriptor
    int status, sockfd, connfd;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results

    memset(&hints, 0, sizeof hints); // ensure the struct is empty
    hints.ai_family = AF_UNSPEC;     // works with either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream socket
    
    struct sockaddr_in sa;
    inet_pton(AF_INET, SERVER_IP, &(sa.sin_addr)); 
    // point servinfo to a linked list of 1 or more struct addrinfos with
    // the above specified specs and port 3721
    status = getaddrinfo(sa.sin_addr, SERVER_PORT, &hints, &servinfo);
    if (status != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

    //open a socket with the connection info from getaddrinfo
    sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if(sockfd == -1){
        frprintf(stderr, "socket error: %s\n", strerror(errno));
        exit(1);
    }
    
    //We don't need to bind since the port number on the client side can 
    //change dynamically without any issue for the server
    //open a connection on the socket
	if(connect(sockfd, res->ai_addr, res->ai_addrlen) == -1){
		fprintf(stderr, "connect error: %s\n", strerror(errno));
        close(sockfd);
		exit(1);
	}

    //Send messages from the user to the server until user enters exit
    char * request = "";
    void * buffer = malloc(255);
    int len = 0; //length of request
    while(strcmp(request, "exit") != 0){
        request = generateRequest();
        if(request != NULL && strcmp(request, "save-exit") == 0){
            strcpy(request, "save\n\n");
            break;
        }

        //send request to server
        len = strlen(request); //get length fo request
        if(send(sockfd, request, len, 0) > 0){
            //receive reply from server (max 255 length of 255)
            recv(sockfd, buffer, 255, 0);
            printf((char *)buffer); //print out reply from the servers
        }
        else
            fprintf(stderr, "Send error. Please reenter request.\n");

    }
    close(sockfd); //close the socket
    free(buffer);

    printf("Goodbye!\n");

    freeaddrinfo(servinfo); // free the linked-list
}