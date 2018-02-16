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
// 5. Quanta do not pertain to FCFS

// Run a first come first serve schedule with processes
void fcfs(Schedule process,  int switches);
// Run a round robin schedule
void rr(Schedule process, int quanta, int switches);
// Run a shortest job first schedule
void sjf(Schedule processes, int quanta, int switches);

int main()
{
	ifstream in;
	in.open("processes3.txt");

	// Prepare a vector of processes
	Schedule processes;
	readProcessesFromFile(in, processes);
//	for (int i = 0; i < processes.size(); i++)
//		processes[i].printBursts(cout);

	int quanta, switches;
	//cout << "Enter the length of time slice quanta: "; cin >> quanta;
	//cout << "Enter the max number of context switches: "; cin >> switches;
	quanta = 2;
	switches = 20;
	cout << endl << endl;
	cout << "//////////////////////// FCFS ////////////////////////" << endl;
	// Simulate first come, first served
	fcfs(processes, switches);

	cout << endl << endl;
	cout << "//////////////////////// RR /////////////////////////" << endl;
	// Simulate round robin
	rr(processes, quanta, switches);

	//cout << endl << endl;
	//cout << "//////////////////////// SJF ////////////////////////" << endl;
	//// Simulate shortest job first
	//rr(processes, quanta, switches);

	return 0;
}

// Run a first come first serve schedule with processes
void fcfs(Schedule processes, int switches)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule s = sortProcessesByArrivalTime(time, processes);

	// Start the first process
	time = s[0].getArrivalTime();
	s[0].start(time);
	string activeP = s[0].getId();

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
			io.pop();
		}

		if (s[active].getIsReady() && s[active].getArrivalTime() <= time && !s[active].isDone())
		{
			// See if CPU or IO burst for active process
			if (s[active].bursts[s[active].getCurrentBurst()] == 1)
			{
				s[active].setIsReady(false);
				io.push(s[active]);
			}
			else
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
		}

		bool allIo = true;
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if (s[i].getIsReady())
				allIo = false;
		}
		if (allIo)
		{
			time++;
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
				if (s[active].bursts[s[active].getCurrentBurst()] != 1)
				{
					if(!s[active].isDone())
					{
						if (s[active].getId() != activeP)
						{
							s[active].start(time);
							numS++;
							activeP = s[active].getId();
						}
						else
						{
							numS++;
						}
					}
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

// Run a round robin schedule
void rr(Schedule processes, int q, int switches)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule s = sortProcessesByArrivalTime(time, processes);
	// Make another Schedule to hold original burst values
	Schedule original = sortProcessesByArrivalTime(time, processes);

	// Start the first process
	time = s[0].getArrivalTime();
	s[0].start(time);
	string activeP = s[0].getId();

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
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if (s[i].getIsReady())
				allIo = false;
		}
		if (allIo)
		{
			time++;
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
/*{
	int time = 0;
	int numS = 0;

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
		if (((tempTime == 0) || ((tempTime % q) > 0)) && (sortedProcesses[active].bursts[sortedProcesses[active].getCurrentBurst()] > 0))
		{
			tempTime++;
			time++;
			sortedProcesses[active].bursts[sortedProcesses[active].getCurrentBurst()]--;
		}
		// If the active process finished its burst, start next process
		else
		{
			if (sortedProcesses[active].bursts[sortedProcesses[active].bursts.size()-1] == 0)
			{
				sortedProcesses[active].end(time);
			}
			if (sortedProcesses[active].bursts[sortedProcesses[active].getCurrentBurst()] == 0)
			{
				sortedProcesses[active].setCurrentBurst(sortedProcesses[active].getCurrentBurst()+1);
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
				numS++;
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

	if (numS <= switches && allDone)
	{
		calcAvgTurnaroundAndResponse(sortedProcesses);
	}
	else
	{
		cout << "Max number of context switches was reached before all processes ended." << endl;
	}
}*/

// Run a shortest job first schedule
void sjf(Schedule processes, int q, int switches)
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