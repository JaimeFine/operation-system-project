#include <string>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <queue>
#include <algorithm>
#include <vector>

using namespace std;
const int MAXJOB = 50;

typedef struct node
{
	int id;         		// Job Index
	int arrival_time;   	// Arrival Time
	int total_time;     	// Total Time
	int priority;	  		// Priority
	float response_ratio;	// Response Ratio
	int start_time; 		// Start Time
	int wait_time;      	// Waiting Time
	int tr_time;        	// Turnaround Time
	double wtr_time;   		// Weighted Turnaround Time
	int exec_time;      	// Executed Time
	int visited;
	bool arrived;
} job;

job jobs[MAXJOB];
int job_number = 0;

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
		while (fin >> jobs[job_number].id
				>> jobs[job_number].arrival_time
				>> jobs[job_number].total_time
				>> jobs[job_number].priority) {
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

vector<int> get_arrived_jobs(job job[], int count, int now) {
	vector<int> available_idx;
	for (int i = 0; i < count; i++) {
		if (jobs[i].arrival_time <= now && jobs[i].visited == 0) {
            available_idx.push_back(i);
        }
	}
	return available_idx;
}

int find_shortest_job(job jobs[], int count, int now) {
	vector<int> available = get_arrived_jobs(jobs, count, now);

	if (available.empty()) {
		return -1;
	}

	int shortest_idx = available[0];
	int min_time = jobs[shortest_idx].total_time;

	for (size_t i = 1; i < available.size(); i++) {
		int idx = available[i];
		if (jobs[idx].total_time < min_time) {
			min_time = jobs[idx].total_time;
			shortest_idx = idx;
		}
	}
	return shortest_idx;
}

void SJF(job jobs[], int count) {
	int now = 0;
	int completed = 0;
	double sumwait = 0, sumtr = 0, sumwtr = 0;

	cout << "\n--- This is Shortest Job First (SFJ) Scheduling ---\n";

	while (completed < count) {
		int idx = find_shortest_job(jobs, count, now);
		
		if (idx == -1) {
			now++;
			continue;
		}

		job& current_job = jobs[idx];

		current_job.start_time = now;
		current_job.wait_time = now - current_job.arrival_time;

		now += current_job.total_time;
		current_job.exec_time = current_job.total_time;
		current_job.visited = 1;
		completed++;

		current_job.tr_time = now - current_job.arrival_time;
        current_job.wtr_time = (double)current_job.tr_time / current_job.total_time;

		cout << "Job " << current_job.id << " started at " << current_job.start_time 
             << ", finished at " << now 
             << " | Wait: " << current_job.wait_time 
             << " | TR: " << current_job.tr_time 
             << " | WTR: " << fixed << setprecision(2) << current_job.wtr_time << "\n";

		sumwait += current_job.wait_time;
		sumtr += current_job.tr_time;
		sumwtr += current_job.wtr_time;
	}
	cout << "\nAverage Waiting Time: " << sumwait / count;
    cout << "\nAverage Turnaround Time: " << sumtr / count;
    cout << "\nAverage Weighted Turnaround Time: " << sumwtr / count << "\n";
}

void compute_response_ratio(job& jobs, int now) {
	jobs.wait_time = now - jobs.arrival_time;
	jobs.response_ratio = 1.0 + (double)jobs.wait_time / jobs.total_time;
}

int find_highest_response_ratio(job jobs[], int count, int now) {
	vector<int> available = get_arrived_jobs(jobs, count, now);

	if (available.empty()) {
		return -1;
	}

	for (auto i : available) {
		compute_response_ratio(jobs[i], now);
	}

	int best_idx = available[0];
	double max_ratio = jobs[best_idx].response_ratio;

	for (size_t i = 1; i < available.size(); i++) {
		int idx = available[i];
		if (jobs[idx].response_ratio > max_ratio) {
			max_ratio = jobs[idx].response_ratio;
			best_idx = idx;
		}
	}
	return best_idx;
}

void HRRF(job jobs[], int count) {
	int now = 0;
	int completed = 0;
	double sumwait = 0, sumtr = 0, sumwtr = 0;

	cout << "\n--- This is Highest Response Ratio First (HRRF) Scheduling ---\n";

	while (completed < count) {
		int idx = find_highest_response_ratio(jobs, count, now);
		
		if (idx == -1) {
			now++;
			continue;
		}

		job& current_job = jobs[idx];

		current_job.start_time = now;
		current_job.wait_time = now - current_job.arrival_time;

		now += current_job.total_time;
		current_job.exec_time = current_job.total_time;
		current_job.visited = 1;
		completed++;

		current_job.tr_time = now - current_job.arrival_time;
        current_job.wtr_time = (double)current_job.tr_time / current_job.total_time;

		cout << "Job " << current_job.id << " started at " << current_job.start_time 
             << ", finished at " << now 
             << " | Wait: " << current_job.wait_time 
             << " | TR: " << current_job.tr_time 
             << " | WTR: " << fixed << setprecision(2) << current_job.wtr_time << "\n";

		sumwait += current_job.wait_time;
		sumtr += current_job.tr_time;
		sumwtr += current_job.wtr_time;
	}
	cout << "\nAverage Waiting Time: " << sumwait / count;
    cout << "\nAverage Turnaround Time: " << sumtr / count;
    cout << "\nAverage Weighted Turnaround Time: " << sumwtr / count << "\n";
}

int find_highest_priority(job jobs[], int count, int now) {
    vector<int> available = get_arrived_jobs(jobs, count, now);

    if (available.empty()) {
        return -1;
    }

    int best_idx = available[0];
    int highest_priority = jobs[best_idx].priority;

    for (size_t i = 1; i < available.size(); i++) {
        int idx = available[i];

        if (jobs[idx].priority > highest_priority) {
            highest_priority = jobs[idx].priority;
            best_idx = idx;
        }
    }

    return best_idx;
}

void HPF(job jobs[], int count) {
    int now = 0;
    int completed = 0;
    double sumwait = 0, sumtr = 0, sumwtr = 0;

    cout << "\n--- This is Highest Priority First (HPF) Scheduling ---\n";

    while (completed < count) {
        int idx = find_highest_priority(jobs, count, now);

        if (idx == -1) {
            now++;
            continue;
        }

        job& current_job = jobs[idx];

        current_job.start_time = now;
        current_job.wait_time = now - current_job.arrival_time;

        now += current_job.total_time;
        current_job.exec_time = current_job.total_time;
        current_job.visited = 1;
        completed++;

        current_job.tr_time = now - current_job.arrival_time;
        current_job.wtr_time =
            (double)current_job.tr_time / current_job.total_time;

        cout << "Job " << current_job.id
             << " started at " << current_job.start_time
             << ", finished at " << now
             << " | Wait: " << current_job.wait_time
             << " | TR: " << current_job.tr_time
             << " | WTR: " << fixed << setprecision(2)
             << current_job.wtr_time << "\n";

        sumwait += current_job.wait_time;
        sumtr += current_job.tr_time;
        sumwtr += current_job.wtr_time;
    }

    cout << "\nAverage Waiting Time: " << sumwait / count;
    cout << "\nAverage Turnaround Time: " << sumtr / count;
    cout << "\nAverage Weighted Turnaround Time: " << sumwtr / count << "\n";
}

int main() {
    int choice;
    
    read_file();

    while (true) {
        cout << "\n================================";
        cout << "\n   Job Scheduling Simulator";
        cout << "\n================================";
        cout << "\n1. Shortest Job First (SJF)";
        cout << "\n2. Highest Response Ratio (HRRF)";
        cout << "\n3. Highest Priority First (HPF)";
        cout << "\n0. Exit";
        cout << "\nSelect an algorithm: ";
        cin >> choice;

        if (choice == 0) break;

        init();

        switch (choice) {
            case 1:
                SJF(jobs, job_number);
                break;
            case 2:
                HRRF(jobs, job_number);
                break;
            case 3:
                HPF(jobs, job_number);
                break;
            default:
                cout << "Invalid choice. Try again.\n";
        }
    }

    cout << "Exiting simulator. Goodbye!\n";
    return 0;
}