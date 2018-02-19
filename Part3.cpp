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
void sjf(Schedule processes, int switches);

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
	cout << "Enter the length of time slice quanta: "; cin >> quanta;
	cout << "Enter the max number of context switches: "; cin >> switches;
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

	cout << endl << endl;
	cout << "//////////////////////// SJF ////////////////////////" << endl;
	// Simulate shortest job first
	sjf(processes, switches);

	return 0;
}

// Run a first come first serve schedule with processes
void fcfs(Schedule processes, int switches)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule s = sortProcessesByArrivalTime(time, processes);

	string activeP;
	// Start the first process
	if(s[0].getArrivalTime() > 0)
	{
		activeP = "IDLE";
		cout << time << ":IDLE";
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

// Run a shortest job first schedule
void sjf(Schedule processes, int switches)

{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule s = sortProcessesByArrivalTime(time, processes);

	int currentTime = s[0].bursts[0];
	int active = 0;
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if(s[i].bursts[0] < currentTime && s[i].getArrivalTime() <= s[0].getArrivalTime())
		{
			active = i;
			currentTime = s[i].bursts[0];
		}
	}

	string activeP;
	// Start the first process
	if(s[active].getArrivalTime() > 0)
	{
		activeP = "IDLE";
		cout << "TIME " << setw(2) << time << ": IDLE." << endl;
	}
	time = s[active].getArrivalTime();
	s[active].start(time);
	activeP = s[active].getId();

	queue<Process> io;

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
				activeP = s[active].getId();
				// Burst shorter than quanta
				if (s[active].burstsLeft[s[active].getCurrentBurst()] == 1)
				{
					time++;
					s[active].burstsLeft[s[active].getCurrentBurst()]--;
					if(s[active].getCurrentBurst() == s[active].bursts.size()-1)
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
				else if (s[active].burstsLeft[s[active].getCurrentBurst()] > 1)
				{
					time++;
					s[active].burstsLeft[s[active].getCurrentBurst()]--;
				}
			}
		}

		bool allIo = true;
		bool noneArrived = true;
		allDone = true;
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if (s[i].getIsReady())
				allIo = false;
			if (s[i].getArrivalTime() <= time)
				noneArrived = false;
			if (!s[i].isDone())
				allDone = false;
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

		// If it is not over, context switch
		if (!allDone)
		{
			int currentLow;
			int tempActive = -1;
			if (s[active].burstsLeft[s[active].getCurrentBurst()] == 0 ||
				s[active].bursts[s[active].getCurrentBurst()] == 1)
			{
				currentLow = 1000;
				tempActive = active;
			}
			else
				currentLow = s[active].burstsLeft[s[active].getCurrentBurst()];
			for (unsigned int i = 0; i < s.size(); i++)
			{
				if(tempActive != -1)
				{
					if(i != tempActive)
					{
						if(s[i].burstsLeft[s[i].getCurrentBurst()] < currentLow && s[i].getArrivalTime() <= time && !s[i].isDone() )
						{
							active = i;
							break;
						}
					}
				}
				else
				{
					if(s[i].burstsLeft[s[i].getCurrentBurst()] < currentLow && s[i].getArrivalTime() <= time && !s[i].isDone() )
					{
						active = i;
						break;
					}
				}
			}
			if (s[active].getId() != activeP)
			{
				if (s[active].bursts[s[active].getCurrentBurst()] != 1)
				{
					s[active].start(time);
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










