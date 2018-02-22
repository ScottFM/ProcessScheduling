#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>	// for finding element in vector
#include <iomanip>
#include "math.h"		// for ceil
#include <random>
#include <time.h>
#include <stdlib.h>

// Process class
#include "Process.h"

using namespace std;

typedef vector<Process> Schedule;

// This file assumes:
// 1. Processes can have alternating CPU and IO bursts
// 2. There is a max number of context switches.
// 3. A CPU burst will come first and last. An IO burst will never start or finish the sequence
// 4. There will never be a CPU sequence of only -1.

// Simulate shorted remaining time first with a lottery
void lottery(Schedule processes, int switches, float alpha, float est);
// Helper function to get the active process after drawing
int getActive(int draw, Schedule s);

int main()
{
	ifstream in;
	in.open("processes3.txt");

	// Prepare a vector of processes
	Schedule processes;
	readProcessesFromFile(in, processes);

	int switches;
	float alpha = 0;
	float est = 0;
	cout << "Enter the max number of context switches: "; cin >> switches;
    cout << "Enter a value for alpha between 0 and 1: "; cin >> alpha;
	cout << "Enter a guess for initial process run time: "; cin >> est;
	//switches = 30;
	//alpha = 0.5;
	//est = 5;
	srand((unsigned) time(NULL));

	for (int i = 0; i < 100; i++)
	{
		cout << endl << endl;
		cout << "//////////////////////// LOTTERY ////////////////////////" << endl;
		// Simulate shortest remaining time first with a lottery
		lottery(processes, switches, alpha, est);
	}

	return 0;
}

// Simulate shorted remaining time first with a lottery
void lottery(Schedule processes, int switches, float alpha, float est)
{
	int time = 0;

	// Sort the processes by time until they arrive
	Schedule s = sortProcessesByArrivalTime(time, processes);

	// Fast forward time to first arrival
	string activeP;
	if (s[0].getArrivalTime() > 0)
	{
		activeP = "IDLE";
		cout << time << ":IDLE ";
	}
	time = s[0].getArrivalTime();

	// Set initial estimate for each process
	int totalTix = 0;
	for (unsigned int i = 0; i < s.size(); i++)
	{
		s[i].setBurstAvg(est);
		totalTix += s[i].getTickets();
	}
	// Do the first random draw
	int draw = (rand() % totalTix) + 1;
	int active = 0;
	int currentTotal = 0;
	active = getActive(draw, s);

//	cout << "initial draw = " << draw << endl;

	// Start first process
	while(!s[active].getIsReady() || s[active].getArrivalTime() > time || s[active].isDone())
	{
		draw = (draw+1) % s.size();
		active = getActive(draw, s);
	}
	s[active].start(time);
	activeP = s[active].getId();

	queue<Process> io;
	int numS = 0;
	bool allDone = false;

	// Run the processes according to lottery
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
				// Calculate the new estimate
				float a = alpha*(float)s[active].bursts[s[active].getCurrentBurst()];
				float b = (1-alpha)*(float)s[active].getBurstAvg();
				float nextEst = (a + b);
				s[active].setBurstAvg(ceil(nextEst));
//				cout << "New estimate for " << s[active].getId() << " = " << s[active].getBurstAvg() << endl;

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
			s = sortProcessesByAvg(s);

			// Calculate new ticket distribution
			int smallestAvg = 0;
			int largestAvg = 100;
			for (unsigned int i = 0; i < s.size(); i++)
			{
				if (!s[i].isDone())
				{
					smallestAvg = s[i].getBurstAvg();
					break;
				}
			}
			for (unsigned int i = s.size()-1; i >= 0; i--)
			{
				if (!s[i].isDone())
				{
					largestAvg = s[i].getBurstAvg();
					break;
				}
			}
			totalTix = 0;
			for (unsigned int i = 0; i < s.size(); i++)
			{
				if (!s[i].isDone())
				{
					int avg = s[i].getBurstAvg();
					int num;
					if (s[i].getBurstAvg() == smallestAvg)
					{
						num = 10;
					}
					else if (s[i].getBurstAvg() == largestAvg)
					{
						num = 4;
					}
					else
					{
						num = 7;
					}
					s[i].setTickets(num);
					totalTix += num;
				}
			}

			bool allNotReadyOrDone = true;
			for (unsigned int i = 0; i < s.size(); i++)
			{
				if (s[i].getIsReady() && !s[i].isDone() && s[i].getArrivalTime() <= time)
					allNotReadyOrDone = false;
			}
			
			// Block like a son of a bitch if things are stuck
			draw = (rand() % totalTix) + 1;
			active = getActive(draw, s);
			while (s[active].isDone())
			{
				draw = (rand() % totalTix) + 1;
				active = getActive(draw, s);
			}
			while (allNotReadyOrDone)
			{
				stuck(io, s, activeP, time);
				for (unsigned int i = 0; i < s.size(); i++)
				{
					if (s[i].getIsReady() && !s[i].isDone() && s[i].getArrivalTime() <= time)
						allNotReadyOrDone = false;
				}
			}

			// Let process start or resume if it can
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

int getActive(int draw, Schedule s)
{
	int currentTotal = 0;
	int active = 0;
	for (unsigned int i = 0; i < s.size(); i++)
	{
		if (!s[i].isDone())
		{
			if(draw > currentTotal && draw <= currentTotal + s[i].getTickets())
			{
				active = i;
				break;
			}
			else
			{
				currentTotal += s[i].getTickets();
			}
		}
	}
	return active;
}