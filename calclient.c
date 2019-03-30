/* -------------------------------------------------------------------------
 * Student Names: Jonathan Stregger
 * Assignment 3: calclient source file
 * Lab Section: X01L, Winter 2019
 * Lab Instructor’s Name: Ron Meleshko
 * CMPT 361, Class Section: AS01, Winter 2019
 * Instructor's Name: Ron Meleshko
 * Purpose: 
 *    Connects to a calserv over TCP on port 12000.  Sends requests to add
 *    appointments, delete appointments, and save calendar of appointments.
 *    Requests are "add", "delete", "save", and "exit".
 *    Appointments are entered as follows:
 *    <request type>  (required)
 *    yy-mm-dd        (not required for save)
 *    hh:mm-hh:mm     (not required for save)
 *    notes           (not required for save/delete, optional for add)
 * 
 *    Requests must be followed by two newlines.
 * ---------------------------------------------------------------------- */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include "calclient.h"

//The port and IP to connect to.
//default to localhost:12000 for tests on student server
#define SERVER_IP "0.0.0.0"
#define SERVER_PORT "12000"

int main(int argc, char **argv){
  //Display use instructions for the client.
  printf("\nWelcome to the Calander Client\n\n"
    "Server port and address may be specified with the options:\n"
    "[-p portnumber] [-a address]\n"
    "Default port is 12000 and address 0.0.0.0\n\n"
    "Requests must be entered in the following form:\n"
    "Add Command\tDelete Command\tGet Command\tSave Command\n"
    "add\t\tdelete\t\tget\t\tsave\n"
    "date\t\tdate\t\tdate or index #\n"
    "time\t\ttime\t\ttime (optional)\n"
    "note (optional)\n\n"
    "Press the enter key an additional time after the last line of data.\n"
    "Type \"help\" to review format of requests\n"
    "Type \"reset\" to restart a request.\n"
    "Type \"exit\" to leave program.\n\n");

  char * port = SERVER_PORT;
  char * servIP = SERVER_IP;
  
  getOptions(argc, argv, &port, &servIP);

  connectToCalServ(port, servIP);

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
    if(getChar == '\n'){// && requestType < 3){
      //get the current line of data
      char line[255];
      memset(&line, 0, 255);
      int j = 0;
      for(; j < 255 && request[lineIndex] != '\n'; j++, lineIndex++)
        line[j] = request[lineIndex];
      line[j] = 0;
      lineIndex++;
      
      //if exit is encountered close client unless it is for the
      //note, which could be reminding them to close something
      if(requestLine != 3 && strcmp(line, "exit") == 0){
        free(request);
        return NULL;
      }
      //if help is encountered give help unless it is for the note.
      //maybe someone wants to schedule help for a move and is too
      //lazy to type more than help
      else if(requestLine != 3 && strcmp(line, "help") == 0){
        printf("\nAvailable commands:\n"
  "Requests must be entered in the following form:\n"
  "Add Command\tDelete Command\tGet Command\tSave Command\n"
  "add\t\tdelete\t\tget\t\tsave\n"
  "date\t\tdate\t\tdate or index #\n"
  "time\t\ttime\t\ttime (optional)\n"
  "note (optional)\n\n"
  "Press the enter key an additional time after the last line of data.\n"
  "Type \"help\" to review format of requests\n"
  "Type \"reset\" to restart a request.\n"
  "Type \"exit\" to leave program.\n\n");
        free(request);
        return generateRequest();
      }
      //reset the request unless it is for the note
      else if(requestLine != 3 && strcmp(line, "reset") == 0){
        free(request);
        printf("\n");
        return generateRequest();
      }
      
      //skip checks for save
      if(requestType == 3 || (requestType == 4 && strnlen(line,12) < 8 && (requestLine == 1 || requestLine == 2))){
        requestLine++;
        prevChar = getChar;
        continue;
      }
      
      //requestLine++;
      //continue; //skip checks for tests on server with bad values

      int k, invalid; //counter for date/time and invalid flag
      switch(requestLine){
        case 0: //request line
          //set requestType depending on the command given
          if(strcmp(line, "add") == 0)
            requestType = 1;
          else if(strcmp(line, "delete") == 0)
            requestType = 2;
          else if(strcmp(line, "save") == 0)
            requestType = 3;
          else if(strcmp(line, "get") == 0)
            requestType = 4;
          else{
            printf("Invalid command \"%s\" encountered. "
              "Please enter a new command.\n", line);
            i = -1; //restart request & overwrite bad command
            requestLine--; //set requestLine to take command again
            lineIndex -= j + 1;
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
              //check null terminated after yy-mm-dd, no extras
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
              " Reenter date.\n", line);
            i -= (j + 1); //set line counter to length of the line
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
            i -= j + 1; //set line counter back length of the line
            lineIndex -= j + 1;
            requestLine--; //set back line counter to date
          }
          break;
        //note can be empty or many chars for add, not used for delete
        case 3: 
          if(requestType == 2){
            printf("Delete does not require a note. "
              "Note removed from request.\n");
            request[i - j] = 0; //null terminate after the time
            return request; //return modified request
          }
          break;
        default:
          printf("Too many lines of data encountered.\n\n");
          return generateRequest();
      }
      requestLine++;
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

  request[i] = '\n';
  request[i+1] = 0; //null terminate string
  return request;
}

/*
 * Open a connection to the calendar server.
 */
void connectToCalServ(char *port, char *serverAddress){
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
  if ((status = getaddrinfo(serverAddress, port,
    &hints, &servinfo)) != 0) {
  fprintf(stderr, "Error occured getting address info: %s\n",
    gai_strerror(status));
  exit(1);
}

  //open a socket with the connection info from getaddrinfo
  if((sockfd = socket(servinfo->ai_family, servinfo->ai_socktype, 
      servinfo->ai_protocol)) == -1){
    fprintf(stderr, "Socket error occured: %s\n", gai_strerror(sockfd));
    exit(1);
  }
  
  //We don't need to bind since the port number on the client side can 
  //change dynamically without any issue for the server
  //open a connection on the socket
  int connErr = 0;
if((connErr = connect(sockfd, servinfo->ai_addr,
    servinfo->ai_addrlen)) == -1){
  fprintf(stderr, "Connect error occured: %s\n", gai_strerror(connErr));
  close(sockfd);
  exit(1);
}

  //Send messages from the user to the server until user enters exit
  char * request = NULL;

  //overflow test
  /*int test = 0;
  char testC[400];
  memset(&testC, 'a', 399);
  testC[399] = 0;*/

  int len = 0; //length of request
  while((request = generateRequest()) != NULL){
    //check for reset
    /*if(request[0] == 0){
      printf("\n");
      free(request);
      continue;
    }*/
    
    char buffer[255];
    memset(&buffer, 0, 255);
    //send request to server
    len = strlen(request); //get length fo request
    int sent = 0; //bytes sent
    int bytesSent = 0; //bytes sent on each send
    char * reqPt = request; //pointer to place in request

    //overflow test
    /*if(test == 0){
      test++;
      len = strlen(testC);
      reqPt = testC;
    }*/

    while(sent < len){
      //Attempt a send to the server
      //on failure to send, free memory, and go back to request
      bytesSent = send(sockfd, reqPt, len, 0);
      if(bytesSent < 0){
        fprintf(stderr,"Fatal send error! Please reenter request.\n");
        break;
      }
      sent += bytesSent; //adjust sent bytes total 
      reqPt += bytesSent; //move forward pointer so no repeat data
    }
    //receive reply from server (max 255 length of 255)
    int bytesRecv = 0; //bytes received in a recv
    int ttlRecv = 0; //total bytes received from server
    int maxRecv = 255; //track room left in the buffer to receive
    char * buffPt = buffer; //pointer to the receive buffer
    while(sent == len && strstr(buffer, "\n") == NULL && ttlRecv < 255){
      if((bytesRecv = recv(sockfd, (void *)buffPt, maxRecv, 0)) < 0){
        fprintf(stderr,"Fatal recv error! Please reenter request.\n");
        ttlRecv = 0;
        break;
      }
      ttlRecv += bytesRecv; //count number of bytes received
      maxRecv = bytesRecv; //decrease max for next receive
      buffPt += bytesRecv; //move buffer pointer forward
      printf("%s", buffPt);
    }
    //report reply if one was received.
    if(ttlRecv > 0)
      printf("Server reply: %s", buffer); //print out reply from server
    free(request);
  }
  close(sockfd); //close the socket

  printf("\nGoodbye!\n");

  freeaddrinfo(servinfo); // free the linked-list
}

/*
 * Handles command line arguements.
 *
 * Parameters:  Number of command line arguemnts
 *        Command line arguements
 *        Server IP
 *        Port number
 */
void getOptions(int argc, char **argv, char **port, char **serverIP){
  char prevOpt = 0;
  int i = 1;
  for(; i < argc; i++){
    //handle option if one was encountered
    if(prevOpt != 0){
      switch(prevOpt){
        //set port option
        case 'p':
          //check that port is in range
          if(atoi(argv[i]) < 1024)
            printf("Invalid socket value received.\n");
          else
            *port = argv[i];
          break;
        //set server ip option
        case 'a':
          *serverIP = argv[i];
          break;
        default:
          printf("Illegal option '%c' encountered. -h for help\n",
            argv[1][1]);
      }
      prevOpt = 0; //option handled, reset option
    }
    //if no option yet encountered check for one
    else if(argv[i][0] == '-' && argv[i][1] != 0){
      prevOpt = argv[i][1];
      //show help option if encountered
      if(prevOpt == 'h'){
        printf("Available options:"
          "\n\t-a <ip address>\n\t Specify server IP address."
          "\n\t-p <port number>\n\t Specicify port number."
          "\n\t-h\n\t Display this help dialog."
          "\n\nPress any key to continue...");
        getchar();
        printf("\n\n");
        prevOpt = 0; //option handled, reset option
      }
    }
    else
      printf("Invalid option encountered.  -h for help.\n");
  }
}