/* ---------------------------------------------------------------------------
 * Student Names: Jonathan Stregger
 * Assignment 3: calclient header file
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

#ifndef CALCLIENT_H
#define CALCLIENT_H

/*
 * Recieves a requests from the user for the server.  Checks for validity
 * before sending on to the calendar server.
 * 
 * Return:  A string that can be sent to the server
 */
char * generateRequest();

/*
 * Open a connection to the calendar server.
 */
void connectToCalServ();

#endif