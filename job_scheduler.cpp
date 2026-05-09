#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <queue>

using namespace std;
const int MAXJOB = 50;

typedef struct node
{
	int id;         	// Job Index
	int arrival_time;   // Arrival Time
	int total_time;     // Total Time
	int priority;	  	// Priority
	float excellent;    //响应比
	int start_time; 	// Start Time
	int wait_time;      // Waiting Time
	int tr_time;        // Turnaround Time
	double wtr_time;   	// Weighted Turnaround Time
	int exec_time;      // Executed Time
	int visited;
	bool arrived;
} job;

job jobs[MAXJOB];
int job_number;

void read_file() {
	string file, text;
	cout << "Enter file name: ";
	cin >> file;
	fstream fin(file);
	if (!fin.is_open()) {
		cout << "[Failed] Failed to open the file!!!\n";
		exit(0);
	} else {
		getline(fin, text);
		while (fin.good()) {
			fin >> jobs[job_number].id >> jobs[job_number].arrival_time
				>> jobs[job_number].total_time >> jobs[job_number].priority;
			job_number++;
		}
		fin.close();
	}

	cout << "Job number: " << job_number << endl;
	cout << "Job ID \t Arrival time \t Total Execution time \t Priority" << endl;
	for (int i = 0; i < job_number; i++) {
		cout << jobs[i].id << "\t" << jobs[i].arrival_time << "\t"
			<< jobs[i].total_time << "\t" << jobs[i].priority << endl;
	}
	cout << "[Success] File read!!!";
}

void init() {
	for (int i = 0; i < job_number; i++) {
		jobs[i].start_time = 0;
		jobs[i].wait_time = 0;
		jobs[i].tr_time = 0;
		jobs[i].wtr_time = 0.0;
		jobs[i].exec_time = 0;
		jobs[i].visited = 0;
		jobs[i].arrived = false;
	}

	cout << "[Success] All jobs initialized!!!\n";
}