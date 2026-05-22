#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <chrono>
#include <semaphore>

using namespace std;

class CountingSemaphore {
private:
    std::mutex mtx;
    std::condition_variable cv;
    int count;

public:
    explicit CountingSemaphore(int initial_count = 1) : count(initial_count) {}

    void acquire() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]() {
            return count > 0;
        });
        --count;
    }

    void release() {
        std::unique_lock<std::mutex> lock(mtx);
        ++count;
        cv.notify_one();
    }
};

struct ThreadParam {
    int id;
    char type;
    int delay;
    int duration;
};

// Global synchronization primitives
std::mutex g_mtx;
CountingSemaphore g_writeSem(1);
int g_nReaders = 0;

// Thread-safe console output function
void print_msg(const std::string& msg) {
    static std::mutex count_mtx;
    std::lock_guard<std::mutex> lock(count_mtx);
    std::cout << msg << std::endl;
}

void ReaderProc(ThreadParam p) {
    // Reader request
    print_msg("Reader thread " + std::to_string(p.id) + " sends R require...");

    std::this_thread::sleep_for(std::chrono::seconds(p.delay));

    {
        std::lock_guard<std::mutex> lock(g_mtx);
        g_nReaders++;
        if (g_nReaders == 1) {
            g_writeSem.acquire();
        }
    }

    print_msg("Reader thread " + std::to_string(p.id) + " begins to read.");
    std::this_thread::sleep_for(std::chrono::seconds(p.duration));
    print_msg("Reader thread " + std::to_string(p.id) + " finished reading.");

    {
        std::lock_guard<std::mutex> lock(g_mtx);
        g_nReaders--;
        if (g_nReaders == 0) {
            g_writeSem.release();   // Wake up
        }
    }
}

void WriterProc(ThreadParam p) {
    // Writer request
    print_msg("Writer thread " + std::to_string(p.id) + " sends W require...");

    std::this_thread::sleep_for(std::chrono::seconds(p.delay));

    g_writeSem.acquire();

    print_msg("Writer thread " + std::to_string(p.id) + " begins to write.");
    std::this_thread::sleep_for(std::chrono::seconds(p.duration));
    print_msg("Writer thread " + std::to_string(p.id) + " finished writing.");

    g_writeSem.release();
}

void reader_writer() {
    std::vector<ThreadParam> tasks = {
        {1, 'R', 1, 3},
        {2, 'R', 2, 2},
        {1, 'W', 3, 4},
        {3, 'R', 4, 2},
        {2, 'W', 5, 3},
        {4, 'R', 6, 1}
    };

    std::vector<std::thread> threads;
    
    for (const auto& task : tasks) {
        if (task.type == 'R') {
            threads.emplace_back(
                ReaderProc,
                task
            );
        } else {
            threads.emplace_back(
                WriterProc,
                task
            );
        }
    }

    // Join the threads
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    print_msg("All reader and writer have been finished operating");
}

const int N = 5;

std::mutex forks[N];

// Trying C++20 hahaha
std::counting_semaphore<4> room(4);

void philosopher(int id) {
    int left = id;
    int right = (id + 1) % N;

    for (int meal = 0; meal < 3; ++meal) {
        // Thinking
        print_msg("Philosopher " + std::to_string(id) + " is thinking.");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Enter dining room
        room.acquire();

        // Pick up forks
        forks[left].lock();
        forks[right].lock();

        // Eat
        print_msg("Philosopher " + std::to_string(id) + " is eating.");
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Put down forks
        forks[right].unlock();
        forks[left].unlock();

        // Leave room
        room.release();
    }
}

std::counting_semaphore<1> left_side(1);
std::counting_semaphore<2> middle(2);
std::counting_semaphore<1> right_side(1);

void cross_bridge(int id) {
    print_msg("Person " + to_string(id) + " arrive at left side.");

    left_side.acquire();
    print_msg("Person " + to_string(id) + " enters left narrow section.");
    this_thread::sleep_for(chrono::seconds(1));
    left_side.release();

    middle.acquire();
    print_msg("Person " + to_string(id) + " enters middle section.");
    this_thread::sleep_for(chrono::seconds(2));
    middle.release();

    right_side.acquire();
    print_msg("Person " + to_string(id) + " crossed the bridge.");
    this_thread::sleep_for(chrono::seconds(1));
    right_side.release();

    print_msg("Person " + to_string(id) + " crossed the bridge.");
}

int main() {
    reader_writer();

    std::vector<std::thread> philosophers;

    for (int i = 0; i < N; ++i) {
        philosophers.emplace_back(philosopher, i);
    }

    for (auto& p : philosophers) {
        p.join();
    }

    vector<thread> people;

    const int NUM_PEOPLE = 10;

    for (int i = 1; i <= NUM_PEOPLE; ++i) {

        people.emplace_back(cross_bridge, i);

        // Small delay to simulate arrivals
        this_thread::sleep_for(chrono::milliseconds(300));
    }

    for (auto& t : people) {
        t.join();
    }

    print_msg("All people crossed the bridge.");

    return 0;
}

/*
PS C:\Users\13647\OneDrive\Desktop\MiMundo\2026 Spring\operation_system> g++ -std=c++20 course_experiment_3.cpp -o run
PS C:\Users\13647\OneDrive\Desktop\MiMundo\2026 Spring\operation_system> ./run
Reader thread 1 sends R require...
Reader thread 2 sends R require...
Writer thread 1 sends W require...
Reader thread 3 sends R require...
Writer thread 2 sends W require...
Reader thread 4 sends R require...
Reader thread 1 begins to read.
Reader thread 2 begins to read.
Reader thread 3 begins to read.
Reader thread 1 finished reading.
Reader thread 2 finished reading.
Reader thread 4 begins to read.
Reader thread 3 finished reading.
Reader thread 4 finished reading.
Writer thread 1 begins to write.
Writer thread 1 finished writing.
Writer thread 2 begins to write.
Writer thread 2 finished writing.
All reader and writer have been finished operating
Philosopher 0 is thinking.
Philosopher 1 is thinking.
Philosopher 2 is thinking.
Philosopher 3 is thinking.
Philosopher 4 is thinking.
Philosopher 1 is eating.
Philosopher 3 is eating.
Philosopher 3 is thinking.
Philosopher 2 is eating.
Philosopher 1 is thinking.
Philosopher 0 is eating.
Philosopher 0 is thinking.
Philosopher 4 is eating.
Philosopher 1 is eating.
Philosopher 2 is thinking.
Philosopher 1 is thinking.
Philosopher 3 is eating.
Philosopher 4 is thinking.
Philosopher 0 is eating.
Philosopher 2 is eating.
Philosopher 3 is thinking.
Philosopher 4 is eating.
Philosopher 0 is thinking.
Philosopher 4 is thinking.
Philosopher 2 is thinking.
Philosopher 1 is eating.
Philosopher 3 is eating.
Philosopher 2 is eating.
Philosopher 0 is eating.
Philosopher 4 is eating.
Person 1 arrive at left side.
Person 1 enters left narrow section.
Person 2 arrive at left side.
Person 3 arrive at left side.
Person 4 arrive at left side.
Person 1 enters middle section.
Person 4 enters left narrow section.
Person 5 arrive at left side.
Person 6 arrive at left side.
Person 7 arrive at left side.
Person 4 enters middle section.
Person 7 enters left narrow section.
Person 8 arrive at left side.
Person 9 arrive at left side.
Person 10 arrive at left side.
Person 1 crossed the bridge.
Person 7 enters middle section.
Person 8 enters left narrow section.
Person 1 crossed the bridge.
Person 4 crossed the bridge.
Person 8 enters middle section.
Person 9 enters left narrow section.
Person 4 crossed the bridge.
Person 7 crossed the bridge.
Person 6 enters left narrow section.
Person 9 enters middle section.
Person 7 crossed the bridge.
Person 8 crossed the bridge.
Person 6 enters middle section.
Person 5 enters left narrow section.
Person 9 crossed the bridge.
Person 8 crossed the bridge.
Person 5 enters middle section.
Person 10 enters left narrow section.
Person 9 crossed the bridge.
Person 6 crossed the bridge.
Person 2 enters left narrow section.
Person 10 enters middle section.
Person 5 crossed the bridge.
Person 3 enters left narrow section.
Person 2 enters middle section.
Person 6 crossed the bridge.
Person 5 crossed the bridge.
Person 3 enters middle section.
Person 10 crossed the bridge.
Person 10 crossed the bridge.
Person 2 crossed the bridge.
Person 3 crossed the bridge.
Person 2 crossed the bridge.
Person 3 crossed the bridge.
All people crossed the bridge.
*/

