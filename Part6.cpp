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
// 5. I use a fixed duration of 10 units of time before aging
// 6. Processes blocking at higher IO priorities will always block first. Not FCFS at different priorities

// Run a multi-level feedback queue
void mlfq(Schedule process, int switches);
// Helper function to update master schedules
void updateMasters(vector<Schedule>& masterIo, Schedule io1, Schedule io2, Schedule io3, 
				   vector<Schedule>& masterSchedule, Schedule s1, Schedule s2, Schedule s3);
// Helper function to get current schedule to use
Schedule getSchedule(vector<Schedule> master, int& current, bool& none, int time);
// Helper function that returns quantum length
int getQuantum(Process p);
// Helper function to check if schedule is stumped
void checkAllIoOrNotArrived(Schedule& io, Schedule& s, string& activeP, int& time, bool& allDone);
// Helper function when nothing can run
void stuck(Schedule& io, Schedule& s, string& activeP, int& time);

int main()
{
	ifstream in;
	in.open("processes3.txt");

	// Prepare a vector of processes
	Schedule processes;
	readProcessesFromFile(in, processes);

	int switches;
	cout << "Enter the max number of context switches: "; cin >> switches;
//	switches = 20;

	// Simulate multi-level feedback queue
	cout << endl << endl;
	cout << "//////////////////////// MLFQ ////////////////////////" << endl;
	mlfq(processes, switches);

	return 0;
}

// Run a round robin schedule
void mlfq(Schedule processes, int switches)
{
	int time = 0;

	// Vectors of vectors to hold different priority levels
	vector<Schedule> masterIo;
	Schedule io1 = Schedule();
	Schedule io2;
	Schedule io3;
	Schedule io = io1;
	vector<Schedule> masterSchedule;
	Schedule s1 = sortProcessesByArrivalTime(time, processes);
	Schedule s2;
	Schedule s3;
	Schedule s = s1;

	// All entering processes get highest priority
	for (unsigned int i = 0; i < s.size(); i++)
	{
		s[i].setPriority(1);
	}

	// Start the first process
	string activeP = "";
	if (s[0].getArrivalTime() > 0)
	{
		activeP = "IDLE";
		cout << time << ":IDLE ";
	}
	time = s[0].getArrivalTime();

	updateMasters(masterIo, io1, io2, io3, masterSchedule, s1, s2, s3);

	int q;
	int active = 0;
	int numS = 0;
	bool allDone = false;
	int currentSchedule = 0;
	int currentIo = 0;
	int interval = 10;
	// Run the processes in sorted order until the last process is finished
	while (!allDone && numS < switches)
	{
		// Aging
		if (time % interval == 0)
		{
			for (unsigned int i = 0; i < masterSchedule.size(); i++)
			{
				for (unsigned int j = 0; j < masterSchedule[i].size(); j++)
				{
					masterSchedule[i][j].setPriority(0);
				}
			}
		}

		bool empty, ioempty;
		s = getSchedule(masterSchedule, currentSchedule, empty, time);

		// Find out which process should run
		if (!empty)
		{
			active = (active+1) % masterSchedule[currentSchedule].size();
			while (masterSchedule[currentSchedule][active].getArrivalTime() > time || s[active].isDone())
				active = (active+1) % masterSchedule[currentSchedule].size();

			if (masterSchedule[currentSchedule][active].getId() != activeP)
			{
				masterSchedule[currentSchedule][active].start(time);
				string pid = masterSchedule[currentSchedule][active].getId();
				int loc = getProcessLocWithId(pid, s1);
				if (s1[loc].getResponse() == -1)
				{
					s1[loc].setStartTime(time);
					s1[loc].setResponse(time - s1[loc].getArrivalTime());
				}
				numS++;
				activeP = s[active].getId();
			}
		}
		// Find out if there is more left in the IO queues
		io = getSchedule(masterIo, currentIo, ioempty, time);
		if (currentIo == masterIo.size())
			currentIo = currentSchedule;

		if (!empty)
		{
			// First see if you are at the end of a repeating sequence
			if (masterSchedule[currentSchedule][active].bursts[masterSchedule[currentSchedule][active].getCurrentBurst()] == -1)
			{
				masterSchedule[currentSchedule][active].setCurrentBurst(0);
			}

			// Using quantum of current queue, see if process finishes within quantum
			q = getQuantum(masterSchedule[currentSchedule][active]);
			int timeChange;

			// Processes that finish within quanta or right at end get to stay at the same priority
			if (masterSchedule[currentSchedule][active].burstsLeft[masterSchedule[currentSchedule][active].getCurrentBurst()] <= q)
			{
				timeChange = masterSchedule[currentSchedule][active].burstsLeft[masterSchedule[currentSchedule][active].getCurrentBurst()];
				masterSchedule[currentSchedule][active].burstsLeft[masterSchedule[currentSchedule][active].getCurrentBurst()] -= timeChange;
				time += timeChange;

				// Process can be ended if it is at its last burst
				if (masterSchedule[currentSchedule][active].getCurrentBurst() == masterSchedule[currentSchedule][active].bursts.size()-1)
				{
					masterSchedule[currentSchedule][active].end(time);
					string pid = masterSchedule[currentSchedule].front().getId();
					int loc = getProcessLocWithId(pid, s1);
					s1[loc].end(time);
					masterSchedule[currentSchedule].erase(masterSchedule[currentSchedule].begin() + active);
				}
				// Processes that aren't ending get moved to IO then
				else
				{
					masterSchedule[currentSchedule][active].setCurrentBurst(masterSchedule[currentSchedule][active].getCurrentBurst()+1);
					masterIo[currentSchedule].push_back(masterSchedule[currentSchedule][active]);
					masterSchedule[currentSchedule].erase(masterSchedule[currentSchedule].begin() + active);
				}
			}
			//Processes that don't complete a burst get moved down a priority
			else if (masterSchedule[currentSchedule][active].burstsLeft[masterSchedule[currentSchedule][active].getCurrentBurst()] > q)
			{
				timeChange = q;
				masterSchedule[currentSchedule][active].burstsLeft[masterSchedule[currentSchedule][active].getCurrentBurst()] -= timeChange;
				time += timeChange;

				// Priority 3 is already as low as it goes
				if (masterSchedule[currentSchedule][active].getPriority() != 3)
				{
					masterSchedule[currentSchedule][active].setPriority(masterSchedule[currentSchedule][active].getPriority()+1);
					masterSchedule[currentSchedule+1].push_back(masterSchedule[currentSchedule][active]);
					masterSchedule[currentSchedule].erase(masterSchedule[currentSchedule].begin() + active);
				}
				// If priority is 3, just move to back of the priority level
				else
				{
					masterSchedule[currentSchedule].push_back(masterSchedule[currentSchedule][active]);
					masterSchedule[currentSchedule].erase(masterSchedule[currentSchedule].begin() + active);
				}
			}

			// And as time passes, update those processes waiting in the IO queue
			while (!ioempty && masterIo[currentIo].front().burstsLeft[masterIo[currentIo].front().getCurrentBurst()] <= timeChange)
			{
				timeChange -= masterIo[currentIo].front().burstsLeft[masterIo[currentIo].front().getCurrentBurst()];
				string pid = masterIo[currentIo].front().getId();
				masterIo[currentIo].front().setCurrentBurst(masterIo[currentIo].front().getCurrentBurst()+1);
				masterSchedule[masterIo[currentIo].front().getPriority()].push_back(masterIo[currentIo].front());
				masterIo[currentIo].erase(masterIo[currentIo].begin());

				// Jump down to next priority if the current IO queue is empty
				if (masterIo[currentIo].size() == 0 && currentIo != masterIo.size()-1 && masterIo[currentIo+1].size() != 0)
					currentIo++;
				else if (masterIo[currentIo+1].size() == 0)
					ioempty = true;
			}
			if (!ioempty && masterIo[currentIo].size() > 0 && timeChange > 0)
			{
				masterIo[currentIo].front().burstsLeft[masterIo[currentIo].front().getCurrentBurst()] -= timeChange;
			}
		}
		else if (empty && !ioempty)
		{
			if (activeP != "IDLE")
			{
				activeP = "IDLE";
				cout << time << ":IDLE ";
			}
			time++;
			if (io.size() > 0)
			{
				masterIo[currentIo].front().burstsLeft[masterIo[currentIo].front().getCurrentBurst()] -= 1;
				if (masterIo[currentIo].front().burstsLeft[masterIo[currentIo].front().getCurrentBurst()] == 0)
				{
					masterIo[currentIo].front().setCurrentBurst(masterIo[currentIo].front().getCurrentBurst()+1);
					masterSchedule[masterIo[currentIo].front().getPriority()].push_back(masterIo[currentIo].front());
					masterIo[currentIo].erase(masterIo[currentIo].begin());
				}
			}
		}
		allDone = true;
		for (unsigned int i = 0; i < masterSchedule.size(); i++)
		{
			if (masterSchedule[i].size() != 0 || masterIo[i].size() != 0)
				allDone = false;
		}
	}

	// FINISHING CODE
	cout << time << ":END." << endl;

	bool allFiniteEnded = true;
	for (unsigned int i = 0; i < s1.size(); i++)
	{
		if (s1[i].getRunTime() != -1 && !s1[i].isDone())
			allFiniteEnded = false;
	}

	if (numS <= switches && allFiniteEnded)
	{
		calcAvgTurnaroundAndResponse(s1);
	}
	else
	{
		cout << "Max number of context switches was reached before all processes ended." << endl;
	}
		
}

// Helper function to update master schedules
void updateMasters(vector<Schedule>& masterIo, Schedule io1, Schedule io2, Schedule io3, 
				   vector<Schedule>& masterSchedule, Schedule s1, Schedule s2, Schedule s3)
{
	masterIo.clear();
	masterIo.push_back(io1);
	masterIo.push_back(io2);
	masterIo.push_back(io3);
	masterSchedule.clear();
	masterSchedule.push_back(s1);
	masterSchedule.push_back(s2);
	masterSchedule.push_back(s3);
}

// Helper function to get current schedule to use
Schedule getSchedule(vector<Schedule> master, int& current, bool& none, int time)
{
	Schedule s;
	current = 0;
	none = true;
	for (unsigned int i = 0; i < master.size(); i++)
	{
		if (master[i].size() > 0 )
		{
			for (unsigned int j = 0; j < master[i].size(); j++)
			{
				if (master[i][j].getArrivalTime() <= time)
				{
					none = false;
					s = master[i];
					break;
				}
			}
		}
		if (none)
			current++;
		else
			break;
	}

	return s;
}

// Helper function that returns quantum length
int getQuantum(Process p)
{
	int q  = 0;
	switch (p.getPriority())
	{
	default:
	case 1:
		q = 2;
		break;
	case 2:
		q = 4;
		break;
	case 3:
		q = 100;
		break;
	}
	return q;
}