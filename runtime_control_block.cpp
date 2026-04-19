#include <iostream>
#include <string>

using namespace std;

enum class State {
    Created,
    ReadyActive,
    ReadySuspend,
    Running,
    BlockedActive,
    BlockedSuspend
};

class RuntimeControlBlock {
private:
    unsigned int id;    // Internal label
    string name;        // External label
    unsigned int time;
    State state;

public:
    RuntimeControlBlock(unsigned int id, const string &name)
        : id(id), name(name), time(0), state(State::Created) {}

    void readya() {
        state = State::ReadyActive;
    }

    void readys() {
        state = State::ReadySuspend;
    }

    void dispatch(unsigned int slice) {
        if (state == State::ReadyActive) {
            state = State::Running;
            time += slice;
        } else {
            cout << "Error dispatching!!!" << endl;
        }
    }
    
    void print() const {
        cout << "Process " << id << " (" << name << "), time="
        << time << endl;
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

    ~RuntimeControlBlock() {};
};

int main() {
    RuntimeControlBlock* runtime = new RuntimeControlBlock(
        1, "Chrome"
    );

    runtime->print();

    delete runtime;
    return 0;
}