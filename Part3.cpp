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
// 1. Processes arrive at different times
// 2. Processes can have alternating CPU and IO bursts
// 3. There is a max number of context switches.

// Run a first come first serve schedule with processes
void fcfs(Schedule process, int quanta);
// Run a round robin schedule
void rr(Schedule process, int quanta);
// Run a shortest job first schedule
void sjf(Schedule processes, int quanta);

int main()
{
	ifstream in;
	in.open("processes.txt");

	// Prepare a vector of processes
	Schedule processes;
	readProcessesFromFile(in, processes);
//	for (int i = 0; i < processes.size(); i++)
//		processes[i].printBursts(cout);

	int quanta;
	cout << "Enter the length of time slice quanta: "; cin >> quanta;

	cout << endl << endl;
	cout << "//////////////////////// FCFS ////////////////////////" << endl;
	// Simulate first come, first served
	fcfs(processes, quanta);

	cout << endl << endl;
	cout << "//////////////////////// RR /////////////////////////" << endl;
	// Simulate round robin
	rr(processes, quanta);

	cout << endl << endl;
	cout << "//////////////////////// SJF ////////////////////////" << endl;
	// Simulate shortest job first
	rr(processes, quanta);

	return 0;
}

// Run a first come first serve schedule with processes
void fcfs(Schedule processes, int q)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule sortedProcesses = sortProcessesByArrivalTime(time, processes);

	int p = 0;
	// Run the processes in sorted order until the last process is finished
	while (!sortedProcesses[sortedProcesses.size()-1].isDone())
	{
		if (sortedProcesses[p].timeUntilArrival(time) <= 0 && sortedProcesses[p].getStartTime() == -1)
		{
			sortedProcesses[p].start(time);
			time++;
		}
		else if (sortedProcesses[p].getRunTime() != -1)
		{
			time += sortedProcesses[p].getRunTime();
			sortedProcesses[p].end(time);
			p++;
		}
		// How do you process if burst has a -1?
		/////////////////////////////////////////////////////////////////////////////////////////HERE
		else
		{
			// This process runs indefinitely in FCFS
			throw("Houston, we've got a problem.");
		}
	}

	cout << "TIME " << time << ": END." << endl;

	calcAvgTurnaroundAndResponse(sortedProcesses);
}

// Run a round robin schedule
void rr(Schedule processes, int q)
{
	int time = 0;

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
		// If it is still the turn for the active process, advance time
//		cout << tempTime << " " << q << " " << tempTime%q << endl;
		if (((tempTime == 0) || ((tempTime % q) > 0)) && (sortedProcesses[active].bursts[sortedProcesses[active].currentBurst] > 0))
		{
			tempTime++;
			time++;
			sortedProcesses[active].bursts[sortedProcesses[active].currentBurst]--;
		}
		// If the active process finished its burst, start next process
		else
		{
			if (sortedProcesses[active].bursts[sortedProcesses[active].bursts.size()-1] == 0)
			{
				sortedProcesses[active].end(time);
			}
			if (sortedProcesses[active].bursts[sortedProcesses[active].currentBurst] == 0)
			{
				sortedProcesses[active].currentBurst++;
			}

			allDone = true;
			for (unsigned int i = 0; i < processes.size(); i++)
			{
				if (!sortedProcesses[i].isDone())
					allDone = false;
			}

			active = (active+1) % sortedProcesses.size();
			while ((sortedProcesses[active].isDone() || time < sortedProcesses[active].getArrivalTime()) && !allDone)
			{
				active = (active+1) % sortedProcesses.size();
			}
			if (!allDone)
			{
				sortedProcesses[active].start(time);
				tempTime = 0;
			}
		}


		allDone = true;
		for (unsigned int i = 0; i < processes.size(); i++)
		{
			if (!sortedProcesses[i].isDone())
				allDone = false;
		}
	}

	cout << "TIME " << time << ": END." << endl;

	calcAvgTurnaroundAndResponse(sortedProcesses);
}

// Run a shortest job first schedule
void sjf(Schedule processes, int q)
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
	cout << "TIME " << time << ": END." << endl;

	calcAvgTurnaroundAndResponse(s);
}