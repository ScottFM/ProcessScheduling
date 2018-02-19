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
void rr(Schedule process, int quanta);
// Run a shortest job first schedule
void sjf(Schedule processes);

int main()
{
	ifstream in;
	in.open("processes2.txt");

	// Prepare a vector of processes
	Schedule processes;
	readProcessesFromFile(in, processes);

	int quanta;
	cout << "PART 2- Simple Burst Stats" << endl;
	cout << "Enter the length of time slice quanta for round robin: "; cin >> quanta;

	cout << endl << endl;
	cout << "//////////////////////// FCFS ////////////////////////" << endl;
	// Simulate first come, first served
	fcfs(processes);

	cout << endl << endl;
	cout << "//////////////////////// RR /////////////////////////" << endl;
	// Simulate round robin
	rr(processes, quanta);

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

	cout << time << ":END" << endl;

	calcAvgTurnaroundAndResponse(sortedProcesses);
}

// Run a round robin schedule
void rr(Schedule processes, int quanta)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule sortedProcesses = sortProcessesByArrivalTime(time, processes);

	// Get earliest arrival time
	time = sortedProcesses[0].getArrivalTime();
	sortedProcesses[0].start(time);
	int active = 0;
	string activeP = sortedProcesses[active].getId();

	bool allDone = false;
	while (!allDone)
	{
		// If process should end after quanta
		if (sortedProcesses[active].getRunTime() == quanta)
		{
			time += quanta;
			sortedProcesses[active].end(time);
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
			if (sortedProcesses[active].getId() != activeP)
			{
				sortedProcesses[active].start(time);
				activeP = sortedProcesses[active].getId();
			}
		}
	}

	cout << time << ":END" << endl;

	calcAvgTurnaroundAndResponse(sortedProcesses);
}

// Run a shortest job first schedule
void sjf(Schedule processes)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule s = sortProcessesByRunTime(processes);

	// Start the first possible shortest process
	int active = 0;
	string activeP;
	bool started = false;
	while(!started)
	{
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if (s[i].getArrivalTime() <= time)
			{
				s[i].start(time);
				active = i;
				activeP = s[i].getId();
				started = true;
				break;
			}
		}
		time++;
	}	

	bool allDone = false;
	while (!allDone)
	{
		time++;
		s[active].bursts[0]--;
		s = sortProcessesByRunTime(s);
		// Check if the active process should end
		if (s[active].bursts[0] == 0)
		{
			s[active].end(time);

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
			active = 0;
			while ((s[active].isDone() || time < s[active].getArrivalTime()) && !allDone)
			{
				active = (active+1) % s.size();
			}
			if (s[active].getId() != activeP)
			{
				s[active].start(time);
				activeP = s[active].getId();
			}
		}
	}
	cout << time << ":END" << endl;

	calcAvgTurnaroundAndResponse(s);
}