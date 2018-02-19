#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>	// for finding element in vector
#include <iomanip>
#include <queue>

// Process class
#include "Process.h"

using namespace std;

typedef vector<Process> Schedule;

// This file assumes:
// 1. Processes arrive at different times
// 2. Processes can have alternating CPU and IO bursts
// 3. There is a max number of context switches.
// 4. There will never be a CPU sequence of only -1.

// Run a round robin schedule
void rr(Schedule process, int quanta, int switches);
// Run a multi-level feedback queue
void mlfq(Schedule processes, int quanta, int switches);

int main()
{
	ifstream in;
	in.open("processes4.txt");

	// Prepare a vector of processes
	Schedule processes;
	readProcessesFromFile(in, processes);

	int quanta, switches;
//	cout << "Enter the length of time slice quanta: "; cin >> quanta;
//	cout << "Enter the max number of context switches: "; cin >> switches;
	quanta = 5;
	switches = 20;

	// Simulate multi-level feedback queue
	mlfq(processes, quanta, switches);

	return 0;
}

// Run a round robin schedule
void rr(Schedule processes, int q, int switches)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule s = sortProcessesByArrivalTime(time, processes);
	// Make another Schedule to hold original burst values
	Schedule original = sortProcessesByArrivalTime(time, processes);

	string activeP;
	// Start the first process
	if(s[0].getArrivalTime() > 0)
	{
		activeP = "IDLE";
		cout << "TIME " << setw(2) << time << ": IDLE." << endl;
	}
	time = s[0].getArrivalTime();
	s[0].start(time);
	activeP = s[0].getId();

	queue<Process> io;

	int active = 0;
	int numS = 0;
	bool allDone = false;
	int t = 0;
	// Run the processes in sorted order until the last process is finished
	while (!allDone && numS < switches)
	{
		// Process IO queue
		if (io.size() > 0)
		{
			string pid = io.front().getId();
			int loc = getProcessLocWithId(pid, s);
			s[loc].setIsReady(true);
			s[loc].setCurrentBurst(io.front().getCurrentBurst()+1);
			original[loc].setIsReady(true);
			original[loc].setCurrentBurst(io.front().getCurrentBurst()+1);
			io.pop();
		}

		if (s[active].getIsReady() && s[active].getArrivalTime() <= time && !s[active].isDone())
		{
			// See if CPU or IO burst for active process
			if (original[active].bursts[original[active].getCurrentBurst()] == 1)
			{
				s[active].setIsReady(false);
				io.push(s[active]);
			}
			else
			{
				activeP = s[active].getId();
				// Burst shorter than quanta
				if (s[active].bursts[s[active].getCurrentBurst()] < q)
				{
					time += s[active].bursts[s[active].getCurrentBurst()];
					if (s[active].getCurrentBurst() == s[active].bursts.size()-1)
					{
						s[active].end(time);
					}
					else
					{
						s[active].setCurrentBurst(s[active].getCurrentBurst()+1);
						if (s[active].bursts[s[active].getCurrentBurst()] == -1)
						{
							s[active].setCurrentBurst(0);
						}
					}
				}
				// Burst longer than quanta
				else if (s[active].bursts[s[active].getCurrentBurst()] > q)
				{
					time += q;
					s[active].bursts[s[active].getCurrentBurst()] -= q;
				}
				// Burst ends at end of quanta
				else
				{
					time += q;
					if (s[active].getCurrentBurst() == s[active].bursts.size()-1)
					{
						s[active].end(time);
					}
					else
					{
						s[active].setCurrentBurst(s[active].getCurrentBurst()+1);
						if (s[active].bursts[s[active].getCurrentBurst()] == -1)
						{
							s[active].setCurrentBurst(0);
						}
					}
				}
			}
		}

		bool allIo = true;
		bool noneArrived = true;
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if (s[i].getIsReady())
				allIo = false;
			if (s[i].getArrivalTime() <= time)
				noneArrived = false;
		}

		if (allIo || noneArrived)
		{
			time++;
			if (activeP != "IDLE")
			{
				activeP = "IDLE";
				cout << "TIME " << setw(2) << time << ": " << "IDLE." << endl;
			}
		}

		// See if the simulation is over
		allDone = true;
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if (!s[i].isDone())
				allDone = false;
		}

		// If it is not over, context switch
		if (!allDone)
		{
			// Increment the active only one time
			// This way, if all processes are blocked or haven't arrived
			// io burst can run while the cpu burst doesnt run
			active = (active+1) % s.size();

			if (s[active].getArrivalTime() <= time)
			{
				if(!s[active].isDone())
				{
					if (s[active].getId() != activeP)
					{
						if (original[active].bursts[original[active].getCurrentBurst()] != 1)
							s[active].start(time);
					}
					numS++;
				}
			}
		}
	}

	cout << "TIME " << time << ": END." << endl;

	if (numS <= switches && allDone)
	{
		calcAvgTurnaroundAndResponse(s);
	}
	else
	{
		cout << "Max number of context switches was reached before all processes ended." << endl;
	}
}

// Run a multi-level feedback queue
void mlfq(Schedule processes, int quanta, int switches)
{
	int time = 0;

	// Start by putting all processes into priority1 queue


}