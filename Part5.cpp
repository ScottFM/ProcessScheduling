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
// 1. Processes all arrive at time t=0
// 2. Processes can have alternating CPU and IO bursts
// 3. There is a max number of context switches.
// 4. There will never be a CPU sequence of only -1.

// Simulate shorted remaining time first with a lottery
void lottery(Schedule processes, int switches);

int main()
{
	ifstream in;
	in.open("processes4.txt");

	// Prepare a vector of processes
	Schedule processes;
	readProcessesFromFile(in, processes);

	int switches;
	cout << "Enter the max number of context switches: "; cin >> switches;
//	switches = 20;
	srand((unsigned) time(NULL));

	cout << endl << endl;
	cout << "//////////////////////// LOTTERY ////////////////////////" << endl;
	// Simulate shortest remaining time first with a lottery
	lottery(processes, switches);

	return 0;
}

// Simulate shorted remaining time first with a lottery
void lottery(Schedule processes, int switches)
{
	int time = 0;
	int est = 3;
	float alpha = 0.5;
	int totalTix = 0;

	// Sort the processes by estimate
	// Don't need to sort by arrival time due to assumption 1
	Schedule s = sortProcessesByAvg(processes);

	// Set initial estimate for each process
	for (unsigned int i = 0; i < s.size(); i++)
	{
		s[i].setBurstAvg(est);
	}

	string activeP;
	int active = 0;
	int numS = 0;
	bool allDone = false;
	int draw = 0;

	// Run the processes in sorted order until the last process is finished
	while (!allDone && numS < switches)
	{
		// Redistribute the tickets
		s = sortProcessesByAvg(s);
		int lowestEst = s[0].getBurstAvg();
		int highestEst = s[s.size()-1].getBurstAvg();
		totalTix = 0;
		for (unsigned int i = 0; i < s.size(); i++)
		{
			int avg = s[i].getBurstAvg();
			int num;

			// Highest priority
			if(avg == lowestEst)
				num = 10;
			// Lowest priority
			else if(avg == highestEst)
				num = 1;
			// Intermediate priority
			else
				num = 5;

			s[i].setTickets(num);
			totalTix += num;
		}

		// Pull a winner from the tickets
		draw = (rand() % totalTix) + 1;
		int currentTotal = 0;
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if(!s[i].isDone())
			{
				if(draw > currentTotal && draw <= currentTotal + s[i].getTickets())
				{
//					cout << "Process " << s[i].getId() << " was drawn with a chance of " << s[i].getTickets() << "/" << totalTix << endl;
					active = i;
					break;
				}
				currentTotal += s[i].getTickets();
			}
		}

		if(s[active].getId() != activeP)
		{
			s[active].start(time);
			activeP = s[active].getId();
			numS++;
		}

		activeP = s[active].getId();

		// Let process run
		time += s[active].bursts[s[active].getCurrentBurst()];

		// Calculate the new estimate
		float nextEst = (alpha*s[active].bursts[s[active].getCurrentBurst()]) + ((1-alpha)*s[active].getBurstAvg());
		s[active].setBurstAvg(ceil(nextEst));
//		cout << "New estimate for " << s[active].getId() << " = " << ceil(nextEst) << endl;
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

		allDone = true;
		// See if the schedule is done now
		for (unsigned int i = 0; i < s.size(); i++)
		{
			if (!s[i].isDone())
				allDone = false;
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