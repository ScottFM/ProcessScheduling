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

/////////////////////// Helper functions ////////////////////////////
// Helper function for finishing
void finish(int time, int num, int switches, Schedule s);
// Helper function to check if schedule is stumped
void checkAllIoOrNotArrived(queue<Process>& io, Schedule& s, string& activeP, int& time, bool& allDone);
// Helper function when nothing can run
void stuck(queue<Process>& io, Schedule& s, string& activeP, int& time);

int main()
{
	ifstream in;
	in.open("processes3.txt");

	// Prepare a vector of processes
	Schedule processes;
	readProcessesFromFile(in, processes);

	int quanta, switches;
	cout << "PART 3- Complex Bursts" << endl;
	cout << "Enter the length of time slice quanta for round robin: "; cin >> quanta;
	cout << "Enter the max number of context switches: "; cin >> switches;
	//quanta = 2;
	//switches = 20;

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

	// Start the first process
	string activeP;
	if (s[0].getArrivalTime() > 0)
	{
		activeP = "IDLE";
		cout << time << ":IDLE ";
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
		// See if the current process is ready to run
		if (s[active].getIsReady() && s[active].getArrivalTime() <= time && !s[active].isDone())
		{
			// First see if you are at the end of a repeating sequence
			if (s[active].bursts[s[active].getCurrentBurst()] == -1)
			{
				s[active].setCurrentBurst(0);
				s[active].burstsLeft = s[active].bursts;
			}
			
			// Let this process run for its CPU burst duration
			time += s[active].bursts[s[active].getCurrentBurst()];

			// And as time passes, update those processes waiting in the IO queue
			int countdown = s[active].bursts[s[active].getCurrentBurst()];
			while (io.size() > 0 && io.front().burstsLeft[io.front().getCurrentBurst()] <= countdown)
			{
				countdown -= io.front().burstsLeft[io.front().getCurrentBurst()];
				string pid = io.front().getId();
				int loc = getProcessLocWithId(pid, s);
				s[loc].setIsReady(true);
				if (s[loc].bursts[s[loc].getCurrentBurst()] == -1)
				{
					s[loc].setCurrentBurst(0);
					s[loc].burstsLeft = s[active].bursts;
				}
				else
				{
					s[loc].setCurrentBurst(s[loc].getCurrentBurst()+1);
				}

				io.pop();
			}
			if (io.size() > 0 && countdown > 0)
			{
				io.front().burstsLeft[io.front().getCurrentBurst()] -= countdown;
			}

			// Finish the process if it is at its end
			if (s[active].getCurrentBurst() == s[active].bursts.size()-1)
			{
				s[active].end(time);
			}
			// Otherwise the next burst is IO; push into IO queue
			else
			{
				s[active].setIsReady(false);
				if (s[active].bursts[s[active].getCurrentBurst()] == -1)
				{
					s[active].setCurrentBurst(0);
					s[active].burstsLeft = s[active].bursts;
				}
				else
				{
					s[active].setCurrentBurst(s[active].getCurrentBurst()+1);
				}
				io.push(s[active]);
			}
		}

		checkAllIoOrNotArrived(io, s, activeP, time, allDone);
		// If it is not over, context switch
		if (!allDone)
		{
			active = (active+1) % s.size();
			while (s[active].getArrivalTime() > time || s[active].isDone())
				active = (active+1) % s.size();

			if (s[active].getIsReady())
			{
				if (s[active].getId() != activeP)
				{
					s[active].start(time);
					numS++;
					activeP = s[active].getId();
				}
			}
		}
	}

	finish(time, numS, switches, s);
}

// Run a round robin schedule
void rr(Schedule processes, int q, int switches)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule s = sortProcessesByArrivalTime(time, processes);

	// Start the first process
	string activeP;
	if (s[0].getArrivalTime() > 0)
	{
		activeP = "IDLE";
		cout << time << ":IDLE ";
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
		// See if the current process is ready to run
		if (s[active].getIsReady() && s[active].getArrivalTime() <= time && !s[active].isDone())
		{
			// First see if you are at the end of a repeating sequence
			if (s[active].bursts[s[active].getCurrentBurst()] == -1)
			{
				s[active].setCurrentBurst(0);
			}
			
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
				s[loc].setCurrentBurst(s[loc].getCurrentBurst()+1);
				io.pop();
			}
			if (io.size() > 0 && timeChange > 0)
			{
				io.front().burstsLeft[io.front().getCurrentBurst()] -= timeChange;
			}

			// Handle processes that complete a burst
			if (s[active].burstsLeft[s[active].getCurrentBurst()] == 0)
			{
				// Finish the process if it is at its end
				if (s[active].getCurrentBurst() == s.size()-1)
				{
					s[active].end(time);
				}
				// Otherwise the next burst is IO; push into IO queue
				else
				{
					s[active].setIsReady(false);
					s[active].setCurrentBurst(s[active].getCurrentBurst()+1);
					io.push(s[active]);
				}
			}
		}

		checkAllIoOrNotArrived(io, s, activeP, time, allDone);
		// If it is not over, context switch
		if (!allDone)
		{
			active = (active+1) % s.size();
			while (s[active].getArrivalTime() > time || s[active].isDone())
				active = (active+1) % s.size();

			if (s[active].getIsReady())
			{
				if (s[active].getId() != activeP)
				{
					s[active].start(time);
					numS++;
					activeP = s[active].getId();
				}
			}
		}
	}

	finish(time, numS, switches, s);
}

// Run a shortest job first schedule
void sjf(Schedule processes, int switches)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule s = sortProcessesByArrivalTime(time, processes);

	// Fast forward time to first arrival
	string activeP;
	int active = 0;
	if (s[0].getArrivalTime() > 0)
	{
		activeP = "IDLE";
		cout << time << ":IDLE ";
	}
	time = s[0].getArrivalTime();

	// Start first process
	s = sortProcessesByRunTime(s);
	active = 0;
	while(!s[active].getIsReady() || s[active].getArrivalTime() > time || s[active].isDone())
	{
		active++;
	}
	s[active].start(time);
	activeP = s[active].getId();

	queue<Process> io;

	int numS = 0;
	bool allDone = false;
	int t = 0;
	// Run the processes in sorted order until the last process is finished
	while (!allDone && numS < switches)
	{			
		if (!s[active].isDone())
		{
			// First check to see if the process is at the -1 to trigger repeat
			if (s[active].bursts[s[active].getCurrentBurst()] == -1)
			{
				s[active].setCurrentBurst(0);
				s[active].burstsLeft = s[active].bursts;
			}

			// Let this process run for one length of time
			// Only one because if a shorter process becomes ready, it will run
			time++;
			s[active].burstsLeft[s[active].getCurrentBurst()] -= 1;

			// And as time passes, update those processes waiting in the IO queue
			int countdown = 1;
			if (io.size() > 0)
			{
				io.front().burstsLeft[io.front().getCurrentBurst()] -= countdown;
				if (io.front().burstsLeft[io.front().getCurrentBurst()] == 0)
				{
					string pid = io.front().getId();
					int loc = getProcessLocWithId(pid, s);
					s[loc].setIsReady(true);
					s[loc].setCurrentBurst(s[loc].getCurrentBurst()+1);
					io.pop();
				}
			}

			// Check if the process ended after time length of 1
			if (s[active].burstsLeft[s[active].getCurrentBurst()] == 0)
			{
				// Finish the process if it is at its end
				if (s[active].getCurrentBurst() == s[active].bursts.size()-1)
				{
					s[active].end(time);
				}
				// Otherwise the next burst is IO; push into IO queue
				else
				{
					s[active].setIsReady(false);
					s[active].setCurrentBurst(s[active].getCurrentBurst()+1);
					if (s[active].bursts[s[active].getCurrentBurst()] == -1)
					{
						s[active].setCurrentBurst(0);
						s[active].burstsLeft = s[active].bursts;
					}
					io.push(s[active]);
				}
			}
		}

		checkAllIoOrNotArrived(io, s, activeP, time, allDone);

		// If it is not over, context switch
		if (!allDone)
		{
			s = sortProcessesByRunTime(s);
			bool allNotReadyOrDone = true;
			for (unsigned int i = 0; i < s.size(); i++)
			{
				if (s[i].getIsReady() && !s[i].isDone())
					allNotReadyOrDone = false;
			}
			
			// Block like a son of a bitch if things are stuck
			active = (active+1) % s.size();
			while (allNotReadyOrDone)
			{
				stuck(io, s, activeP, time);
				for (unsigned int i = 0; i < s.size(); i++)
				{
					if (s[i].getIsReady() && !s[i].isDone())
						allNotReadyOrDone = false;
				}
			}

			// Get next process to run
			while (s[active].isDone() || !s[active].getIsReady() || s[active].getArrivalTime() > time)
				active = (active+1) % s.size();


			if (s[active].getArrivalTime() <= time && s[active].getId() != activeP)
			{
				s[active].start(time);
				numS++;
				activeP = s[active].getId();
			}
		}
	}

	finish(time, numS, switches, s);
}

