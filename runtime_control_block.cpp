#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <thread>
#include <chrono>
#include <condition_variable>
#include <mutex>
#include <memory>

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

    int run(int quantum) {
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

        return slice;
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
    vector<unique_ptr<Process>> allProcesses;

    int quantum;
    int clock;

public:
    RuntimeSystem(int q)
        : quantum(q), clock(0) {}

    void addProcess(unique_ptr<Process> p) {
        cout << "[ADD] Process "
             << p->getID()
             << " added."
             << endl;
        
        if (p->isRunnable()) {
            readyQueue.push(p.get());
            cout << "[READY] Process "
                 << p->getID()
                 << " added."
                 << endl;
        }
        allProcesses.push_back(move(p));
    }

    void run() {
        cout << "\n===== Scheduler Started =====\n";

        while (!readyQueue.empty()) {
            Process* current = readyQueue.front();
            readyQueue.pop();

            cout << "[CLOCK " << clock << "] Dispatching PID "
                 << current->getID()
                 << endl;

            clock += current->run(quantum);

            if (current->getExecState() == ExecState::Finished) {
                cout << "[EXIT] Process "
                     << current->getID()
                     << " finished."
                     << endl;
            } else {
                readyQueue.push(current);
            }
        }
        cout << "\n===== Scheduler Finished =====\n";
    }
};

class OSMessageQueue {
private:
    queue<int> buffer;
    const unsigned int maxSize;
    mutex mtx;
    condition_variable cv_producer;
    condition_variable cv_consumer;

public:
    OSMessageQueue(unsigned int size) : maxSize(size) {}

    // Producer
    void produce(int item, int pid) {
        unique_lock<mutex> lock(mtx);

        cv_producer.wait(lock, [this]() {
            return buffer.size() < maxSize;
        });

        buffer.push(item);
        cout << "[PID " << pid << " : PRODUCER] Inserted job: "
             << item << " | Queue size: " << buffer.size()
             << "/" << maxSize << endl;

        cv_consumer.notify_one();
    }

    // Consumer
    int consume(int pid) {
        unique_lock<mutex> lock(mtx);
        cv_consumer.wait(lock, [this]() {
            return !buffer.empty();
        });

        int item = buffer.front();
        buffer.pop();
        cout << "[PID " << pid << " : CONSUMER] Processed job: "
             << item << " | Queue size: " << buffer.size()
             << "/" << maxSize << endl;

        cv_producer.notify_one();
        return item;
    }
};

int main() {
    RuntimeSystem kernel(5);
    kernel.addProcess(make_unique<Process>(1, "Init", 12));
    kernel.addProcess(make_unique<Process>(2, "Shell", 7));
    
    kernel.run();

    cout << "\n===== Starting IPC Simulation (Producer-Consumer) =====\n";
    OSMessageQueue ipc_bus(3); // Buffer size of 3

    // Launching concurrent "Processes" using C++ threads
    thread producer([&]() {
        for(int i = 0; i < 5; ++i) {
            this_thread::sleep_for(chrono::milliseconds(100));
            ipc_bus.produce(i + 100, 301);
        }
    });

    thread consumer([&]() {
        for(int i = 0; i < 5; ++i) {
            this_thread::sleep_for(chrono::milliseconds(150));
            ipc_bus.consume(401);
        }
    });

    producer.join();
    consumer.join();

    cout << "\n[KERNEL] System Shutdown Cleanly.\n";
    return 0;
}