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

// Helper function: turn a string of integers into a vector
vector<int> processBurstString(string bString);

// Fill a vector of processes from a file; close file
void readProcessesFromFile(ifstream& in, Schedule& processes);

// Run a first come first serve schedule with processes
void fcfs(Schedule process);

// Run a round robin schedule
void rr(Schedule process);

// Run a shortest job first schedule
void sjf(Schedule processes);

// Helper function to sort unfinished processes according to time until they arrive
// The processes have finished and have are >= 0 time units away from arriving
Schedule sortProcessesByArrivalTime(int time, Schedule processes);

// Helper function to compute and output averages
void calcAvgTurnaroundAndResponse(Schedule s);

int main()
{
	ifstream in;
	in.open("processes.txt");

	// Prepare a vector of processes
	Schedule processes;
	readProcessesFromFile(in, processes);
//	for (int i = 0; i < processes.size(); i++)
//		processes[i].printBursts(cout);

	cout << "//////////////////////// FCFS ////////////////////////" << endl;
	// Simulate first come, first served
	fcfs(processes);

	cout << "//////////////////////// RR ////////////////////////" << endl;
	// Simulate round robin
	rr(processes);

	return 0;
}

// Helper function: turn a string of integers from file into a vector
vector<int> processBurstString(string bString)
{
	vector<int> bVec;
	std::stringstream stream(bString);

	int number;
	while (stream >> number)
		bVec.push_back(number);

	return bVec;
}

// Fill a vector of processes from a file
void readProcessesFromFile(ifstream& in, Schedule& processes)
{
	string id;
	int arrival;
	vector<int> bursts;

	// Read the file line by line
	while (!in.eof())
	{
		in >> id >> arrival;

		// Read the bursts number by number
		string burstString = "";
		char z;
		in.get(z);
		while (z != '\n' && !in.eof())
		{
			burstString += z;
			in.get(z);
		}
		bursts = processBurstString(burstString);
		Process p = Process(id, arrival, bursts);
		processes.push_back(p);
	}
	
	in.close();
}

// Run a first come first serve schedule with processes
void fcfs(Schedule processes)
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
void rr(Schedule processes)
{
	int time = 0;
	int quanta;
	cout << "Enter the length of time slice quanta: "; cin >> quanta;

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
		cout << tempTime << " " << quanta << " " << tempTime%quanta << endl;
		if (((tempTime == 0) || ((tempTime % quanta) > 0)) && (sortedProcesses[active].bursts[sortedProcesses[active].currentBurst] > 0))
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
			for (int i = 0; i < processes.size(); i++)
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
		for (int i = 0; i < processes.size(); i++)
		{
			if (!sortedProcesses[i].isDone())
				allDone = false;
		}
	}

	cout << "TIME " << time << ": END." << endl;

	calcAvgTurnaroundAndResponse(sortedProcesses);
}

// Run a shortest job first schedule
void sjf(Schedule processes)
{
	int time = 0;

}

// Helper function to sort unfinished processes according to time until they arrive
// The processes have not arrived yet
Schedule sortProcessesByArrivalTime(int time, Schedule processes)
{
	vector<Process> sortedByTimes;
	sortedByTimes.push_back(processes[0]);

	// First find the earliest arrival time for any process
	for (int i = 1; i < processes.size(); i++)
	{
		int idx = 0;
		bool stop = false;
		if (processes[i].timeUntilArrival(time) >= 0)
		{
			while (idx < sortedByTimes.size() && stop != true)
			{
				if (processes[i].timeUntilArrival(time) >= sortedByTimes[idx].timeUntilArrival(time))
				{
					idx++;
				}
				else
				{
					stop = true;
				}
			}
			sortedByTimes.insert(sortedByTimes.begin()+idx, processes[i]);
		}
	}

	return sortedByTimes;
}

// Helper function to compute and output averages
void calcAvgTurnaroundAndResponse(Schedule s)
{
	float avgTurnaround = 0;
	float avgResponseTime = 0;
	for (int i = 0; i < s.size(); i++)
	{
		// Calculate average turnaround and response time
		avgTurnaround += s[i].getTurnaround();
		avgResponseTime += s[i].getResponse();
	}
	avgTurnaround /= s.size();
	avgResponseTime /= s.size();

	cout << "Average turnaround time was: " << avgTurnaround << endl;
	cout << "Average response time was: " << avgResponseTime << endl;
}