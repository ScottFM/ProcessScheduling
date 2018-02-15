#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>	// for finding element in vector
#include <iomanip>

// Process class
#include "Process.h"

using namespace std;

typedef vector<Process> Schedule;

// This file assumes:
// 1. The process has only one CPU intensive burst

// Run a first come first serve schedule with processes
void fcfs(Schedule process);
// Run a round robin schedule
void rr(Schedule process);
// Run a shortest job first schedule
void sjf(Schedule processes);

int main()
{
	ifstream in;
	in.open("processes2.txt");

	// Prepare a vector of processes
	Schedule processes;
	readProcessesFromFile(in, processes);

	cout << "//////////////////////// FCFS ////////////////////////" << endl;
	// Simulate first come, first served
	fcfs(processes);

	cout << endl << endl;
	cout << "//////////////////////// RR /////////////////////////" << endl;
	// Simulate round robin
	rr(processes);

	cout << endl << endl;
	cout << "//////////////////////// SJF ////////////////////////" << endl;
	// Simulate shortest job first
	sjf(processes);

	return 0;
}

// Run a first come first serve schedule with processes
void fcfs(Schedule processes)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule sortedProcesses = sortProcessesByArrivalTime(time, processes);

	// Start the earliest process.
	time = sortedProcesses[0].getArrivalTime();

	for (unsigned int i = 0; i < sortedProcesses.size(); i++)
	{
		sortedProcesses[i].start(time);
		time += sortedProcesses[i].getRunTime();
		sortedProcesses[i].end(time);
	}

	cout << "TIME " << time << ": END." << endl;

	calcAvgTurnaroundAndResponse(sortedProcesses);
}

// Run a round robin schedule
void rr(Schedule processes)
{
	int time = 0;
	int quanta;
	cout << "Enter the length of time slice quanta: "; cin >> quanta;

	// Sort the processes by time until they arrive
	Schedule sortedProcesses = sortProcessesByArrivalTime(time, processes);

	// Get earliest arrival time
	time = sortedProcesses[0].getArrivalTime();
	sortedProcesses[0].start(time);
	int tempTime = 0;
	int active = 0;

	bool allDone = false;
	while (!allDone)
	{
		// If process should end after quanta
		if (sortedProcesses[active].getRunTime() == quanta)
		{
			time += quanta;
			sortedProcesses[active].end(time);
			// active = (active+1) % sortedProcesses.size();
		}
		// Process will run over its quanta
		else if (sortedProcesses[active].getRunTime() > quanta)
		{
			time += quanta;
			sortedProcesses[active].bursts[0] -= quanta;
		}
		// Process will end before quanta ends
		else if (sortedProcesses[active].getRunTime() < quanta)
		{
			time += sortedProcesses[active].getRunTime();
			sortedProcesses[active].end(time);
		}

		// See if the Round robin is over
		allDone = true;
		for (unsigned int i = 0; i < processes.size(); i++)
		{
			if (!sortedProcesses[i].isDone())
				allDone = false;
		}

		// If it is not over, context switch
		if (!allDone)
		{
			active = (active+1) % sortedProcesses.size();
			while ((sortedProcesses[active].isDone() || time < sortedProcesses[active].getArrivalTime()) && !allDone)
			{
				active = (active+1) % sortedProcesses.size();
			}
			sortedProcesses[active].start(time);
		}
	}

	cout << "TIME " << time << ": END." << endl;

	calcAvgTurnaroundAndResponse(sortedProcesses);
}

// Run a shortest job first schedule
void sjf(Schedule processes)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule s = sortProcessesByRunTime(processes);

	// Get earliest arrival time
	time = s[0].getArrivalTime();
	s[0].start(time);
	int tempTime = 0;
	int active = 0;

	bool allDone = false;
	while (!allDone)
	{
		// Check if the active process will end
		i
	}


	// See if the Shortest Job First is over
	allDone = true;
	for (unsigned int i = 0; i < processes.size(); i++)
	{
		if (!s[i].isDone())
			allDone = false;
	}

	// If it is not over, context switch
	if (!allDone)
	{
		active = (active+1) % s.size();
		while ((s[active].isDone() || time < s[active].getArrivalTime()) && !allDone)
		{
			active = (active+1) % s.size();
		}
		s[active].start(time);
	}


}