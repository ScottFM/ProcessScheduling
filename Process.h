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
#endif // PROCESS_H