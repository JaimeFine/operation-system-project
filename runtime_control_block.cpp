#include <iostream>
#include <string>
#include <queue>
#include <vector>

using namespace std;

enum class State {
    Created,
    ReadyActive,
    ReadySuspend,
    Running,
    BlockedActive,
    BlockedSuspend,
    Finished
};

// Process Contrl Block (PCB)
class Process {
private:
    int id;         // Internal label
    string name;    // External label
    int total_time;
    int executed_time;
    State state;

public:
    Process(int pid, const string& pname, int runtime)
        : id(pid),
          name(pname),
          total_time(runtime),
          executed_time(0),
          state(State::Ready) {}

    int getID() const { return id; }
    string getName() const { return name; }
    State getState() const { return state; }

    bool isFinished() const {
        return executed_time >= total_time;
    }

    void run(int quantum) {
        state = State::Running;

        int remain = total_time - executed_time;
        int slice = min(remain, quantum);

        executed_time += slice;

        cout << "[RUN] Process "
             << id << " (" << name << ") ran for "
             << slice << " ticks. "
             << executed_time << "/" << total_time
             << endl;

        if (isFinished()) {
            state = State::Finished;
        } else {
            state = State::Ready;
        }
    }

    void readya() {
        state = State::ReadyActive;
    }

    void readys() {
        state = State::ReadySuspend;
    }
    
    void print() const {
        cout << "PID=" << id
             << " Name=" << name
             << " Progress=" << executed_time
             << "/" << total_time
             << endl;
    }

    void active() {
        if (state == State::ReadySuspend) {
            state = State::ReadyActive;
        } else if (state == State::BlockedSuspend) {
            state = State::BlockedActive;
        } else {
            cout << "Error in activation!" << endl;
        }
    }
    void suspend() {
        if (state == State::ReadyActive) {
            state = State::ReadySuspend;
        } else if (state == State::BlockedActive) {
            state = State::BlockedSuspend;
        } else {
            cout << "Error in suspension!" << endl;
        }
    }
    void block() {
        if (state == State::ReadyActive) {
            state = State::BlockedActive;
        } else if (state == State::ReadySuspend) {
            state = State::BlockedSuspend;
        } else {
            cout << "Error in suspension!" << endl;
        }
    }
    void wakeup() {
        if (state == State::BlockedSuspend) {
            state = State::ReadySuspend;
        } else if (state == State::BlockedActive) {
            state = State::ReadyActive;
        } else {
            cout << "Error in wakeup!" << endl;
        }
    }

    ~Process() {};
};

// Runtime Scheduler
class RuntimeSystem {
private:
    queue<Process*> readyQueue;
    vector<Process*> blockedList;
    int quantum;
    int clock;

public:
    RuntimeSystem(int q)
        : quantum(q), clock(0) {}

    void addProcess(Process* p) {
        readyQueue.push(p);
        cout << "[ADD] Process "
             << p->getID()
             << " added to ready queue."
             << endl;
    }

    void blockCurrent(Process* p) {
        p->block();
        blockedList.push_back(p);

        cout << "[BLOCK] Process "
             << p->getID()
             << " moved to blocked list."
             << endl;
    }

    void wakeAllBlocked() {
        for (auto p : blockedList) {
            p->wakeup();
            readyQueue.push(p);

            cout << "[WAKEUP] Process "
                 << p->getID()
                 << " returned to ready queue."
                 << endl;
        }
        blockedList.clear();
    }

    void run() {
        cout << "\n===== Runtime Started =====\n";

        while (!readyQueue.empty()) {
            Process* current = readyQueue.front();
            readyQueue.pop();

            cout << "\n[CLOCK " << clock << "] Dispatching PID "
                 << current->getID()
                 << endl;

            current->run(quantum);
            clock += quantum;

            if (current->getState() == State::Finished) {
                cout << "[EXIT] Process "
                     << current->getID()
                     << " finished."
                     << endl;
            } else {
                readyQueue.push(current);
            }
        }
        cout << "\n===== Runtime Finished =====\n";
    }
};