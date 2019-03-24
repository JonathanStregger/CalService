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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
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

    //connectToCalServ();
    char * request;
    while((request = generateRequest()) != NULL){
        printf(":%s", request);
        int i = 0, count = 0;
        for(; request[i] != 0; i++){
            if(request[i] == '\n')
                count++;
        }
        printf(":%d:\n", count);
        free(request);
    }

    return 1;
}

/*
 * Recieves a requests from the user for the server.  Checks for validity
 * before sending on to the calendar server.
 * 
 * Return:  A string that can be sent to the server.
 *              This string needs to be freed after use.
 */
char * generateRequest(){
    printf("Calserv available, enter a request:\n");
    int requestLine = 0; //track the line of the command for 
    int requestType = 0; //track request type 1:add, 2:delete, 3:save
    int lineIndex = 0; //track the index of the end of last line
    char *request = malloc(256); //max size of request is 255 characters
    char getChar = 0, prevChar = 0; //previous char to track double return
    //receive input until double return received
    int i = 0;
    for(; i < 255; i++){
        getChar = fgetc(stdin);
        request[i] = getChar; //save char to request

        //if user entered twice then input is complete.
        if(getChar == '\n' && prevChar == '\n'){
            request[i] = getChar;
            request[i + 1] = 0;
            break;
        }
        //Test the line of input if a newline detected
        if(getChar == '\n' && requestType != 3){
            //get the current line of data
            char * line = malloc(255);
            int j = 0;
            for(; j < 255 && request[lineIndex] != '\n'; j++, lineIndex++)
                line[j] = request[lineIndex];
            line[j] = 0;
            lineIndex++;
            int k, invalid; //counter for date/time and invalid flag
            switch(requestLine){
                case 0: //request line
                    //if exit is encountered, check if save is needed and then exit
                    if(strcmp(line, "exit") == 0){
                        free(request);
                        free(line);
                        return NULL;
                    }
                    //set requestType depending on the command given
                    if(strcmp(line, "add") == 0)
                        requestType = 1;
                    else if(strcmp(line, "delete") == 0)
                        requestType = 2;
                    else if(strcmp(line, "save") == 0)
                        requestType = 3;
                    else{
                        printf("Invalid command \"%s\" encountered. Please enter a new command.\n", line);
                        i = -1; //start at start of request again and overwrite previous bad command
                        requestLine--; //set back requestLine to take command again
                    }
                    break;
                case 1: //date
                    invalid = 0;
                    //date is fixed length: "yy-mm-dd"
                    for(k = 0; k < 9; k++){
                        switch(k){
                            //check correct dashes
                            case 2:
                            case 5:
                                if(line[k] != '-')
                                    invalid = 1;
                                break;
                            //check null terminated after yy-mm-dd, no extra characters
                            case 8:
                                if(line[k] != 0)
                                    invalid = 1;
                                break;
                            //check that any other of yy,mm,dd are numbers
                            default:
                                if(line[k] < 48 || line[k] > 57)
                                    invalid = 1;
                                break;
                        }
                    }
                    if(invalid != 0){
                        printf("Invalid date \"%s\".  Format is yy-mm-dd."
                            " Reenter date.%d:%d\n", line, lineIndex, invalid);
                        i -= (j + 1); //set line counter back the length of the line
                        lineIndex -= (j + 1);
                        requestLine--; //set back line counter to date
                    }
                    break;
                case 2: //time
                    invalid = 0;
                    //time is fixed length: "hh:mm-hh:mm"
                    for(k = 0; k < 12; k++){
                        switch(k){
                            //check for dash between times
                            case 5:
                                if(line[k] != '-')
                                    invalid = 1;
                                break;
                            //check for colons between hours and minutes
                            case 2:
                            case 8:
                                if(line[k] != ':')
                                    invalid = 1;
                                break;
                            //check for proper length of time string
                            case 11:
                                if(line[k] != 0)
                                    invalid = 1;
                                break;
                            //check that hours and minutes are numbers
                            default:
                                if(line[k] < 48 || line[k] > 57)
                                    invalid = 1;
                                break;
                        }
                    }
                    if(invalid != 0){
                        printf("Invalid time \"%s\".  Format is hh:mm-hh:mm."
                            " Reenter time.\n", line);
                        i -= j + 1; //set line counter back the length of the line
                        lineIndex -= j + 1;
                        requestLine--; //set back line counter to date
                    }
                    break;
                case 3: //note can be empty or many characters for add, but is not used for delete
                    if(requestType == 2){
                        printf("Delete does not require a note. Note removed from request.\n");
                        request[i - j] = 0; //terminate the string after the time
                        free(line);
                        return request; //return modified request
                    }
                    break;
                default:
                    printf("Too many lines of data encountered.\n\n");
                    return generateRequest();
            }
            requestLine++;
            free(line);
        }
        prevChar = getChar; //store inserted char for 
    }
    //Check that there wasn't just a full 255 chars on a single line
    if(requestLine == 0){
        printf("Invalid command encountered.  Please try again.\n");
        free(request);
        return generateRequest();
    }
    if(requestType == 3 && strcmp(request, "save\n\n") != 0){
        printf("Invalid save command format.  Please try again.\n");
        free(request);
        return generateRequest();
    }

    request[i] = 0; //null terminate string
    return request;
}

/*
 * Open a connection to the calendar server.
 */
void connectToCalServ(){
    //status: status of getaddrinfo
    //sockfd: the socket file descriptor
    int status, sockfd;
    struct addrinfo hints;
    struct addrinfo *servinfo;  // will point to the results

    memset(&hints, 0, sizeof hints); // ensure the struct is empty
    hints.ai_family = AF_UNSPEC;     // works with either IPv4 or IPv6
    hints.ai_socktype = SOCK_STREAM; // TCP stream socket
    
    // point servinfo to a linked list of 1 or more struct addrinfos with
    // the above specified specs and port 3721
    status = getaddrinfo(SERVER_IP, SERVER_PORT, &hints, &servinfo);
    if (status != 0) {
		fprintf(stderr, "Error occured getting address info: %s\n", gai_strerror(status));
		exit(1);
	}

    //open a socket with the connection info from getaddrinfo
    sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, servinfo->ai_protocol);
    if(sockfd == -1){
        fprintf(stderr, "Socket error occured.\n");
        exit(1);
    }
    
    //We don't need to bind since the port number on the client side can 
    //change dynamically without any issue for the server
    //open a connection on the socket
	if(connect(sockfd, servinfo->ai_addr, servinfo->ai_addrlen) == -1){
		fprintf(stderr, "Connect error occured.\n");
        close(sockfd);
		exit(1);
	}

    //Send messages from the user to the server until user enters exit
    char * request = NULL;
    void * buffer = malloc(255);
    int len = 0; //length of request
    while((request = generateRequest()) != NULL){
        //if the request was save-exit
        /*if(request != NULL && strcmp(request, "save-exit") == 0){
            strcpy(request, "save\n\n");
            //send save request and then exit
            if(send(sockfd, request, len, 0) > 0){
                //receive reply from server (max 255 length of 255)
                recv(sockfd, buffer, 255, 0);
                printf((char *)buffer); //print out reply from the servers
            }
            free(request);
            break;
        }*/

        //send request to server
        len = strlen(request); //get length fo request
        if(send(sockfd, request, len, 0) > 0){
            //receive reply from server (max 255 length of 255)
            recv(sockfd, buffer, 255, 0);
            printf((char *)buffer); //print out reply from the servers
        }
        else
            fprintf(stderr, "Send error. Please reenter request.\n");
        free(request);
    }
    close(sockfd); //close the socket
    free(buffer);

    printf("Goodbye!\n");

    freeaddrinfo(servinfo); // free the linked-list
}
