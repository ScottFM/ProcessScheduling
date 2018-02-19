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
// 6. A CPU burst will come first and last. An IO burst will never start or finish the sequence
// 7. If a -1 is in the burst sequence, it will always follow a CPU burst

// Run a first come first serve schedule with processes
void fcfs(Schedule process, int switches);
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

	int quanta, switches;
	cout << "PART 3- Complex Bursts" << endl;
//	cout << "Enter the length of time slice quanta for round robin: "; cin >> quanta;
//	cout << "Enter the max number of context switches: "; cin >> switches;
	quanta = 2;
	switches = 20;

	//cout << endl << endl;
	//cout << "//////////////////////// FCFS ////////////////////////" << endl;
	//// Simulate first come, first served
	//fcfs(processes, switches);

	//cout << endl << endl;
	//cout << "//////////////////////// RR /////////////////////////" << endl;
	//// Simulate round robin
	//rr(processes, quanta, switches);

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

	// Start the first process
	string activeP;
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
		// Clean out a process from IO if it got missed somehow
		if (io.size() > 0 && io.front().burstsLeft[io.front().getCurrentBurst()] == 0)
		{
			string pid = io.front().getId();
			int loc = getProcessLocWithId(pid, s);
			s[loc].setIsReady(true);
			s[loc].setCurrentBurst(s[active].getCurrentBurst()+1);
			if (s[loc].bursts[s[loc].getCurrentBurst()] == -1)
			{
				s[loc].setCurrentBurst(0);
			}
			io.pop();
		}

		if (s[active].getIsReady() && s[active].getArrivalTime() <= time && !s[active].isDone())
		{
			// See if CPU or IO burst is scheduled for active process
			// Add process to queue if it is at IO burst
			if (s[active].getCurrentBurst() % 2 == 1)
			{
				s[active].setIsReady(false);
				io.push(s[active]);
			}
			// Or handle CPU if it is at CPU burst
			else
			{
				// Let this process run for its duration
				time += s[active].bursts[s[active].getCurrentBurst()];

				// And as time passes, update those processes waiting in the IO queue
				int countdown = s[active].bursts[s[active].getCurrentBurst()];
				while (io.size() > 0 && io.front().burstsLeft[io.front().getCurrentBurst()] <= countdown)
				{
					countdown -= io.front().burstsLeft[io.front().getCurrentBurst()];
					string pid = io.front().getId();
					int loc = getProcessLocWithId(pid, s);
					s[loc].setIsReady(true);
					s[loc].setCurrentBurst(s[active].getCurrentBurst()+1);
					if (s[loc].bursts[s[loc].getCurrentBurst()] == -1)
					{
						s[loc].setCurrentBurst(0);
					}
					io.pop();
				}
				if(io.size() > 0 && countdown > 0)
				{
					io.front().burstsLeft[io.front().getCurrentBurst()] -= countdown;
				}

				// Finish active process if it is at its end
				if (s[active].getCurrentBurst() == s[active].bursts.size()-1)
				{
					s[active].end(time);
				}
				// Or else continue on to the process's next burst
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

		bool allIoOrNotArrived = true;
		allDone = true;
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if (s[i].getIsReady() && s[i].getArrivalTime() <= time && !s[i].isDone())
				allIoOrNotArrived = false;
			if (!s[i].isDone())
				allDone = false;
		}

		if (allIoOrNotArrived && !allDone )
		{
			if (activeP != "IDLE")
			{
				activeP = "IDLE";
				cout << time << ":IDLE ";
			}
			time++;
			if (io.size() > 0)
			{
				string pid = io.front().getId();
				int loc = getProcessLocWithId(pid, s);
				if (io.front().burstsLeft[io.front().getCurrentBurst()] == 1)
				{
					s[loc].setIsReady(true);
					s[loc].setCurrentBurst(s[loc].getCurrentBurst()+1);
					if (s[loc].bursts[s[loc].getCurrentBurst()] == -1)
					{
						s[loc].setCurrentBurst(0);
					}
					io.pop();
				}
				else
				{
					io.front().burstsLeft[s[loc].getCurrentBurst()] -= 1;
				}
			}
		}

		// If it is not over, context switch
		if (!allDone)
		{
			// Increment the active only one time
			active = (active+1) % s.size();
			while (s[active].getArrivalTime() > time || s[active].isDone() || s[active].getId() == activeP && !s[active].getIsReady())
				active = (active+1) % s.size();

			if(s[active].getCurrentBurst() % 2 != 1 && !s[active].isDone())
			{
				if(s[active].getId() != activeP)
				{
					s[active].start(time);
					numS++;
					activeP = s[active].getId();
				}
			}
		}
	}

	cout << time << ":END." << endl;

	bool allFiniteEnded = true;
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (s[i].getRunTime() != -1 && !s[i].isDone())
			allFiniteEnded = false;
	}

	if (numS <= switches && allFiniteEnded)
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

	// Start the first process
	string activeP;
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
	// Run the processes in sorted order until the last process is finished
	while (!allDone && numS < switches)
	{
		// Clean out a process from IO if it got missed somehow
		if (io.size() > 0 && io.front().burstsLeft[io.front().getCurrentBurst()] == 0)
		{
			string pid = io.front().getId();
			int loc = getProcessLocWithId(pid, s);
			s[loc].setIsReady(true);
			s[loc].setCurrentBurst(s[active].getCurrentBurst()+1);
			if (s[loc].bursts[s[loc].getCurrentBurst()] == -1)
			{
				s[loc].setCurrentBurst(0);
			}
			io.pop();
		}

		if (s[active].getIsReady() && s[active].getArrivalTime() <= time && !s[active].isDone())
		{
			// See if CPU or IO burst is scheduled for active process
			// Add process to queue if it is at IO burst
			if (s[active].getCurrentBurst() % 2 == 1)
			{
				s[active].setIsReady(false);
				io.push(s[active]);
			}
			// Or handle CPU if it is at CPU burst
			else
			{
				// See if process finishes within quanta
				int timeChange;
				// Process finishes within quanta or right at end
				if (s[active].burstsLeft[s[active].getCurrentBurst()] <= q)
				{
					timeChange = s[active].burstsLeft[s[active].getCurrentBurst()];
				}
				//Process doesn't finish in quanta
				else if (s[active].burstsLeft[s[active].getCurrentBurst()] > q)
				{
					timeChange = q;
				}
				s[active].burstsLeft[s[active].getCurrentBurst()] -= timeChange;
				time += timeChange;

				// And as time passes, update those processes waiting in the IO queue
				while (io.size() > 0 && io.front().burstsLeft[io.front().getCurrentBurst()] <= timeChange)
				{
					timeChange -= io.front().burstsLeft[io.front().getCurrentBurst()];
					string pid = io.front().getId();
					int loc = getProcessLocWithId(pid, s);
					s[loc].setIsReady(true);
					s[loc].setCurrentBurst(s[active].getCurrentBurst()+1);
					if (s[loc].bursts[s[loc].getCurrentBurst()] == -1)
					{
						s[loc].setCurrentBurst(0);
					}
					io.pop();
				}
				if(io.size() > 0 && timeChange > 0)
				{
					io.front().burstsLeft[io.front().getCurrentBurst()] -= timeChange;
				}

				// Finish active process if it is at its end
				if (s[active].burstsLeft[s[active].getCurrentBurst()] <= 0)
				{
					if (s[active].getCurrentBurst() == s[active].bursts.size()-1)
					{
						s[active].end(time);
					}
					// Or else continue on to the process's next burst
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

		bool allIoOrNotArrived = true;
		allDone = true;
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if (s[i].getIsReady() && s[i].getArrivalTime() <= time && !s[i].isDone())
				allIoOrNotArrived = false;
			if (!s[i].isDone())
				allDone = false;
		}

		if (allIoOrNotArrived && !allDone )
		{
			if (activeP != "IDLE")
			{
				activeP = "IDLE";
				cout << time << ":IDLE ";
			}
			time++;
			if (io.size() > 0)
			{
				string pid = io.front().getId();
				int loc = getProcessLocWithId(pid, s);
				if (io.front().burstsLeft[io.front().getCurrentBurst()] == 1)
				{
					s[loc].setIsReady(true);
					s[loc].setCurrentBurst(s[loc].getCurrentBurst()+1);
					if (s[loc].bursts[s[loc].getCurrentBurst()] == -1)
					{
						s[loc].setCurrentBurst(0);
					}
					io.pop();
				}
				else
				{
					io.front().burstsLeft[s[loc].getCurrentBurst()] -= 1;
				}
			}
		}

		// If it is not over, context switch
		if (!allDone)
		{
			// Increment the active only one time
			active = (active+1) % s.size();
			while (s[active].getArrivalTime() > time || s[active].isDone() || s[active].getId() == activeP && !s[active].getIsReady())
				active = (active+1) % s.size();

			if(s[active].getCurrentBurst() % 2 != 1 && !s[active].isDone())
			{
				if(s[active].getId() != activeP)
				{
					s[active].start(time);
					numS++;
					activeP = s[active].getId();
				}
			}
		}
	}

	cout << time << ":END." << endl;

	bool allFiniteEnded = true;
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (s[i].getRunTime() != -1 && !s[i].isDone())
			allFiniteEnded = false;
	}

	if (numS <= switches && allFiniteEnded)
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

	// Start the first process
	string activeP;
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
		// Sort the process according to next burst size
		s = sortProcessesByRunTime(s);

		// Clean out a process from IO if it got missed somehow
		if (io.size() > 0 && io.front().burstsLeft[io.front().getCurrentBurst()] == 0)
		{
			string pid = io.front().getId();
			int loc = getProcessLocWithId(pid, s);
			s[loc].setIsReady(true);
			s[loc].setCurrentBurst(s[active].getCurrentBurst()+1);
			if (s[loc].bursts[s[loc].getCurrentBurst()] == -1)
			{
				s[loc].setCurrentBurst(0);
			}
			io.pop();
		}

		if (s[active].getIsReady() && s[active].getArrivalTime() <= time && !s[active].isDone())
		{
			// See if CPU or IO burst is scheduled for active process
			// Add process to queue if it is at IO burst
			if (s[active].getCurrentBurst() % 2 == 1)
			{
				s[active].setIsReady(false);
				io.push(s[active]);
			}
			// Or handle CPU if it is at CPU burst
			else
			{
				// Let this process run for its duration
				time += s[active].bursts[s[active].getCurrentBurst()];

				// And as time passes, update those processes waiting in the IO queue
				int countdown = s[active].bursts[s[active].getCurrentBurst()];
				while (io.size() > 0 && io.front().burstsLeft[io.front().getCurrentBurst()] <= countdown)
				{
					countdown -= io.front().burstsLeft[io.front().getCurrentBurst()];
					string pid = io.front().getId();
					int loc = getProcessLocWithId(pid, s);
					s[loc].setIsReady(true);
					s[loc].setCurrentBurst(s[active].getCurrentBurst()+1);
					if (s[loc].bursts[s[loc].getCurrentBurst()] == -1)
					{
						s[loc].setCurrentBurst(0);
					}
					io.pop();
				}
				if(io.size() > 0 && countdown > 0)
				{
					io.front().burstsLeft[io.front().getCurrentBurst()] -= countdown;
				}

				// Finish active process if it is at its end
				if (s[active].getCurrentBurst() == s[active].bursts.size()-1)
				{
					s[active].end(time);
				}
				// Or else continue on to the process's next burst
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

		bool allIoOrNotArrived = true;
		allDone = true;
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if (s[i].getIsReady() && s[i].getArrivalTime() <= time && !s[i].isDone())
				allIoOrNotArrived = false;
			if (!s[i].isDone())
				allDone = false;
		}

		if (allIoOrNotArrived && !allDone )
		{
			if (activeP != "IDLE")
			{
				activeP = "IDLE";
				cout << time << ":IDLE ";
			}
			time++;
			if (io.size() > 0)
			{
				string pid = io.front().getId();
				int loc = getProcessLocWithId(pid, s);
				if (io.front().burstsLeft[io.front().getCurrentBurst()] == 1)
				{
					s[loc].setIsReady(true);
					s[loc].setCurrentBurst(s[loc].getCurrentBurst()+1);
					if (s[loc].bursts[s[loc].getCurrentBurst()] == -1)
					{
						s[loc].setCurrentBurst(0);
					}
					io.pop();
				}
				else
				{
					io.front().burstsLeft[s[loc].getCurrentBurst()] -= 1;
				}
			}
		}

		// If it is not over, context switch
		if (!allDone)
		{
			// Increment the active only one time
			active = (active+1) % s.size();
			while (s[active].getArrivalTime() > time || s[active].isDone() || s[active].getId() == activeP && !s[active].getIsReady())
				active = (active+1) % s.size();

			if(s[active].getCurrentBurst() % 2 != 1 && !s[active].isDone())
			{
				if(s[active].getId() != activeP)
				{
					s[active].start(time);
					numS++;
					activeP = s[active].getId();
				}
			}
		}
	}

	cout << time << ":END." << endl;

	bool allFiniteEnded = true;
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (s[i].getRunTime() != -1 && !s[i].isDone())
			allFiniteEnded = false;
	}

	if (numS <= switches && allFiniteEnded)
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

	// Start the first process
	string activeP;
	if(s[active].getArrivalTime() > 0)
	{
		activeP = "IDLE";
		cout << time << ":IDLE ";
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
				cout << time << ":IDLE " << endl;
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

	cout << time << ":END." << endl;

	if (numS <= switches && allDone)
	{
		calcAvgTurnaroundAndResponse(s);
	}
	else
	{
		cout << "Max number of context switches was reached before all processes ended." << endl;
	}
}*/