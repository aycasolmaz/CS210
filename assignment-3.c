/**
 * Assignment #3: Strings, structs, pointers, command-line arguments.
 *  Let's use our knowledge to write a simple flight management system!
 **/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>

// Limit constants
#define MAX_CITY_NAME_LEN 20
#define MAX_FLIGHTS_PER_CITY 5
#define MAX_DEFAULT_SCHEDULES 50

// Time definitions
#define TIME_MIN 0
#define TIME_MAX ((60 * 24)-1)
#define TIME_NULL -1


/******************************************************************************
 * Structure and Type definitions                                             *
 ******************************************************************************/
typedef int time_t;                        // integers used for time values
typedef char city_t[MAX_CITY_NAME_LEN+1];; // null terminate fixed length city
 
// Structure to hold all the information for a single flight
//   A city's schedule has an array of these
struct flight {
  time_t time;       // departure time of the flight
  int available;  // number of seats currently available on the flight
  int capacity;   // maximum seat capacity of the flight
};

// Structure for an individual flight schedule
// The main data structure of the program is an Array of these structures
// Each structure will be placed on one of two linked lists:
//                free or active
// Initially the active list will be empty and all the schedules
// will be on the free list.  Adding a schedule is finding the first
// free schedule on the free list, removing it from the free list,
// setting its destination city and putting it on the active list
struct flight_schedule {
  city_t destination;                          // destination city name
  struct flight flights[MAX_FLIGHTS_PER_CITY]; // array of flights to the city
  struct flight_schedule *next;                // link list next pointer
  struct flight_schedule *prev;                // link list prev pointer
};

/******************************************************************************
 * Global / External variables                                                *
 ******************************************************************************/
// This program uses two global linked lists of Schedules.  See comments
// of struct flight_schedule above for details
struct flight_schedule *flight_schedules_free = NULL;
struct flight_schedule *flight_schedules_active = NULL;


/******************************************************************************
 * Function Prototypes                                                        *
 ******************************************************************************/
// Misc utility io functions
int city_read(city_t city);           
bool time_get(time_t *time_ptr);      
bool flight_capacity_get(int *capacity_ptr);
void print_command_help(void);

// Core functions of the program
void flight_schedule_initialize(struct flight_schedule array[], int n);
struct flight_schedule * flight_schedule_find(city_t city);
struct flight_schedule * flight_schedule_allocate(void);
void flight_schedule_free(struct flight_schedule *fs);
void flight_schedule_add(city_t city);
void flight_schedule_listAll(void);
void flight_schedule_list(city_t city);
void flight_schedule_add_flight(city_t city);
void flight_schedule_remove_flight(city_t city);
void flight_schedule_schedule_seat(city_t city);
void flight_schedule_unschedule_seat(city_t city);
void flight_schedule_remove(city_t city);

void flight_schedule_sort_flights_by_time(struct flight_schedule *fs);
int  flight_compare_time(const void *a, const void *b);



int main(int argc, char *argv[]) 
{
  long n = MAX_DEFAULT_SCHEDULES;
  char command;
  city_t city;

  if (argc > 1) {
    // If the program was passed an argument then try and convert the first
    // argument in the a number that will override the default max number
    // of schedule we will support
    char *end;
    n = strtol(argv[1], &end, 10); // CPAMA p 787
    if (n==0) {
      printf("ERROR: Bad number of default max scedules specified.\n");
      exit(EXIT_FAILURE);
    }
  }

  // C99 lets us allocate an array of a fixed length as a local 
  // variable.  Since we are doing this in main and a call to main will be 
  // active for the entire time of the program's execution this
  // array will be alive for the entire time -- its memory and values
  // will be stable for the entire program execution.
  struct flight_schedule flight_schedules[n];
 
  // Initialize our global lists of free and active schedules using
  // the elements of the flight_schedules array
  flight_schedule_initialize(flight_schedules, n);

  // DEFENSIVE PROGRAMMING:  Write code that avoids bad things from happening.
  //  When possible, if we know that some particular thing should have happened
  //  we think of that as an assertion and write code to test them.
  // Use the assert function (CPAMA p749) to be sure the initilization has set
  // the free list to a non-null value and the the active list is a null value.
  assert(flight_schedules_free != NULL && flight_schedules_active == NULL);

  // Print the instruction in the beginning
  print_command_help();

  // Command processing loop
  while (scanf(" %c", &command) == 1) {
    switch (command) {
    case 'A': 
      //  Add an active flight schedule for a new city eg "A Toronto\n"
      city_read(city);
      flight_schedule_add(city);

      break;
    case 'L':
      // List all active flight schedules eg. "L\n"
      flight_schedule_listAll();
      break;
    case 'l': 
      // List the flights for a particular city eg. "l\n"
      city_read(city);
      flight_schedule_list(city);
      break;
    case 'a':
      // Adds a flight for a particular city "a Toronto\n
      //                                      360 100\n"
      city_read(city);
      flight_schedule_add_flight(city);
      break;
    case 'r':
      // Remove a flight for a particular city "r Toronto\n
      //                                        360\n"
      city_read(city);
      flight_schedule_remove_flight(city);
	break;
    case 's':
      // schedule a seat on a flight for a particular city "s Toronto\n
      //                                                    300\n"
      city_read(city);
      flight_schedule_schedule_seat(city);
      break;
    case 'u':
      // unschedule a seat on a flight for a particular city "u Toronto\n
      //                                                      360\n"
        city_read(city);
        flight_schedule_unschedule_seat(city);
        break;
    case 'R':
      // remove the schedule for a particular city "R Toronto\n"
      city_read(city);
      flight_schedule_remove(city);  
      break;
    case 'h':
        print_command_help();
        break;
    case 'q':
      goto done;
    default:
      printf("Bad command. Use h to see help.\n");
    }
  }
 done:
  return EXIT_SUCCESS;
}

/**********************************************************************
 * city_read: Takes in and processes a given city following a command *
 *********************************************************************/
int city_read(city_t city) {
  int ch, i=0;

  // skip leading non letter characters
  while (true) {
    ch = getchar();
    if ((ch >= 'A' && ch <= 'Z') || (ch >='a' && ch <='z')) {
      city[i++] = ch;
      break;
    }
  }

  while ((ch = getchar()) != '\n') {
    if (i < MAX_CITY_NAME_LEN) {
      city[i++] = ch;
    }
  }
  city[i] = '\0';
  return i;
}


/****************************************************************
 * Message functions so that your messages match what we expect *
 ****************************************************************/
void msg_city_bad(char *city) {
  printf("No schedule for %s\n", city);
}

void msg_city_exists(char *city) {
  printf("There is a schedule of %s already.\n", city);
}

void msg_schedule_no_free(void) {
  printf("Sorry no more free schedules.\n");
}

void msg_city_flights(char *city) {
  printf("The flights for %s are:", city);
}

void msg_flight_info(int time, int avail, int capacity) {
  printf(" (%d, %d, %d)", time, avail, capacity);
}

void msg_city_max_flights_reached(char *city) {
  printf("Sorry we cannot add more flights on this city.\n");
}

void msg_flight_bad_time(void) {
  printf("Sorry there's no flight scheduled on this time.\n");
}

void msg_flight_no_seats(void) {
    printf("Sorry there's no more seats available!\n");
}

void msg_flight_all_seats_empty(void) {
  printf("All the seats on this flights are empty!\n");
}

void msg_time_bad() {
  printf("Invalid time value\n");
}

void msg_capacity_bad() {
  printf("Invalid capacity value\n");
}

void print_command_help()
{
  printf("Here are the possible commands:\n"
	 "A <city name>     - Add an active empty flight schedule for\n"
	 "                    <city name>\n"
	 "L                 - List cities which have an active schedule\n"
	 "l <city name>     - List the flights for <city name>\n"
	 "a <city name>\n"
         "<time> <capacity> - Add a flight for <city name> @ <time> time\n"
	 "                    with <capacity> seats\n"  
	 "r <city name>\n"
         "<time>            - Remove a flight form <city name> whose time is\n"
	 "                    <time>\n"
	 "s <city name>\n"
	 "<time>            - Attempt to schedule seat on flight to \n"
	 "                    <city name> at <time> or next closest time on\n"
	 "                    which their is an available seat\n"
	 "u <city name>\n"
	 "<time>            - unschedule a seat from flight to <city name>\n"
	 "                    at <time>\n"
	 "R <city name>     - Remove schedule for <city name>\n"
	 "h                 - print this help message\n"
	 "q                 - quit\n"
);
}


/****************************************************************
 * Resets a flight schedule                                     *
 ****************************************************************/
void flight_schedule_reset(struct flight_schedule *fs) {
    fs->destination[0] = 0;
    for (int i=0; i<MAX_FLIGHTS_PER_CITY; i++) {
      fs->flights[i].time = TIME_NULL;
      fs->flights[i].available = 0;
      fs->flights[i].capacity = 0;
    }
    fs->next = NULL;
    fs->prev = NULL;
}

/******************************************************************
* Initializes the flight_schedule array that will hold any flight *
* schedules created by the user. This is called in main for you.  *
 *****************************************************************/

void flight_schedule_initialize(struct flight_schedule array[], int n)
{
  flight_schedules_active = NULL;
  flight_schedules_free = NULL;

  // takes care of empty array case
  if (n==0) return;

  // Loop through the Array connecting them
  // as a linear doubly linked list
  for (int i = 0; i< n-1; i++) {

    flight_schedule_reset(&array[i]);
    array[i].next = &array[i+1];
    array[i+1].prev = &array[i];

  }

  // Takes care of last node.  
  flight_schedule_reset(&array[n-1]); // reset clears all fields
  array[n-1].next = NULL;
  array[n-1].prev = &array[n-2];
  flight_schedules_free = &array[0];

}

/***********************************************************
 * time_get: read a time from the user
   Time in this program is a minute number 0-((24*60)-1)=1439
   -1 is used to indicate the NULL empty time 
   This function should read in a time value and check its 
   validity.  If it is not valid eg. not -1 or not 0-1439
   It should print "Invalid Time" and return false.
   othewise it should return the value in the integer pointed
   to by time_ptr.
 ***********************************************************/
bool time_get(int *time_ptr) {

  if (scanf("%d", time_ptr)==1) {

    return (TIME_NULL == *time_ptr || 
	    (*time_ptr >= TIME_MIN && *time_ptr <= TIME_MAX));

  } 

  msg_time_bad();
  return false;
  
}

/***********************************************************
 * flight_capacity_get: read the capacity of a flight from the user
   This function should read in a capacity value and check its 
   validity.  If it is not greater than 0, it should print 
   "Invalid capacity value" and return false. Othewise it should 
   return the value in the integer pointed to by cap_ptr.
 ***********************************************************/
bool flight_capacity_get(int *cap_ptr) {
  if (scanf("%d", cap_ptr)==1) {
    return *cap_ptr > 0;
  }
  msg_capacity_bad();
  return false;
}

void flight_schedule_sort_flights_by_time(struct flight_schedule *fs) 
{
  qsort(fs->flights, MAX_FLIGHTS_PER_CITY, sizeof(struct flight),
	flight_compare_time);
}

int flight_compare_time(const void *a, const void *b) 
{
  const struct flight *af = a;
  const struct flight *bf = b;
  
  return (af->time - bf->time);
}


struct flight_schedule * flight_schedule_allocate(void) {

  struct flight_schedule *fs = flight_schedules_free; // pointer to be used while dealing with the flight_schedule list. Pointer points to the address of the flight_schedule_free list.
  
  if (fs == NULL) { // test case: if free_schedules_free is NULL

    return NULL;  // return NULL

  }
  
  struct flight_schedule *fst = flight_schedules_free->next; // a temeporary pointer to hold the place for the next node of the flight_schedules_free.

  if (flight_schedules_active == NULL) {  // if flight_schedules_active is equal to NULL

    fs->next = NULL; // assign free_schedules_free's next node to NULL.
    flight_schedules_active = fs; // assigned the flight_schedule_active to the pointer fs.

    }

  else  { // if flight_schedules_active is not NULL.

    fs->next = flight_schedules_active; // then the next node of the fs pointer is going to point to the fligh_schedules_active
    flight_schedules_active->prev = fs; // the previous node of the active list is also going to point to the fs pointer.
    flight_schedules_active = fs; // assigned the address of the node of the flight_schedules_active list to the the pointer.

    }

    flight_schedules_free = fst;

  if (flight_schedules_free != NULL) {  // if flight_schedules_free does not equal NULL

    flight_schedules_free->prev = NULL;  // the previous node of flight_schedules_free is going to be NULL.

    }

  return flight_schedules_active;  // return the flight_schedules_active

}


void flight_schedule_add(city_t city) {

  struct flight_schedule *fs = flight_schedule_find(city); // assigns a pointer (fs) to the flight_schedule and finds the city inside the flight_schedule

  if (fs != NULL) { // if the pointer fs finds a city

    msg_city_exists(city); // Already exists; error message that prints "There is a schedule of %s already.""
    return;

  }  
  
  struct flight_schedule *p = flight_schedule_allocate(); // checks if there are free seats available for the given city
  if (p != NULL) {
    strcpy(p->destination, city);  // copy the into the destination.
  }
  
  else {

    msg_schedule_no_free(); // print "Sorry no more free schedules.\n"
    return;

  }

}


void flight_schedule_free(struct flight_schedule *fs) {
  
  if (fs->prev == NULL) {  // if the previous node of fs is equal to NULL

    if (fs->next == NULL) {  // if the next node of fs is equal to NULL

      flight_schedules_active = NULL;  // then flight_schedules_active is equal to NULL

    }

    else {

      flight_schedules_active = fs->next;  // else: if next node of fs is not NULL then flight_schedules_active is equal to the next node of fs.

    }

    
  }

  else {  //else: if the previous node of fs is not equal to NULL.

    if (fs->next == NULL) {  // if next node of fs is equal to NULL

      fs->prev->next = NULL;  // then the previous node of the next node of fs is equal to NULL

    }

    else {  // if the next node of fs is not equal to NULL.

      fs->prev->next = fs->next;  // then the previous node of the next node of fs jumps to the next node of fs
      fs->next->prev = fs->prev;  // and the next node of the previous node of fs is equal to the previous node of fs.

    }
  }


  flight_schedule_reset(fs); // reset the node

  if (flight_schedules_free == NULL) {  // if flight_schedules_free is equal to NULL

    flight_schedules_free = fs;  // then flight_schedules_free is equal to the fs node.

  }

  else {  // else: if flight_schedules_free is not equal to NULL.
    
    flight_schedules_free->prev = fs; // the previous node of flight_schedules_free is equal to fs
    fs->next = flight_schedules_free;  // next node of fs is equal to flight_schedules_free
    flight_schedules_free = fs;  // flight_schedules_free is equal to fs

  }
  
}


void flight_schedule_remove(city_t city) {
  struct flight_schedule *fs = flight_schedule_find(city);  // intializes a pointer to found schedule of given city's flight

  if (fs == NULL) { // if fs is equal to NULL

    msg_city_bad(city);  // print an error message
    return;
  }

  flight_schedule_free(fs);  // used flight_schedule_free to remove a flight_schedule

}


struct flight_schedule * flight_schedule_find(city_t city) {

  struct flight_schedule *fs = flight_schedules_active; // assigned a pointer to thee flight_schedule; the pointer equals to flight_schedule_active

  if (fs == NULL) {  // if the pointer is NULL

    return NULL;     // then return NULL

  }

  while (fs != NULL) {  // while the pointer is not NULL

    if (strcmp(city, fs->destination) == 0){  // compare two strings: city and destination of the pointer and if they're not correct

      return fs;   // return fs

    }

    fs = fs->next;  // check the next node of fs
    
  }

  return NULL; // return NULL if nothing is true

}


void flight_schedule_listAll(void) {

  struct flight_schedule *temp;  // pointer to the flight_schedule
  temp = flight_schedules_active;    // points to the address of the first node of flight_schedule_active

  while(temp != NULL) {       // while the pointer is not NULL meaning at the end of the list.

    printf("%s\n", temp->destination);    // print the destinations of the flight_schedule
    temp = temp->next;       // point to the next node of the flight_schedule list.

  }

}


void flight_schedule_list(city_t city) {
  struct flight_schedule *fs = flight_schedule_find(city);  // initialized a pointer; assigned the pointer to find the given city

  if (fs != NULL) {   // if fs is found meaning it's not NULL

    msg_city_flights(city);    // print "The flights for %s are:"
    for (int i = 0; i < MAX_FLIGHTS_PER_CITY; i++) {  // for loop iterates over all of the flights

      if (fs->flights[i].time != TIME_NULL) {  // checks to see if it's a valid time. 

        time_t time = fs->flights[i].time;   // if it is assigns the time of the flight
        int availability = fs->flights[i].available;   // assigns the availability of the flight
        int capacity = fs->flights[i].capacity;  // assigns the capacity of the flight
        msg_flight_info(time, availability, capacity);   // prints all the information
        
      }
    }
    printf("\n");
  }

  else {  // if it is NULL

    msg_city_bad(city);  // prints "No schedule for %s\n"
    return;

  }
}


void flight_schedule_add_flight(city_t city){

  struct flight_schedule *point;     // a pointer (named point) has been initialized.
  point = flight_schedule_find(city); // pointer points to the address of the flight_schedule struct.
  int i = 0; // intialized variable to find the flights of a given city.
  time_t time;  // initalized a variable for time
  int capacity; // initialized a variable for capacity

  time_get(&time); //finds the time of a given city
  flight_capacity_get(&capacity); //finds the capacity of a given city

  if (capacity <= 0) {  // if capacity is less than or equal to zero

    return;  // then finish the function

  }

  else if (point->flights[i].time != -1) {  // if time is invalid = -1.

    msg_city_max_flights_reached(city);   // then the number of max flights has been reached; print an error message
    return;

  }

  else if (point == NULL) {  // if the pointer is NULL

    msg_city_bad(city); // then the city couldn't be found; print an error message
    return;

  }

  else {

    point->flights[i].time = time;     // add a time for the given city
    point->flights[i].capacity = capacity;  // add a capacity for given city
    point->flights[i].available = capacity; // arrange the availability of a given city's flight
    flight_schedule_sort_flights_by_time(point); // sorts the flights by given times of a certain city.

  }
}


void flight_schedule_remove_flight(city_t city) {
  struct flight_schedule *point = flight_schedule_find(city);    // a pointer (named point) has been initialized.
  
  if (point == NULL) { // if the given pointer (finds the given city) is equal to NULL

    msg_city_bad(city); // print an error message
    return;

  }

  time_t j = 0;

  if (1 != time_get(&j)) { // if time_get equals anything other than a valid number then it returns.

    return;

  }

  for (int i = 0; i < MAX_FLIGHTS_PER_CITY; i++) {  // for loop from 0 to MAX_FLIGHTS_PER_CITY

    if (point->flights[i].time == j) {  // if the time for the given city is equal to the time.

      point->flights[i].time = TIME_NULL;     // add a time for the given city
      point->flights[i].capacity = 0;  // add a capacity for given city
      point->flights[i].available = 0; // arrange the availability of a given city's flight
      flight_schedule_sort_flights_by_time(point); // sorts the flights by given times of a certain city.
      return;

    }
  }

  msg_flight_bad_time();  // print an error message saying: "Sorry there's no flight scheduled on this time.\n"

}


void flight_schedule_schedule_seat(city_t city) {

  struct flight_schedule *seat = flight_schedule_find(city); // initalized a pointer called seat

  if (seat == NULL) { // if there isn't any city found

    msg_city_bad(city); // then print "No schedule for %s\n"
    return;

  }

  int j = 0; // intialized a int variable j

  if (1 != time_get(&j)) { // if time_get equals anything other than a valid number then it returns.

    return;

  }
  
  for (int i = 0; i < MAX_FLIGHTS_PER_CITY; i++) {  // for loop from 0 to MAX_FLIGHTS_PER_CITY 

    int timeClose = (j - seat->flights[i].time);  // initialized timeClose variable to understand which time is the closest.

    if (timeClose <= 0) {  // if the variable was less than or equal to zero

      if (seat->flights[i].available > 0) { // and if there are available seats in the flight

        seat->flights[i].available--;  // decrement available seats at the flight
        return;

      }
    }
  }

  msg_flight_no_seats(); // there weren't seats available, print "Sorry there's no more seats available!\n"
  return;

}

void flight_schedule_unschedule_seat(city_t city) {
  struct flight_schedule *fs = flight_schedule_find(city);

  if (fs == NULL) { // if there isn't any city found

    msg_city_bad(city); // then print "No schedule for %s\n"
    return;

  }

  else {

    int j = 0; // intialized a int variable j

    if (1 != time_get(&j)) {  // if time_get equals anything other than a valid number then it returns. 

      return;
      
    }
  
    for (int i = 0; i < MAX_FLIGHTS_PER_CITY; i++) {  // for loop from 0 to MAX_FLIGHTS_PER_CITY 

      if (j == fs->flights[i].time) {  // if the time was equal to the flight 

        if (fs->flights[i].capacity > fs->flights[i].available) { // and if there are available seats in the flight

          fs->flights[i].available++;  // increment available seats at the flight
          return;

        }
        else {

        msg_flight_all_seats_empty(); // if there are seats available, print message
        return;

        } 
      }
    }

    msg_flight_bad_time(); // print an error message saying: "Sorry there's no flight scheduled on this time.\n"

  }
}


