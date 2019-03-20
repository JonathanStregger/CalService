/* ---------------------------------------------------------------------------
 * Student Names: Jonathan Stregger
 * Assignment 3: calserv program code
 * Lab Section: X01L, Winter 2019
 * Lab Instructor’s Name: Ron Meleshko
 * CMPT 361, Class Section: AS01, Winter 2019
 * Instructor's Name: Ron Meleshko
 * Purpose: 
 *		Manages a calender server of appointments for a client.
 *		Clients may add, modify, or delete appointments. Success or failure
 *		of client actions on serverdata will be reported to the client upon
 *		completion or failure.
 *			Format of data received from client is yy=mm=dd;hh:mm-hh-mm;note.
 *		Sockets are managed to allow the client to connect over port 12000.
 *		Client data is stored in a binary file named calfile of calendar data
 *		Upon exit, the previous file will be renamed and a new file of
 *		calendar data will be written before deleting the old data.
 * References:
 * 		https://beej.us/guide/bgnet/html/single/bgnet.html
 * ------------------------------------------------------------------------ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include "calserv.h"

int main(int argc, char **argv){
	int portNumber = 0;
	calendar *clientData = NULL;
	char filename[255];
	filename[0] = 0;
	
	//handle command line arguements
	getOptions(argc, argv, &portNumber, filename);
	
	//if no port chosen, default to port 12000
	if(portNumber == 0)
		portNumber = 12000;
	//if no filename option was used to set th filename to the default calfile
	if(filename[0] == 0)
		strcpy(filename, "calfile");
	//Load calendar data from file
	clientData = loadCalendarData("calfile");
	
	//Open a socket for connections
	openConnection(portNumber);
	if(errorCode != 1){
		printf("Fatal error #%d: Socket not be opened.", errorCode);
		freeCalendar(clientData);
		exit(1); //No save is necissary
	}

	//save, release memory, and exit
	if(clientData->added > 0){
		int i = saveCalendarData(clientData);
		freeCalendar(clientData);
		return i;
	}

	return 1;
}

/*
 * Opens a socket for connection.
 *
 * Parameters:	The port to open a socket on
 */
void openConnection(int port, calendar caldata){
	//status: status of getaddrinfo
	//sockfd: the socket file descriptor
	//aptfd: the accept file descriptor
	int status, sockfd, aptfd;
	//hints is the specs for the address
	//servinfo holds the results of getaddrinfo
	struct addrinfo hints, *servinfo;
	//hold the address of the client that connects to the server
	struct sockaddr_storage client_addr;
    socklen_t addr_size;

	memset(&hints, 0, sizeof hints); // make sure the struct is empty
	hints.ai_family = AF_UNSPEC;     // don't care IPv4 or IPv6
	hints.ai_socktype = SOCK_STREAM; // TCP stream sockets
	hints.ai_flags = AI_PASSIVE;     // fill in my IP for me

	// point servinfo to a linked list of 1 or more struct addrinfos with 
	// the above specified specs and the port specified in the call
	if ((status = getaddrinfo(NULL, port, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(status));
		exit(1);
	}

	//open a socket with the connection info from getaddrinfo
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if((sockfd == -1){
        frprintf(stderr, "socket error: %s\n", strerror(errno));
        exit(1);
    }

	//bind the socket to the program so that the client can find the server
	if(bind(sockfd, res->ai_addr, res->ai_addrlen) == -1){
		fprintf(stderr, "binding error on socket: %s\n", strerror(errno));
		close(sockfd);
		exit(1);
	}

	//listen on the connection on the socket
	if(listen(sockfd, 1) == -1){
		fprintf(stderr, "listen error: %s\n", strerror(errno));
		close(sockfd);
		exit(1);
	}

	//accecpt connections to the server
	addr_size = sizeof client_addr;
	if((aptfd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_size)) == -1){
		fprintf(stderr, "accept error: %s\n", strerror(errno));
		close(sockfd);
		exit(1);
	}

	//interact with client until disconnect signal
	char request[255], reply[24]; //max receive from client is 255 bytes, biggest reply is 23 + null terminator
    //receive request from client (max 255 length of 255)
	while(recv(sockfd, (void *)request, 255, 0) != 0)){
		//use the request to execute commands.  I
		switch(parseCommand(request, caldata)){
			case 1:
				strcpy(reply, "404 command not found\n");
				break;
			case 2:
				strcpy(reply, "200 success\n");
				break;
			case 3:
				strcpy(reply, "202 failed to add\n");
				break;
			case 4:
				strcpy(reply, "300 deleted\n");
				break;
			case 5:
				strcpy(reply, "302 not found\n");
				break;
			case 6:
				strcpy(reply, "100 file saved to file\n");
				break;
			case 7:
				strcpy(reply, "102 unable to save\n");
				break;
			default:
				fprintf(stderr, "fatal error in parse\n");
				close(aptfd);
				close(ockfd);
				exit(1);
		}

		//send reply to client
        len = strlen(reply); //get length fo reply
        while(bytes_sent = send(sockfd, reply, len, 0) == 0 && len > 0);
	}
	
	close(aptfd);
	close(sockfd);

	freeaddrinfo(servinfo); // free the linked-list
}

/*
 * Loads calendar data from calfile.  If the file cannot be found one is 
 * created.
 *
 * Parameters:	The filename to load from.
 * Return: A calendar with appointments from calfile.
 */
calendar* loadCalendarData(char * filename){
	printf("Loading calendar from file calfile\n");
	calendar *calData = (calendar *) malloc(sizeof(calendar));
	if(calData == NULL){
		printf("malloc failure in allocating calendar.\n");
		exit(EXIT_FAILURE);
	}
	calData->added = 0;
	calData->filename = filename;

	//start with room for 10 appointments
	calData->appList = (appointment **) malloc(sizeof(appointment *) * 10);
	if(calData->appList == NULL){
		printf("malloc failure in allocating appointment list.\n");
		exit(EXIT_FAILURE);
	}
	for(int i = 0; i < 10; i++)
		calData->appList[i] == NULL;

	FILE *calFile = fopen(calData->filename, "r");
	//If no file is found, then there has not been 
	if(calFile == NULL){
		return calData;
	}
	char getChar = 0;
	//Get a line from the file up to 255 characters long.
	//Each line corresponds to one appointment.
	char line[255];
	while(getChar != -1){
		getChar = fgetc(calFile);
		int i;
		for(i = 0; getChar != -1 && getChar != '\n'; i++){
			//only add first 255 characters
			if(i < 255)
				line[i] = getChar;
			getChar = fgetc(calFile);
		}
		line[i] = 0; //Null terminate the string.

		//Add the appointment to the calendar and increment the calendar size
		//if(line[0] != 0){
		if(i > 20){
			printf("\nLoading apppintment\n");
			//Try to add appointment to calendar
			if(addAppointment(makeAppointment(line), calData) != 1)
				printf("Appointment not loaded.\n");
			else{
				calData->added++;
				if(calData->added == calData->max){
					calData->max *= 2;	//double size
					realloc(calData->appList,
						sizeof(appointment *) * calData->max);
				}
			}
		}
	}
	if(calData->added > 0)
		printf("\nCalendar loaded into memory.\n\n");
	else 
		printf("\nNo appointments loaded.\n\n");
	fclose(calFile);
	return calData;
}

/*
 * Parses a command for the server to execute.
 *
 * Parameters:	The string to pars the command from.
 *				The calendar to execute the command on.
 * Return:	0 : Bad parameter(s) encountered
 * 			1 : Bad command encountered
 * 			2 : Add success
 * 			3 : Add failure, illegal data given
 * 			4 : Delete success
 * 			5 : Delete failure, appointment not found
 * 			6 : Save completed
 * 			7 : Unable to save
 */
int parseCommand(char *cmdLine, calendar *cal){
	if(cmdLine == NULL || cal == NULL)
		return 0;

	char cmd[7]; //max length command is delete
	int i = 0;
	for(; i < 7; i++){
		if(cmdLine[i] != 0 && cmdLine[i] != ' ')
			cmd[i] = cmdLine[i];
		else if(cmdLine[i] == 0)
			return 4; //command had no data
		//if ' ' reached, full command received
		else
			break;
	}
	cmd[i] = 0; //null terminate
	char *app = cmdLine + i; //get the app section after the command

	//for add command, attempt to add appointment
	if(strcmp(cmd, "add") == 0){
		return addAppointment(makeAppointment(app), cal);
	}
	//for delete command, delete the appointment
	else if(strcmp(cmd, "delete")){
		//make an appointment to compare with calendar records
		appointment *newApp = makeAppointment(app); 
		int result = removeAppointment(newApp, cal);
		free(newApp); //free the copy calendar
		return result; //return result of deletion attempt
	}
	else if(strcmp(cmd, "save")){
		return saveCalendarData(cal);
	}
	return 1; //command not recognized
}

/*
 * Makes an appointment from a string
 *
 * Parameters:	A string containing appointment data to parse.
 * Return:	If the string was a valid appointment, then an appointment
 *			will be returned.  Otherwise, a NULL pointer will be returned.
 */
appointment * makeAppointment(char *app){
	if(app == NULL)
		return NULL;

	if(testAppointment(app) == 0){
		printf("Failed. %s\n", app);
		return NULL;
	}
	printf("Passed!\n");

	//find the first open spot in the appList
	appointment *addApp = malloc(sizeof(appointment));
	char *p;
	//Get the date from the line of format yy-mm-dd
	addApp->date = (char *) malloc(sizeof(char)*9);
	if(addApp->date == NULL){
		printf("malloc error in add date to appointment.\n");
		exit(EXIT_FAILURE);
	}
	//increment appointment along with the holder pointer for date this will
	//track with values to be pulled out for time and note and skip the ';'
	for(p = addApp->date; *app != 59; p++, app++)
		*p = *app;
	*p = 0; //null terminate string
	printf("Date: %s\n", addApp->date);
	app++; //pass ';'
	
	//Get the time from the line of format hh:mm-hh-mm
	addApp->time = (char *) malloc(sizeof(char)*12);
	if(addApp->time == NULL){
		printf("malloc error in add time to appointment.\n");
		exit(EXIT_FAILURE);
	}
	for(p = addApp->time; *app != 59; p++, app++)
		*p = *app;
	*p = 0; //null terminate string
	printf("Time: %s\n", addApp->time);
	app++; // pass ';'

	//Get the note from the line of unknown format and length
	int i = 1; //find length of the note
	for(char *q = app; *q != 0; q++, i++){}
	if(i == 0){
		addApp->note = (char *) malloc(sizeof(char)*2);
		if(addApp->note == NULL){
			printf("\nmalloc error in add note to appointment.\n");
			exit(EXIT_FAILURE);
		}
		*addApp->note = ' '; //only a space if no note found
		*(addApp->note + 1) = 0; //null terminate
	}
	else{
		addApp->note = (char *) malloc(sizeof(char)*i);
		if(addApp->note == NULL){
			printf("malloc error in add note to appointment.\n");
			exit(EXIT_FAILURE);
		}
		for(p = addApp->note; *app != 0; p++, app++){
			*p = *app;
		}
		*p = 0; //null terminate string
	}
	printf("Note: %s\n", addApp->note);
	return addApp;
}

/*
 * Adds an appointment to the calendar.
 *
 * Parameters:	The appointment to add to the calendar.
 *				The calendar to add the appointment to.
 * Returns 1 if added to calednar, 2 if improper formatting found.
 */
int addAppointment(appointment *addApp, calendar* cal){
	if(addApp == NULL || cal == NULL)
		return 0;
	int added = 2;
	//Put the appointment in the first empty slot in cal->appList
	for(int i  = 0; i < cal->max; i++)
		if(cal->appList[i] == NULL){
			cal->appList[i] = addApp;
			added = 1;
		}
	if(added == 1)
		printf("Add complete.\n");
	return added;
}

/*
 * Removes the appointment from the calendar.
 *
 * Parameters:	The appointment to remove.
 *				The calendar to remove the appointment from.
 * Returns 4 if appointment could be found and removed, 5 if it was not.
 */
int removeAppointment(appointment *app, calendar *cal){
	if(app == NULL || cal == NULL)
		return 5;

	//check through list of appointments for appointment to be removed
	//and remove it when found.
	for(int i = 0; i < cal->max; i++){
		if(equalsApp(app, cal->appList[i]) == 1){
			free(cal->appList[i]);
			cal->appList[i] = NULL;
			printf("Appointment removed.\n");
			return 4;
		}
	}
	
	printf("Appointment not found.\n"); //app not found in list
	return 5;
}

/*
 * Checks to see if the appointments have the same time and date. Notes are
 * not compared.
 *
 * Parameters:	The two appointments to compare.
 * Return:	1 if the appointments are equal, 0 if they are not.
 */
int equalsApp(appointment *app1, appointment *app2){
	if(app1 == NULL || app2 == NULL)
		return 0;

	int i = 0;
	//check date
	for(; i < 8; i++)
		if((app1->date + i) != (app2->date + i))
			return 0;
	//check time
	for(i = 0; i < 12; i++)
		if((app1->time + i) != (app2->time + i))
			return 0;
	return 1;
}

/*
 * Checks to see if the appointments have the same time and date. Notes are
 * not compared.
 *
 * Parameters:	The two appointments to compare.
 * Return:	1 if the appointments are equal, 0 if they are not.
 */
int testAppointment(char *app){
	if(app == NULL)
		return 0;
	//check for type in yy-mm-dd;hh:mm-hh-mm;note
	if(isNum(app[0]) == 1 && isNum(app[1]) == 1 && 		//yy
		isNum(app[3]) == 1 && isNum(app[4]) == 1 && 	//mm
		isNum(app[6]) == 1 && isNum(app[7]) == 1 && 	//dd
		app[2] == 45 && app[5] == 45 &&	app[8] == 59 &&	//'-' '-' ';'
		isNum(app[9]) == 1 && isNum(app[10]) == 1 &&	//hh
		app[11] == 58 && app[14] == 45 && 				//':' '-'
		isNum(app[12]) == 1 && isNum(app[13]) == 1 &&	//mm
		isNum(app[15]) == 1 && isNum(app[16]) == 1 &&	//hh
		isNum(app[18]) == 1 && isNum(app[19]) == 1 &&	//mm
		app[17] == 58 && app[20] == 59){				//':' ';'
		//check values in each field
		//first field yy can be anything, check for leap year
		char num[3] = {app[0], app[1], 0}; //null terminate
		int leapYear = atoi(num);
		//If the year is a leap year, leapYear is 0, else 1->3
		//assume appointments are for the future, so always after 2000
		leapYear = leapYear % 4;

		num[0] = app[3]; //get month
		num[1] = app[4];
		int month = atoi(num);
		num[0] = app[6]; //get day
		num[1] = app[7];
		int day = atoi(num);
		//check for day length per month.  Also checks for month validity
		switch(month){
			case 1:  //January 31 days
			case 3:  //March 31 days
			case 5:  //May 31 days
			case 7:  //July 31 days
			case 8:  //August 31 days
			case 10: //October 31 days
			case 12: //December 31 days
				if(day > 31 || day < 1){
					printf("Illegal day value. ");
					return 0;
				}
				break;
			case 2: //Februrary 28 days, 29 on leap year
				if((leapYear == 0 && (day > 29 || day < 1)) ||
					(leapYear != 0 && (day > 28 || day < 1))){
					printf("Illegal day value. ");
					return 0;
				}
				break;
			case 4:  //April 30 days
			case 6:  //June 30 days
			case 9:  //September 30 days
			case 11: //November 30 days
				if(day > 30 || day < 1){
					printf("Illegal day value. ");
					return 0;
				}
				break;
			//If not above, then month out of range.
			default:
				return 0;
		}
		//check for hour between 1 and 24
		num[0] = app[9];
		num[1] = app[10];
		int time = atoi(num);
		if(time > 24 || time < 0){
			printf("Illegal hour value. ");
			return 0;
		}
		//check for minutes between 0 and 59
		num[0] = app[12];
		num[1] = app[13];
		time = atoi(num);
		if(time > 59 || time < 0){
			printf("Illegal minute value. ");
			return 0;
		}
		//check 2nd hour between 1 and 24
		num[0] = app[15];
		num[1] = app[16];
		time = atoi(num);
		if(time > 24 || time < 0){
			printf("Illegal hour value. ");
			return 0;
		}
		//check 2nd minute between 0 and 59
		num[0] = app[18];
		num[1] = app[19];
		if(time > 59 || time < 0){
			printf("Illegal minute value. ");
			return 0;
		}

		//all tests pass, return 1
		return 1;
	}
	printf("Format failed. ");
	return 0;
}

/*
 * Checks to see if the appointments have the same time and date. Notes are
 * not compared.
 *
 * Parameters:	The two appointments to compare.
 * Return:	1 if the appointments are equal, 0 if they are not.
 */
int isNum(char num){
	if(num > 47 && num < 58)
		return 1;
	return 0;
}

/*
 * Frees all allocated memory associated with the given calendar.
 *
 * Parameters:	The calendar to free the allocated memory for.
 */
void freeCalendar(calendar *cal){
	if(cal == NULL)
		return;

	//free all memory for each appointment's strings and the appointment
	for(int i = 0; i < cal->max && cal->appList[i] != NULL; i++){
		free(cal->appList[i]->date);
		free(cal->appList[i]->time);
		free(cal->appList[i]->note);
		free(cal->appList[i]);
	}
	free(cal->appList);
	free(cal);
}

/*
 * Saves the calendar data to calfile.
 *
 * Paramenters:	The calendar of all appointments
 * Return: 6 if successful. 7 if unsuccessful.
 */
int saveCalendarData(calendar *calData){
	if(calData == NULL)
		return 7;

	//Create the move and delete file system commands.
	char mvCmd[255], rmCmd[255], newFile[255];
	sprintf(mvCmd, "mv %s1 %s", calData->filename, calData->filename);
	sprintf(rmCmd, "rm %s", calData->filename);
	sprintf(newFile, "%s1", calData->filename);

	FILE *newCal = fopen(newFile,"w"); //open new file for write
	if(newCal == NULL){
		printf("Error creating new calFile. No changes made.");
		return 7;
	}
	//Write all appointments to the file.
	for(int i = 0; i < calData->max && calData->appList[i] != NULL; i++){
		char *line = appointmentToString(calData->appList[i]);
		fputs(line, newCal);
		fflush(newCal);
		free(line);
	}
	fclose(newCal); //close new calfile
	system(rmCmd); //delete old calfile
	system(mvCmd); //rename calfile in case of write failure
	printf("Calendar written to file.\n");
	return 6;
}

/*
 * Converts an appointment to a string: "yy-mm-dd;hh:mm-hh-mm;note\n"
 *
 * Parameters:	The appointment to convert to string.
 * Return:	A string representation of the given appointment.
 */
char * appointmentToString(appointment *app){
	if(app == NULL)
		return NULL;

	//Get the length of the appointment
	int appLength = 22; //base length with not note is 21 + \n
	for(char *p = app->note; *p != 0; p++, appLength++){}
	char *line = (char *) malloc(sizeof(char)*appLength);
	if(line == NULL){
		printf("malloc failure during write to file.\n");
	}
	char *add = line;
	//add the date to the line
	for(char *p = app->date; *p != 0; p++, add++)
		*add = *p;
	*add = 59; //add ';'
	add++;
	//add the time to the line
	for(char *p = app->time; *p != 0; p++, add++)
		*add = *p;
	*add = 59; //add ';'
	add++;
	//add the note to the line
	for(char *p = app->note; *p != 0; p++, add++)
		*add = *p;
	*add = 10; //add '\n'
	add++;
	*add = 0; //null termainate
	return line;
}

/*
 * Handles command line arguements.
 *
 * Parameters:	Number of command line arguemnts
 *				Command line arguements
 *				Port number
 *				Filename
 */
void getOptions(int argc, char **argv, int *portNumber, char *filename){
	char prevOpt = 0;
	for(int i = 1; i < argc; i++){
		//handle option if one was encountered
		if(prevOpt != 0){
			switch(prevOpt){
				case 'p':
				//if a non number is encountered portNumber will be 0
				*portNumber = atoi(argv[i]);
				//check portNumber, 
				if(*portNumber == 0){
					printf("Invalid socket value received.\n");
					exit(EXIT_FAILURE);
				}
				break;
				//set filename option
				case 'f':
					strcpy(filename, argv[i]);
					int len = 0;
					for(;filename[len] != 0; len++){}
					if(len > 250){
						printf("Specified filename is too long. "
							"Using default 'calfile'.\n");
						strcpy(filename, "calfile");
					}
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
					"\n\t-p <port number>\n\t Specicify port number."
					"\n\t-f <filename>\n\t Specify server calendar file."
					"\n\t-h\n\t Display this help dialog."
					"\n\t-d\n\t Display program discription."
					"\n\nPress any key to continue...");
				getchar();
				printf("\n\n");
				prevOpt = 0; //option handled, reset option
			}
			else if(prevOpt == 'd'){
				printf("\n\tCalendar Server 4000 Extreme!!!\n\n"
	"Manages a calender server of appointments for a client.\n"
	"Clients may add, modify, or delete appointments. Success or failure of\n"
	"client actions on server data will be reported to the client upon\n"
	"completion or failure.\n"
	"Port and filename for calendar may be specified with the options:\n"
	"[-p portnumber] [-f filename]\n"
	"Default port is 12000. Default file name is calfile.\n"
	"Simple help is provied with the -h option\n"
	"Format of commands received from client are:\n"
	"Add Command\tDelete Command\tSave Command\n"
	"add\t\tdelete\t\tsave\n"
	"date\t\tdate\n"
	"time\t\ttime\n"
	"note\n\n"
	"Press any key to continue...");
				getchar();
				printf("\n\n");
				prevOpt = 0;
			}
		}
		else
			printf("Invalid option encountered.  -h for help.\n");
	}
}

/*
 * Generates a reply for the client based on the given request code.
 * 
 * Parameters:	The request code to generate a reply for.
 * Return:	A string indicating the state of the server after the last request
 */
char * generateReply(int requestCode){
	switch(requestCode){
		case 1:
			return "200 success";
		case 2:
			return "202 failed to add";
		case 3:
			return "300 deleted";
		case 4:
			return "302 not found";
		case 5:
			return "100 filesaved to file";
		case 6:
			return "102 unable to save";
		default:
			return "404 command not found";
	}
}