#include <iostream>
#include <string>
#include <queue>
#include <vector>

using namespace std;

enum class ExecState {
    Ready,
    Running,
    Blocked,
    Finished
};

enum class Residency {
    Active,
    Suspended
};

// Process Contrl Block (PCB)
class Process {
private:
    int id;         // Internal label
    string name;    // External label
    int total_time;
    int executed_time;

    ExecState exec;
    Residency mem;

public:
    Process(int pid, const string& pname, int runtime)
        : id(pid),
          name(pname),
          total_time(runtime),
          executed_time(0),
          exec(ExecState::Ready),
          mem(Residency::Active) {}

    int getID() const { return id; }
    string getName() const { return name; }
    ExecState getExecState() const { return exec; }
    Residency getResidency() const { return mem; }

    bool isRunnable() const {
        return exec == ExecState::Ready && mem == Residency::Active;
    }

    bool isFinished() const {
        return exec == ExecState::Finished;
    }

    void run(int quantum) {
        if (!isRunnable()) {
            cout << "[ERROR] PID "
                 << id
                 << " not runnable.\n";
            return;
        }

        exec = ExecState::Running;

        int remain = total_time - executed_time;
        int slice = min(remain, quantum);

        executed_time += slice;

        cout << "[RUN] PID "
             << id
             << " (" << name << ") executed "
             << slice << " ticks. "
             << executed_time
             << "/"
             << total_time
             << endl;

        if (executed_time >= total_time) {
            exec = ExecState::Finished;
        } else {
            exec = ExecState::Ready;
        }
    }

    void block() {
        if (exec == ExecState::Running ||
            exec == ExecState::Ready) {
            exec = ExecState::Blocked;
        }
    }

    void wakeup() {
        if (exec == ExecState::Blocked) {
            exec = ExecState::Ready;
        }
    }

    void suspend() {
        if (mem == Residency::Active) {
            mem = Residency::Suspended;
        }
    }

    void activate() {
        if (mem == Residency::Suspended) {
            mem = Residency::Active;
        }
    }
    
    void print() const {
        cout << "PID=" << id
             << " Name=" << name
             << " Progress=" << executed_time
             << "/" << total_time
             << endl;
    }

    ~Process() {};

private:
    string execToString() const {
        switch (exec) {
            case ExecState::Ready: return "Ready";
            case ExecState::Running: return "Running";
            case ExecState::Blocked: return "Blocked";
            case ExecState::Finished: return "Finished";
        }
        return "Unknown";
    }

    string memToString() const {
        switch (mem) {
            case Residency::Active: return "Active";
            case Residency::Suspended: return "Suspended";
        }
        return "Unknown";
    }
};

// Runtime Scheduler
class RuntimeSystem {
private:
    queue<Process*> readyQueue;
    vector<Process*> allProcesses;

    int quantum;
    int clock;

public:
    RuntimeSystem(int q)
        : quantum(q), clock(0) {}

    void addProcess(Process* p) {
        allProcesses.push_back(p);
        cout << "[ADD] Process "
             << p->getID()
             << " added."
             << endl;
        
        if (p->isRunnable()) {
            readyQueue.push(p);
            cout << "[READY] Process "
                 << p->getID()
                 << " added."
                 << endl;
        }
    }

    void rebuildReadyQueue() {
        queue<Process*> fresh;

        for (auto p : allProcesses) {
            if (p->isRunnable()) {
                fresh.push(p);
            }
        }

        readyQueue = fresh;
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

            if (current->getExecState() == ExecState::Finished) {
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