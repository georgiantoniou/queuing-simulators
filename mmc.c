/*******************************************************************************
*                           M/M/c Queue Simulator
********************************************************************************
* Notes: There are some problems with the "Server Utilization" metric, resulting
* in negative values when simulation is run for a sufficiently long time
*------------------------------------------------------------------------------*
* Build Command:
* gcc -o mmc mmc.c -lm
*------------------------------------------------------------------------------*
* Execute command:
* ./mm10
*------------------------------------------------------------------------------*
* Author: Lucas German Wals Ochoa
*******************************************************************************/

/*******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>              // Needed for printf()
#include <stdlib.h>             // Needed for exit() and rand()
#include <unistd.h>             // Needed for getopts()
#include <stdbool.h>            // Needed for bool type
#include "utils.h"              // Needed for expntl()

/*******************************************************************************
* Defined constants and variables
* NOTE: All TIME constants are defined in seconds!
*******************************************************************************/
#define SIM_TIME   1.0e9        // Simulation time
#define ARR_TIME   90.00        // Mean time between arrivals
#define SERV_TIME  60.00        // Mean service time
#define NUM_SERVERS  2         // Number of servers in the system

typedef struct Node {           // Added by Georgia
    double value;                
    struct Node *next;
} Node;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void show_usage(char *name);
int min_departure(double arr[], int capacity); // Find the index of the minimum departure time
int peak_server(int c);                     // added by georgia choose a server randomly [0,c]
bool all_active(double custDepartures[], int c); // added by georgia check if all servers active
Node* addEntry(Node *head, double value);    // added by georgia add entry to a linked list
void freeList(Node *head);                          // added by georgia de - allocate list
void printList(Node *head);                         // added by georgia print list content
double removeFirst(Node **head);                   // added by georgia remove first element from list
void printInterarrival(Node *head);  // added by georgia print interarrival time
void printCoreIdleDistr(Node *head, double idleTime);   // added by georgia print core idle distribution
bool all_idle(double custDepartures[], int c);         // added by georgia  return true if all servers idle
void printPackageIdleDistr(Node *head);                // added by georgia print pkg idle distribution

/*******************************************************************************
* Main Function
*******************************************************************************/
int main(int argc, char **argv)
{
    int opt;    // Hold the options passed as argument
    double endTime = SIM_TIME;        // Total time to do Simulation
    double arrTime = ARR_TIME;        // Mean time between arrivals
    double departTime = SERV_TIME;    // Mean service time
    int c = NUM_SERVERS;              // Number of servers in the system

    double time = 0.0;                  // Current Simulation time
    double nextArrival = 0.0;           // Time for next arrival
    double nextDeparture = SIM_TIME;    // Time for next departure
    
    int nextDepartIndex;                // Index of next departure time in array
    int arrayIndex = 0;                 // Auxiliar variable 
    unsigned int n = 0;           // Actual number of customers in the system

    unsigned int departures = 0;  // Total number of customers served
    double busyTime = 0.0;        // Total busy time
    double s = 0.0;               // Area of number of customers in system
    double lastEventTime = time;  // Variable for "last event time"
    double lastBusyTime;          // Variable for "last start of busy time"
    double x;     // Throughput rate
    double u;     // Utilization of system
    double l;     // Average number of customers in system
    double w;     // Average Sojourn time
    double busyTimeAll = 0.0;     // Added by Georgia sum all busy periods (not only full busy)
    double idleTimeAll = 0.0;     // Added by Georgia sum all idle periods (not only full idle)
    Node *coreidlePeriods = NULL;     // Added by Georgia linked-list to save idle period duration
    Node *packageIdlePeriods = NULL;  // Added by Georgia linked-list to save idle period duration
    Node *arrivalPeriods = NULL;     // Added by Georgia linked-list to save interarrival times duration
    Node *servicePeriods = NULL;     // Added by Georgia linked-list to save service time duration
    Node *queuedArrivals = NULL; // added by georgia to check remember arrivals that are queued
    int lastAssignment = c-1;     // Added by Georgia remember last assignment of request to server for round robin
    
    if (argc > 1)
    {     
        while ( (opt = getopt(argc, argv, "a:d:s:c:")) != -1 )
        {
            switch (opt) {
                case 'a':
                    arrTime = atof(optarg);
                    break;
                case 'd':
                    departTime = atof(optarg);
                    break;
                case 's':
                    endTime = atof(optarg);
                    break;
                case 'c':
                    c = atoi(optarg);
                    break;
                default:    // '?' unknown option
                    show_usage( argv[0] );
            }
        }
    }

    double custDepartures[c]; // Departure times of serving customer
    double custIdle[c];    // added by georgia idle per core
    double custIdleP=0;    // added by georgia idle per Package
    double arrivals[c];    // added by georgia number of arrivals per core
    double custarrivals[c]; // added by georgia to measure the interarrival time of each core sees
    Node* arrivalsPerCore[c]; // added by georgia interarrival time per core;
    Node* jobsQueue[c];

    for (int i=0; i < c; i++)
    {    
        custDepartures[i] = SIM_TIME;   // Fill the array with maximum time
        custIdle[i] = 0;
        arrivals[i] = 0;
        custarrivals[i] = 0;
        arrivalsPerCore[i] = NULL;
        jobsQueue[i] = NULL;
    }

    // Simulation loop
    while (time < endTime)
    {
        // Arrival occurred
        if (nextArrival < nextDeparture)
        {
            time = nextArrival;
            s = s + n * (time - lastEventTime);  // Update area under "s" curve
            n++;    // Customers in system increase
            lastEventTime = time;   // "last event time" for next event
            nextArrival = time + expntl(arrTime);
            arrivalPeriods = addEntry(arrivalPeriods, nextArrival - time);

            arrayIndex=peak_server(c);
            if (custDepartures[arrayIndex] == SIM_TIME)
            {
                custDepartures[arrayIndex] = time + expntl(departTime);
                servicePeriods = addEntry(servicePeriods, custDepartures[arrayIndex] - time);
                busyTimeAll = busyTimeAll + (custDepartures[arrayIndex] - time); //added by Georgia

                if (custIdle[arrayIndex] != -1)
                {
                    idleTimeAll = idleTimeAll + (time - custIdle[arrayIndex]);
                    coreidlePeriods = addEntry(coreidlePeriods, time - custIdle[arrayIndex]);
                    custIdle[arrayIndex] = -1;
                }
            }
            else
            {
                jobsQueue[arrayIndex] = addEntry(jobsQueue[arrayIndex],time);
            }
                               
            arrivalsPerCore[arrayIndex] = addEntry(arrivalsPerCore[arrayIndex],time); // to measure interarrival time per core               

            if (n == 1)
            {
                nextDepartIndex = arrayIndex;
                nextDeparture = custDepartures[nextDepartIndex];    
            }
            else // added by georgia whever a new arrival happens we need to update the Departure index to the minimum
            {
                nextDepartIndex = min_departure(custDepartures, c);
                nextDeparture = custDepartures[nextDepartIndex];
            }
                
            if (n >= c && all_active(custDepartures,c)) 
            {
                lastBusyTime = time;    // Set "last start of busy time"
            }

            if (!all_idle(custDepartures,c))
            {
                if (custIdleP != -1)
                {
                    packageIdlePeriods = addEntry(packageIdlePeriods, time - custIdleP); 
                    custIdleP = -1;
                }
            }
        }
        // Departure occurred
        else
        {
            time = nextDeparture;
            s = s + n * (time - lastEventTime); // Update area under "s" curve
            n--;    // Customers in system decrease
            lastEventTime = time;   // "last event time" for next event
            departures++;           // Increment number of completions
            custDepartures[nextDepartIndex] = SIM_TIME; // Set server as empty
            arrivals[nextDepartIndex]++; 
            if (n > 0)
            {                

                double nextjob = removeFirst(&jobsQueue[nextDepartIndex]);
                if (nextjob == -1)
                {
                    custDepartures[nextDepartIndex] = SIM_TIME;
                    custIdle[nextDepartIndex] = time;
                    
                }
                else 
                {
                    custDepartures[nextDepartIndex] = time + expntl(departTime);
                    arrivalsPerCore[nextDepartIndex] = addEntry(arrivalsPerCore[nextDepartIndex], nextjob);
                    servicePeriods = addEntry(servicePeriods, custDepartures[nextDepartIndex] - time);
                    busyTimeAll = busyTimeAll + (custDepartures[nextDepartIndex] - time);
                }
                    
                
                // Look for the next departure time
                nextDepartIndex = min_departure(custDepartures, c);
                nextDeparture = custDepartures[nextDepartIndex]; 
                
                if (!all_active(custDepartures,c)) 
                { // Update busy time when at least one server idle
                    busyTime = busyTime + time - lastBusyTime;   
                }
            }
            else
            {
                nextDeparture = SIM_TIME;
                custIdle[nextDepartIndex] = time;
                if (all_idle(custDepartures,c))
                {
                    custIdleP = time;
                    
                }
            }
            
        } // end of departure event
    }

    // Compute outputs
    x = departures / time;  // Compute throughput rate
    u = busyTime / time;    // Compute server utilization
    l = s / time;             // Avg number of customers in the system
    w = l / x;              // Avg Sojourn time

    // Output results
    printf("<-------------------------------------------------------------> \n");
    printf("<           *** Results for M/M/%d simulation ***             > \n", c);
    printf("<-------------------------------------------------------------> \n");
    printf("-  INPUTS: \n");
    printf("-    Total simulation time        = %.9f sec \n", endTime);
    printf("-    Mean time between arrivals   = %.9f sec \n", arrTime);
    printf("-    Mean service time            = %.9f sec \n", departTime);
    printf("-    # of Servers in system       = %d servers \n", c);
    printf("<-------------------------------------------------------------> \n");
    printf("-  OUTPUTS: \n");
    printf("-    # of Customers served        = %u cust \n", departures);
    printf("-    Throughput rate              = %f cust/sec \n", x);
    printf("-    Server utilization           = %f %% (time system is full busy)\n", 100.0 * u);
    printf("-    Avg # of cust. in system     = %f cust \n", l);
    printf("-    Mean Sojourn time            = %f sec \n", w);
    // Added by Georgia
    printf("-    Busy Time                    = %f sec (activity time of each core added together)\n", busyTimeAll);
    printf("-    Idle Time                    = %f sec (idle time of each core added together)\n", idleTimeAll);
    printf("-    Average utilization          = %f %% \n", 100.0 * (busyTimeAll/(idleTimeAll+busyTimeAll)));
    printf("-    Arrivals per core: \n");
    for (int i=0; i< c ; i++)
        printf("-    Core %d                    = %f \n", i, arrivals[i]);
    printf("-    Core Idle Time Distribution: \n");
    printCoreIdleDistr(coreidlePeriods, idleTimeAll);
    printf("-    Package Idle Time Distribution: \n");
    printPackageIdleDistr(packageIdlePeriods);

    printf("<-------------------------------------------------------------> \n");
}

/*******************************************************************************
*       min_departure(double arr[], int capacity)
********************************************************************************
* Function that return the index of the minimum departure time
* - Input: arr (array of departures)
* - Input: capacity (size of the array)
*******************************************************************************/
int min_departure(double arr[], int capacity)
{
    int index = 0;

    for (int i=1; i < capacity; i++)
    {
        if (arr[i] < arr[index])
            index = i;
    }
    // printf(" - INDEX = %d \n", index);
    return index;
}

/*******************************************************************************
*       show_usage(char *name)
********************************************************************************
* Function that return a message of how to use this program
* - Input: name (the name of the executable)
*******************************************************************************/
static void show_usage(char *name)
{
    printf("\nUsage: \n");
    printf("%s [option] value \n", name);
    printf("\n");
    printf("Options: \n");
    printf("\t-a\tMean time between arrivals (in seconds) \n");
    printf("\t-d\tMean service time (in seconds) \n");
    printf("\t-s\tTotal simulation time (in seconds) \n");
    printf("\t-c\tNumber of servers in the system\n");
    exit(EXIT_SUCCESS);
}

/*********************************************************************************
*       Node* addEntry(Node *head, double value)
**********************************************************************************
* Function that adds a new entry to the linked-list with the idle period durations
* (added by Georgia)
* - Input: *head (head of linked-list)
*           value (idle time duration i want to add)
* - Output: Node* (pointer to the new Node added to the linked-list)
**********************************************************************************/

Node* addEntry(Node *head, double value) 
{
    Node *newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed!\n");
        return head;
    }
    newNode->value = value;
    newNode->next = head;
    return newNode;
}

/*******************************************************************************
*       freeList(Node *head)
********************************************************************************
* Function that deletes the allocated memory of the  linked-list of idle 
* durations (added by Georgia)
* - Input: *head (head of linked-list)
*******************************************************************************/
void freeList(Node *head) 
{
    Node *temp;
    while (head) {
        temp = head;
        head = head->next;
        free(temp);
    }
}

/*******************************************************************************
*       printList(Node *head)
********************************************************************************
* Function to print the list of idle durations (added by Georgia)
* - Input: *head (head of linked-list)
*******************************************************************************/
void printList(Node *head) 
{
    Node *temp = head;
    while (temp) {
        printf("Value: %.9f\n", temp->value);
        temp = temp->next;
    }
}

/*******************************************************************************
*      void printArrivalsPerCore(Node *head, int c) 
********************************************************************************
* Function print interarrival time per core (added by Georgia)
* - Input: *head (head of linked-list)
* -         c (M/M/c server/core capacity)
*******************************************************************************/
void printInterarrival(Node *head) 
{
    Node *temp = head;
    while (temp->next != NULL)
    {
        double value = temp->value - temp->next->value;
        temp = temp->next;
        printf("-  Value = %f us \n", value*1000000);

    }
}

double removeFirst(Node **head)  // Pass head by reference to modify it
{
    if (*head == NULL) {
        // printf("Error: Trying to remove from an empty list!\n");
        return -1;  // Indicate an error
    }

    Node *temp = *head;
    Node *prev = NULL;

    // Traverse to the last node
    while (temp->next != NULL) {
        prev = temp;
        temp = temp->next;
    }

    double value = temp->value;  // Store the value of the last node

    // If there's only one node, set head to NULL
    if (prev == NULL) {
        *head = NULL;
    } else {
        prev->next = NULL;  // Remove the last node
    }

    free(temp);  // Free the removed node
    return value;  
}

/*******************************************************************************
*       printCoreIdleDistr(Node *head, double idleTime)
********************************************************************************
* Print Idle Time Distribution (added by Georgia)
* - Input: *head (head of linked-list)
*           idleTime (overall idle time of the simulation)
*******************************************************************************/
void printCoreIdleDistr(Node *head, double idleTime)
{
    double distribution[4];
    distribution[0] = 0;
    distribution[1] = 0;
    distribution[2] = 0;
    distribution[3] = 0;

    struct Node *node=NULL, *temp = NULL;
    node = head;
    temp = head;
    while (temp != NULL) 
    {
        if (temp->value < 0.000002)
            distribution[0] = distribution[0]  + temp->value;
        if  (temp->value < 0.000020) 
            distribution[1] = distribution[1]  + temp->value;
        if (temp->value < 0.000600)
            distribution[2] = distribution[2]  + temp->value;
        distribution[3] = distribution[3]  + temp->value;
        temp = temp->next;
    }
      
    printf("-   %% idle < 2us        = %.6f \n", distribution[0]/idleTime);
    printf("-   %% idle < 20us       = %.6f \n", distribution[1]/idleTime);
    printf("-   %% idle < 600us      = %.6f \n", distribution[2]/idleTime);
    printf("-   %% idle > 600us      = %.6f \n", distribution[3]/idleTime);

}

void printPackageIdleDistr(Node *head)
{
    double distribution[4];
    distribution[0] = 0;
    distribution[1] = 0;
    distribution[2] = 0;
    distribution[3] = 0;

    struct Node *node=NULL, *temp = NULL;
    double allIdle=0;
    node = head;
    temp = head;
    while (temp != NULL) 
    {
        if (temp->value < 0.000010)
            distribution[0] = distribution[0]  + temp->value;
        if  (temp->value < 0.000100) 
            distribution[1] = distribution[1]  + temp->value;
        if (temp->value < 0.001000)
            distribution[2] = distribution[2]  + temp->value;
        distribution[3] = distribution[3]  + temp->value;
        allIdle = allIdle + temp->value;
        temp = temp->next;
    }
    
    printf("-   %% idle < 10us        = %.6f \n", distribution[0]/allIdle);
    printf("-   %% idle < 100us       = %.6f \n", distribution[1]/allIdle);
    printf("-   %% idle < 1000us      = %.6f \n", distribution[2]/allIdle);
    printf("-   %% idle > 1000us      = %.6f \n", distribution[3]/allIdle);

}

int peak_server(int c)
{
    
    int rd_num = rand() % (c) + 0;
    return rd_num;

}

bool all_active(double custDepartures[], int c)
{
    for (int i = 0 ; i<c; i++)
    {
        if (custDepartures[i] == SIM_TIME)
            return false;

    }
    return true;
}

bool all_idle(double custDepartures[], int c)
{
    for (int i = 0 ; i<c; i++)
    {
        if (custDepartures[i] != SIM_TIME)
            return false;

    }
    return true;
}
