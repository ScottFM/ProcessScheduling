#ifndef PROCESS_H
#define PROCESS_H

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>	// find element in vector

using namespace std;

/////////////////// PROCESS CLASS ////////////////////////
class Process
{
public:
	// Default constructor
	Process() {};

	// Overloaded constructor
	Process(string id, int arrTime, vector<int> newBursts);
	~Process() {};

	string getId();
	void setId(string s);
	int getArrivalTime();
	void setArrivalTime(int t);
	void printBursts(ostream& out);
	int timeUntilArrival(int time);
	bool isDone();
	void setIsDone(bool isDone);
	void start(int time);
	int getStartTime();
	void end(int time);
	int getTurnaround();
	int getRunTime();
	int getResponse();
	
	vector<int> bursts;
	int currentBurst;
private:
	string id;
	int arrivalTime;
	bool done;
	int startTime;
	int finishTime;
	int turnaround;
	int runTime;
	int response;
};
/////////////////// END PROCESS CLASS ////////////////////

typedef vector<Process> Schedule;

/////// OTHER HELPER FUNCTIONS TO USE FOR EACH PART //////
// tturn a string of integers into a vector
vector<int> processBurstString(string bString);

// Fill a vector of processes from a file; close file
void readProcessesFromFile(ifstream& in, Schedule& processes);

// Helper function to sort unfinished processes according to time until they arrive
// The processes have finished and have >= 0 time units away from arriving
Schedule sortProcessesByArrivalTime(int time, Schedule processes);

// Helper function to sort unfinished processes according to run time
Schedule sortProcessesByRunTime(Schedule processes);

// The processes have finished and have are >= 0 time units away from arriving
Schedule sortProcessesByArrivalTime(int time, Schedule processes);

// Helper function to compute and output averages
void calcAvgTurnaroundAndResponse(Schedule s);


#endif // PROCESS_H