#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <string>
#include <chrono>

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
    unsigned int delay;
    unsigned int duration;
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

void WriteProc(ThreadParam p) {
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
    std::ifstream infile("test1.txt");
    if (!infile.is_open()) {
        std::cerr << "Error opening test1.txt" << std::endl;
    }

    std::vector<std::thread> threads;
    ThreadParam tempParam;

    print_msg("Reader Priority:");

    while (
        infile >> tempParam.id >> tempParam.type >>
        tempParam.delay >> tempParam.duration
    ) {
        if (tempParam.type == 'R') {
            threads.emplace_back(ReaderProc, tempParam);
        } else {
            threads.emplace_back(WriterProc, tempParam);
        }
    }
    infile.close();

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
std::count_semaphore<4> room(4);

void philosopher(int id) {
    int left = id;
    int right = (id + 1) % N;

    while (true) {
        // Thinking
        std::cout << "Philosopher " << id << " is thinking.\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));

        // Enter dining room
        room.acquire();

        // Pick up forks
        forks[left].lock();
        forks[rigth].lock();

        // Eat
        std::cout << "Philosopher " << id << " is eating.\n";
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