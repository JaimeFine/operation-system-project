#include <iostream>
#include <vector>
#include <queue>
#include <memory>
#include <conio.h>
#include <string>

using namespace std;

enum class ExecState {
    Ready,
    Running,
    Blocked,
    Finished
};

class Process {
public:
    int id;
    string name;
    int total;
    int executed_time;
    ExecState state;
    int pc;

    Process(int pid, string pname, int runtime)
        : id(pid), name(pname), total_time(runtime),
          executed_time(0), state(ExecState::Ready), pc(0) {}

    void saveContext() {
        cout << "[SAVE] Saving PC=" << pc << " for PID " << id << endl;
    }
    void restoreContext() {
        cout << "[RESTORE] Restoring PC=" << pc << " for PID " << id << endl;
    }
    void printPCB() const {
        string states[] = {"Ready", "Running", "Blocked", "Finished"};
        cout << "[PCB] ID:" << id << " Name:" << name 
            << " Time:" << executed_time << "/" << total 
            << " State:" << states[(int)state] << " PC:" << pc << endl;
    }
};

class RuntimeSystem {
private:
    queue<Process*> readyQueue;
    vector<Process*> blockedQueue;
    Process* runningProcess = nullptr;
    int quantum;

public:
    RuntimeSystem(int q) : quantum(q) {}

    void addProcess(Process* p) {
        readyQueue.push(p);
    }

    void schedule() {
        while (!readyQueue.empty() || !blockedQueue.empty()) {
            if (readyQueue.empty()) {
                cout << "\n[NOTE] No ready processes. "
                     << "Press 'W' to wakeup blocked processes..." << endl;
                if (toupper(getch()) == 'W')
                    wakeupAll();
                continue;
            }

            runningProcess = readyQueue.front()     // FCFS algorithm
            readyQueue.pop();
            runningProcess->state = ExecState::Running;

            runningProcess->restoreContext()

            cout << "\n[RUNNING] PID " << runningProcess->id
                 << " (" << runningProcess->name << ")" << endl;
            cout << "Press 'B' to Block (Esc simulation), 'Enter' to Finish "
                 << "slice, 'C' to Kill\n";

            char input = toupper(getch());

            if (input == 'B') {
                runningProcess->state = ExecState::Blocked;
                runningProcess->saveContext();
                blockedQueue.push_back(runningProcess);
                cout << "[EVENT] Process Blocked.\n"
            } else {
                int slice = min(
                    quantum,
                    runningProcess->total_time - runningProcess->executed_time
                );
                runningProcess->executed_time += slice;
                runningProcess->pc += slice;

                if (runningProcess->executed_time >= runningProcess->total_time) {
                    runningProcess->state = ExecState::Finished;
                    cout << "[EXIT] Process Finished." << endl;
                } else {
                    runningProcess->state = ExecState::Ready;
                    runningProcess->saveContext();
                    readyQueue.push(runningProcess);
                    cout << "[TIMEOUT] Slice finished, back to Ready Queue.\n";
                }
            }
            runningProcess = nullptr;
        }
    }

    void wakeupAll() {
        for (auto p : blockedQueue) {
            p->state = ExecState::Ready;
            readyQueue.push(p);
        }
        blockedQueue.clear();
        cout << "[EVENT] All processes awakened.\n";
    }
};

int main() {
    int q, n;
    cout << "Quantum: "; cin >> q;
    cout << "Process Count: "; cin >> n;

    RuntimeSystem os(q);
    vector<unique_ptr<Process>> pcbs;

    for (int i = 0; i < n; ++i) {
        int pid, runtime;
        string name;
        cout << "Input for P" << i+1 << ": ";
        cin >> pid >> name >> runtime;

        pcbs.push_back(make_unique<Process>(pid, name, runtime));
        os.addProcess(pcbs.back().get());
    }

    getch();
    os.schedule();

    return 0;
}

/*
Command Notes:
> B is to block
> Enter is to finish slice
> W is to wakeup
*/