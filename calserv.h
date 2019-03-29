/* -------------------------------------------------------------------------
 * Student Names: Jonathan Stregger
 * Assignment 3: calserv header file
 * Lab Section: X01L, Winter 2019
 * Lab Instructor’s Name: Ron Meleshko
 * CMPT 361, Class Section: AS01, Winter 2019
 * Instructor's Name: Ron Meleshko
 * Purpose: 
 *    Manages a calender server of appointments for a client.
 *    Clients may add, modify, or delete appointments. Success or failure
 *    of client actions on serverdata will be reported to the client upon
 *    completion or failure.
 *    Format of data received from client is yy-mm-dd;hh:mm-hh-mm;note.
 *    Sockets are managed to allow the client to connect over port 12000.
 *    Client data is stored in a binary file named calfile of calendar data
 *    Upon exit, the previous file will be renamed and a new file of
 *    calendar data will be written before deleting the old data.
 * ---------------------------------------------------------------------- */

#ifndef CALSERVL_H
#define CALSERVL_H

#include <stdio.h>

// Appointments will be stored in appointment.
typedef struct caldata{
  char *date;      //Date of format yy-mm-dd
  char *time;      //Time of format hh:mm-hh-mm
  char *note;      //A note about the appointment
} appointment;

// All apppointments are stored in a calendar which tracks the 
// nubmer of appointments
typedef struct calList{
  appointment **appList;  //A list of appointments
  int added;              //tracks how many appointments are in the calendar
  int max;                //room available for appointments
  char * filename;        //the filename associated with the calendar
} calendar;

/*
 * Opens a socket for connection.
 * 
 * Parameters:  The port to open a socket on
 *         The active calendar for the connection
 */
void openConnection(char*, calendar*);

/*
 * Loads calendar data from calfile.  If the file cannot be found one is 
 * created.
 *
 * Parameters:  The filename to load from.
 * Return: A calendar with appointments from calfile.
 */
calendar* loadCalendarData(char *);

/*
 * Parses a command for the server to execute.
 *
 * Parameters:  The string to pars the command from.
 *        The calendar to execute the command on.
 * Return:  0 : Bad parameter(s) encountered
 *       1 : Bad command encountered
 *       2 : Add success
 *       3 : Add failure, illegal data given
 *       4 : Delete success
 *       5 : Delete failure, appointment not found
 *       6 : Save completed
 *       7 : Unable to save
 */
int parseCommand(char *, calendar *);

/*
 * Makes an appointment from a string
 *
 * Parameters:  A string containing appointment data to parse.
 * Return:  If the string was a valid appointment, then an appointment
 *      will be returned.  Otherwise, a NULL pointer will be returned.
 */
appointment * makeAppointment(char *);

/*
 * Adds an appointment to the calendar.
 *
 * Parameters:  The appointment to add to the calendar.
 *        The calendar to add the appointment to.
 * Returns 1 if added to calednar, 2 if improper formatting found.
 */
int addAppointment(appointment *, calendar *);

/*
 * Removes the appointment from the calendar.
 *
 * Parameters:  The appointment to remove.
 *        The calendar to remove the appointment from.
 * Returns 4 if appointment could be found and removed, 5 if it was not.
 */
int removeAppointment(appointment *, calendar *);

/*
 * Checks to see if the appointments have the same time and date. Notes
 * are not compared.
 *
 * Parameters:  The two appointments to compare.
 * Return:  1 if the appointments are equal, 0 if they are not.
 */
int equalsApp(appointment *, appointment *);

/*
 * Gets the requested appointment(s) based on the request.
 * 
 * Paramenters: Request is the request from the user
 *              Cal is the calendar to search
 * Return:  The reply to send to the client along with the success code
 *          500 appointment retrieved.  If no appointment is found, then
 *          a the failure code 502 appointment not found is sent.
 */
char * getRequest(char *request, calendar *cal);

/*
 * Checks to see if the appointments have the same time and date. Notes
 * are not compared.
 *
 * Parameters:  The two appointments to compare.
 * Return:  1 if the appointments are equal, 0 if they are not.
 */
int testAppointment(char *);

/*
 * Checks to see if the appointments have the same time and date. Notes 
 * are not compared.
 *
 * Parameters:  The two appointments to compare.
 * Return:  1 if the appointments are equal, 0 if they are not.
 */
int isNum(char);

/*
 * Frees all allocated memory associated with the given calendar.
 *
 * Parameters:  The calendar to free the allocated memory for.
 */
void freeCalendar(calendar *);

/*
 * Saves the calendar data to calfile.
 *
 * Paramenters:  The calendar of all appointments
 * Return: 6 if successful. 7 if unsuccessful.
 */
int saveCalendarData(calendar *);

/*
 * Converts an appointment to a string: "yy-mm-dd;hh:mm-hh-mm;note\n"
 *
 * Parameters:  The appointment to convert to string.
 * Return:  A string representation of the given appointment.
 */
char * appointmentToString(appointment *);

/*
 * Handles command line arguements.
 *
 * Parameters:  Number of command line arguemnts
 *        Command line arguements
 *        Port number
 *        Filename
 */
void getOptions(int argc, char **argv, int *, char **);

/*
 * Generates a reply for the client based on the given request code.
 * 
 * Parameters:  The request code to generate a reply for.
 * Return:  A string indicating server state after the last request
 */
char * generateReply(int);

/*
 * Converts a client message to server format: '\n' between fields become ';'
 */
char * convertToServer(char *);

/*
 * Converts a server message to client format: ';' to '\n'
 */
char * convertToClient(char *app);

#endif