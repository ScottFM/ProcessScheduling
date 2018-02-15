#include <iostream>
#include <string>
#include <vector>
#include <algorithm>	// find element in vector
#include <iomanip>
#include "Process.h"

using namespace std;

/////////////////// PROCESS CLASS ////////////////////////

// Overloaded constructor
Process::Process(string id, int arrTime, vector<int> newBursts)
{
	setId(id);
	setArrivalTime(arrTime);
	bursts = newBursts;
	done = false;
	startTime = -1;
	finishTime = -1;
	turnaround = -1;
	response = -1;
	currentBurst = 0;
}

string Process::getId()
{
	return id;
}
void Process::setId(string s)
{
	id = s;
}
int Process::getArrivalTime()
{
	return arrivalTime;
}
void Process::setArrivalTime(int t)
{
	arrivalTime = t;
}
void Process::printBursts(ostream& out)
{
	out << "[ ";
	for (int i  = 0; i <= bursts.size()-1; i++)
	{
		out << bursts[i] << " ";
	}
	out << "]" << endl;
}
int Process::timeUntilArrival(int time)
{
	return arrivalTime-time;
}
bool Process::isDone()
{
	return done;
}
void Process::setIsDone(bool isDone)
{
	done = isDone;
}
void Process::start(int time)
{
	if (startTime == -1)
		startTime = time;
	if (response == -1)
		response = time - arrivalTime;
	cout << "TIME " << setw(2) << time << ": " << getId() << " is running in CPU." << endl;
}
int Process::getStartTime()
{
	return startTime;
}
void Process::end(int time)
{
	done = true;
	finishTime = time;
	turnaround = finishTime - startTime;
}
int Process::getTurnaround()
{
	return turnaround;
}
int Process::getRunTime()
{
	if (std::find(bursts.begin(), bursts.end(), -1) != bursts.end())
	{
		// Contains a -1. Set accordingly
		runTime = -1;
	}
	else
	{
		int sum = 0;
		for(vector<int>::iterator it = bursts.begin(); it != bursts.end(); ++it)
			sum += *it;
		runTime = sum;
	}
	return runTime;
}
int Process::getResponse()
{
	return response;
}
	
/////////////////// END PROCESS CLASS ////////////////////