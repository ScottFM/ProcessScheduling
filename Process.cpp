#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>	// find element in vector
#include <iomanip>
#include <sstream>
#include "Process.h"

using namespace std;

/////////////////// PROCESS CLASS ////////////////////////
// Overloaded constructor
Process::Process(string id, int arrTime, vector<int> newBursts)
{
	setId(id);
	setArrivalTime(arrTime);
	bursts = newBursts;
	burstsLeft = newBursts;
	setIsDone(false);
	startTime = -1;
	finishTime = -1;
	turnaround = -1;
	response = -1;
	setCurrentBurst(0);
	setIsReady(true);
	setBurstAvg(0);
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
	for (unsigned int i  = 0; i <= bursts.size()-1; i++)
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
	{
		startTime = time;
	}
	if (response == -1)
		response = time - arrivalTime;
	cout << time << ":" << getId() << " ";
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
bool Process::getIsReady()
{
	return isReady;
}
void Process::setIsReady(bool ready)
{
	isReady = ready;
}
int Process::getCurrentBurst()
{
	return currentBurst;
}
void Process::setCurrentBurst(int b)
{
	currentBurst = b;
}
int Process::getBurstAvg()
{
	return burstAvg;
}
void Process::setBurstAvg(int avg)
{
	burstAvg = avg;
}
int Process::getTickets()
{
	return tickets;
}
void Process::setTickets(int t)
{
	tickets = t;
}
/////////////////// END PROCESS CLASS ////////////////////

typedef vector<Process> Schedule;

/////// OTHER HELPER FUNCTIONS TO USE FOR EACH PART //////
// Turn a string of integers from file into a vector
vector<int> processBurstString(string bString)
{
	vector<int> bVec;
	std::stringstream stream(bString);

	int number;
	while (stream >> number)
		bVec.push_back(number);

	return bVec;
}

// Fill a vector of processes from a file
void readProcessesFromFile(ifstream& in, vector<Process>& processes)
{
	string id;
	int arrival;
	vector<int> bursts;

	// Read the file line by line
	while (!in.eof())
	{
		in >> id >> arrival;

		// Read the bursts number by number
		string burstString = "";
		char z;
		in.get(z);
		while (z != '\n' && !in.eof())
		{
			burstString += z;
			in.get(z);
		}
		bursts = processBurstString(burstString);
		Process p = Process(id, arrival, bursts);
		processes.push_back(p);
	}
	in.close();
}

// Helper function to sort unfinished processes according to time until they arrive
// The processes have not arrived yet
Schedule sortProcessesByArrivalTime(int time, Schedule processes)
{
	Schedule sortedByTimes;
	sortedByTimes.push_back(processes[0]);

	// First find the earliest arrival time for any process
	for (unsigned int i = 1; i < processes.size(); i++)
	{
		unsigned int idx = 0;
		bool stop = false;
		if (processes[i].timeUntilArrival(time) >= 0)
		{
			while (idx < sortedByTimes.size() && stop != true)
			{
				if (processes[i].timeUntilArrival(time) >= sortedByTimes[idx].timeUntilArrival(time))
				{
					idx++;
				}
				else
				{
					stop = true;
				}
			}
			sortedByTimes.insert(sortedByTimes.begin()+idx, processes[i]);
		}
	}

	return sortedByTimes;
}

// Helper function to sort unfinished processes according to run time
// Sort according to element at current burst
Schedule sortProcessesByRunTime(Schedule processes)
{
	Schedule sorted;
	sorted.push_back(processes[0]);

	// First find the earliest arrival time for any process
	for (unsigned int i = 1; i < processes.size(); i++)
	{
		unsigned int idx = 0;
		bool stop = false;
		// Find next CPU burst
		int burst = processes[i].getCurrentBurst();
		if (processes[i].getCurrentBurst() % 2 == 1)
		{
			burst++;
			if (processes[i].bursts[burst] == -1)
			{
				burst = 0;
			}
		}
		while (idx < sorted.size() && stop != true)
		{
			if (processes[i].burstsLeft[burst] >= sorted[idx].bursts[sorted[idx].getCurrentBurst()])
			{
				idx++;
			}
			else
			{
				stop = true;
			}
		}
		sorted.insert(sorted.begin()+idx, processes[i]);
	}

	return sorted;
}

// Helper function to sort unfinished processes according to run time
// Sort according to element at current burst
Schedule sortProcessesByAvg(Schedule processes)
{
	Schedule sorted;
	sorted.push_back(processes[0]);

	// First find the earliest arrival time for any process
	for (unsigned int i = 1; i < processes.size(); i++)
	{
		unsigned int idx = 0;
		bool stop = false;
		while (idx < sorted.size() && stop != true)
		{
			if (processes[i].getBurstAvg() >= sorted[idx].getBurstAvg())
			{
				idx++;
			}
			else
			{
				stop = true;
			}
		}
		sorted.insert(sorted.begin()+idx, processes[i]);
	}

	return sorted;
}

// Helper function to compute and output averages
void calcAvgTurnaroundAndResponse(Schedule s)
{
	float avgTurnaround = 0;
	float avgResponseTime = 0;
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (s[i].isDone())
		{
			// Calculate average turnaround and response time
			avgTurnaround += s[i].getTurnaround();
		}
		avgResponseTime += s[i].getResponse();
	}
	avgTurnaround /= s.size();
	avgResponseTime /= s.size();

	cout << "Average turnaround time for finite processes was: " << avgTurnaround << endl;
	cout << "Average response time was: " << avgResponseTime << endl;
}

// Helper function to modify object in queue
int getProcessLocWithId(string id, Schedule s)
{
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (s[i].getId() == id)
			return i;
	}
}