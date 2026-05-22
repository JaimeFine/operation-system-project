#include <iostream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <chrono>
#include <semaphore>

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

int main() {
    reader_writer();

    std::vector<std::thread> philosophers;

    for (int i = 0; i < N; ++i) {
        philosophers.emplace_back(philosopher, i);
    }

    for (auto& p : philosophers) {
        p.join();
    }

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
Reader thread 2 finished reading.
Reader thread 1 finished reading.
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
Philosopher 4 is eating.
Philosopher 1 is eating.
Philosopher 3 is eating.
Philosopher 0 is eating.
Philosopher 4 is thinking.
Philosopher 1 is thinking.
Philosopher 0 is thinking.
Philosopher 4 is eating.
Philosopher 3 is thinking.
Philosopher 2 is eating.
Philosopher 1 is eating.
Philosopher 4 is thinking.
Philosopher 2 is thinking.
Philosopher 3 is eating.
Philosopher 2 is eating.
Philosopher 0 is eating.
Philosopher 1 is thinking.
Philosopher 3 is thinking.
Philosopher 2 is thinking.
Philosopher 4 is eating.
Philosopher 0 is thinking.
Philosopher 1 is eating.
Philosopher 0 is eating.
Philosopher 3 is eating.
Philosopher 2 is eating.
*/