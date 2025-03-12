/*******************************************************************************
*                           M/M/1 Queue Simulator
********************************************************************************
* Notes: UNDER CONSTRUCTION
*------------------------------------------------------------------------------*
* Build Command:
* gcc -o mm1 mm1.c -lm
*------------------------------------------------------------------------------*
* Execute command:
* ./mm1
*------------------------------------------------------------------------------*
* Author: Lucas German Wals Ochoa
*******************************************************************************/

/*******************************************************************************
* Includes
*******************************************************************************/
#include <stdio.h>              // Needed for printf()
#include <stdlib.h>             // Needed for exit() and rand()
#include <unistd.h>             // Needed for getopts()
#include "utils.h"              // Needed for expntl()

/*******************************************************************************
* Defined constants and variables
* NOTE: All TIME constants are defined in seconds!
*******************************************************************************/
#define SIM_TIME   1.0e9        // Simulation time
#define ARR_TIME   90.00        // Mean time between arrivals
#define SERV_TIME  60.00        // Mean service time

typedef struct Node {           // Added by Georgia
    int id;                     // This is for linked-list to store idle 
    double value;                // period durations
    struct Node *next;
} Node;

/*******************************************************************************
* Function Prototypes
*******************************************************************************/
static void show_usage(char *name);
Node* addEntry(Node *head, int id, double value);    // added by georgia
void freeList(Node *head);                          // added by georgia
void printIdleDistr(Node *head, double idleTime);   // added by georgia
void printList(Node *head);                         // added by georgia

/*******************************************************************************
* Main Function
*******************************************************************************/
int main(int argc, char **argv)
{
    int opt;    // Hold the options passed as argument
    double endTime = SIM_TIME;        // Total time to do Simulation
    double arrTime = ARR_TIME;        // Mean time between arrivals
    double departTime = SERV_TIME;    // Mean service time

    double time = 0.0;          // Current Simulation time
    double nextArrival = 0.0;         // Time for next arrival
    double nextDeparture = SIM_TIME;  // Time for next departure
    unsigned int n = 0;           // Actual number of customers in the system

    unsigned int departures = 0;  // Total number of customers served
    double busyTime = 0.0;        // Total busy time
    double idleTime = 0.0;        // Added by georgia current idle time of the system
    double idleTimeTotal = 0.0;   // Added by georgia total idle time 
    double s = 0.0;               // Area of number of customers in system
    double lastEventTime = time;  // Variable for "last event time"
    double lastBusyTime;          // Variable for "last start of busy time"
    double x;     // Throughput rate
    double u;     // Utilization of system
    double l;     // Average number of customers in system
    double w;     // Average Sojourn time
    Node *idlePeriods = NULL;     // Added by Georgia linked-list to save idle period duration
    Node *arrivalPeriods = NULL;     // Added by Georgia linked-list to save idle period duration
    Node *servicePeriods = NULL;     // Added by Georgia linked-list to save idle period duration
    unsigned int idles = 0; // Added by Georgia counter of idle periods
    unsigned int arrivals = 0; // Added by Georgia counter of arrival periods      

    if (argc > 1)
    {     
        while ( (opt = getopt(argc, argv, "a:d:s:")) != -1 )
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
                default:    // '?' unknown option
                    show_usage( argv[0] );
            }
        }
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
            arrivals++;
            arrivalPeriods = addEntry(arrivalPeriods, arrivals , nextArrival - time);
            if (n == 1) // System is full, only have 1 server
            {
                lastBusyTime = time;    // Set "last start of busy time"
                nextDeparture = time + expntl(departTime); 
                servicePeriods = addEntry(servicePeriods, departures , nextDeparture - time);
                if (idleTime != 0) 
                {
                    idles++;
                    idlePeriods = addEntry(idlePeriods, idles, time - idleTime);
                    idleTimeTotal = idleTimeTotal + (time - idleTime);
                    idleTime=0;
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
            if (n > 0)
            {
                nextDeparture = time + expntl(departTime);
                servicePeriods = addEntry(servicePeriods, departures , nextDeparture - time);
            }
            else
            {
                nextDeparture = SIM_TIME;
                // Update busy time sum when no customers
                busyTime = busyTime + time - lastBusyTime;
                idleTime = time;
            }
        }
    }

    // Compute outputs
    x = departures / time;  // Compute throughput rate
    u = busyTime / time;    // Compute server utilization
    l = s / time;           // Avg number of customers in the system
    w = l / x;              // Avg Sojourn time

    // Output results
    printf("<-------------------------------------------------------------> \n");
    printf("<            *** Results for M/M/1 simulation ***             > \n");
    printf("<-------------------------------------------------------------> \n");
    printf("-  INPUTS: \n");
    printf("-    Total simulation time        = %.9f sec \n", endTime);
    printf("-    Mean time between arrivals   = %.9f sec \n", arrTime);
    printf("-    Mean service time            = %.9f sec \n", departTime);
    printf("<-------------------------------------------------------------> \n");
    printf("-  OUTPUTS: \n");
    printf("-    Total busy time              = %.9f sec \n", busyTime);            // added by Georgia
    printf("-    Total idle time              = %.9f sec \n", idleTimeTotal);       // added by Georgia 
    printf("-    # of Customers served        = %u cust \n", departures);
    printf("-    Throughput rate              = %f cust/sec \n", x);
    printf("-    Server utilization           = %f %% \n", 100.0 * u);
    printf("-    Avg # of cust. in system     = %f cust \n", l);
    printf("-    Mean Sojourn time            = %f sec \n", w);
    printf("-    Arrival Time Periods: \n");
    printList(arrivalPeriods);
    printf("-    Service Time Periods: \n");
    printList(servicePeriods);
    printf("-    Idle Time Periods: \n");
    printList(idlePeriods);
    printIdleDistr(idlePeriods,time - busyTime);     // Added by Georgia Print Idle Distribution
    
    printf("<-------------------------------------------------------------> \n");

    // Free memory added by Georgia
    freeList(idlePeriods);
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
    exit(EXIT_SUCCESS);
}

/*********************************************************************************
*       Node* addEntry(Node *head, int id, double value)
**********************************************************************************
* Function that adds a new entry to the linked-list with the idle period durations
* (added by Georgia)
* - Input: *head (head of linked-list)
*           id (id of the item i want to add)
*           value (idle time duration i want to add)
* - Output: Node* (pointer to the new Node added to the linked-list)
**********************************************************************************/

Node* addEntry(Node *head, int id, double value) 
{
    Node *newNode = (Node*)malloc(sizeof(Node));
    if (!newNode) {
        printf("Memory allocation failed!\n");
        return head;
    }
    newNode->id = id;
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
        printf("ID: %d, Value: %.9f\n", temp->id, temp->value);
        temp = temp->next;
    }
}

/*******************************************************************************
*       printIdleDistr(Node *head, double idleTime)
********************************************************************************
* Print Idle Time Distribution (added by Georgia)
* - Input: *head (head of linked-list)
*           idleTime (overall idle time of the simulation)
*******************************************************************************/
void printIdleDistr(Node *head, double idleTime)
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
    
    //print distribution for <10us, < 100us, < 1000us, > 1000us
    printf("-  Distribution of Idle Time: \n");
    printf("-    # idle < 2us        = %.6f \n", distribution[0]/idleTime);
    printf("-    # idle < 20us       = %.6f \n", distribution[1]/idleTime);
    printf("-    # idle < 600us      = %.6f \n", distribution[2]/idleTime);
    printf("-    # idle > 600us      = %.6f \n", distribution[3]/idleTime);

}