#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>	// for finding element in vector
#include <iomanip>
#include "math.h"		// for ceil

// Process class
#include "Process.h"

using namespace std;

typedef vector<Process> Schedule;

// This file assumes:
// 1. Processes all arrive at time t=0
// 2. Processes can have alternating CPU and IO bursts
// 3. There is a max number of context switches.
// 4. There will never be a CPU sequence of only -1.

// Simulate shorted remaining time first
void srtf(Schedule processes, int switches);

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

	cout << endl << endl;
	cout << "//////////////////////// SRTF ////////////////////////" << endl;
	// Simulate shortest job first with calculated average
	srtf(processes, switches);

	return 0;
}

// Simulate shorted remaining time first
void srtf(Schedule processes, int switches)
{
	int time = 0;
	int est = 5;
	float alpha = 0.5;

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

	// Run the processes in sorted order until the last process is finished
	while (!allDone && numS < switches)
	{
		// Always start the process with the lowest average
		s = sortProcessesByAvg(s);
		active = 0;
		while(s[active].isDone())
			active++;
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
		cout << "New estimate for " << s[active].getId() << " = " << ceil(nextEst) << endl;
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