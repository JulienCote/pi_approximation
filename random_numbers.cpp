//Hello fellow human
//I was watching this video: https://www.youtube.com/watch?v=RZBhSi_PwHU
//About using the probability of 2 random natural numbers having a common factor to approximate a value for pi
//Math tells us this ratio is 6/'ratio of coprimes'
//So I decided to make a multithreaded program to test this with very large number of data
//to compile: g++ random_numbers.cpp -O3 -o random_pi -std=c++11 -lpthread
//--------------------------------------------------------------------------------------------------------------

#include <string>
#include <random>
#include <cmath>
#include <mutex>
#include <iostream>
#include <climits>
#include <iomanip>
#include <thread>
#include <csignal>
#include <condition_variable>

constexpr int BATCH_SIZE = 3000000;
constexpr int NBR_BATCH = 8;

unsigned long int total_tries = 0;
unsigned long int total_coprimes = 0;

bool kill_switch = false;

std::mutex data_lock;
std::condition_variable wait_sigint;

//Handler func
void sigint_handler (int param) {
    kill_switch = true;
    wait_sigint.notify_all();
}

//This is where the bit of probabilistic magic happens
//Not thread safe if data_lock is not locked
double pi() {
    long double prob = total_coprimes / (total_tries * 1.0);
    return std::sqrt(6L/prob);
}

//Euclidean Algorithm shamelessly grabbed on internet
unsigned long int gcd(unsigned long int a, unsigned long int b) {
    unsigned long int x;
    while (b) {
        x = a % b;
        a = b;
        b = x;
    }
    return a;
}

//Thread safe function for threads to update coprime and tries values and display the new approximation
void update_count(unsigned long int tries, unsigned long int coprimes) {
    data_lock.lock();
    total_tries += tries;
    total_coprimes += coprimes;
    std::cout << std::to_string(total_tries) << ": " << std::setprecision (30) <<  pi() << '\n';
    data_lock.unlock();
}

//Generate @size number of random pair of numbers and find the number of coprimes there are
void generate_coprimes(int size) {
    std::random_device rd;
    std::mt19937 generator(rd());  //Static threadlocal for performance and safety
    std::uniform_int_distribution<unsigned long int> uniform_dist(1, ULONG_MAX);
    unsigned long int rand1;
    unsigned long int rand2;

    while (!kill_switch) {
        int count = 0;        
        for(int i = 0; i < size; ++i) {
            rand1 = uniform_dist(generator);
            rand2 = uniform_dist(generator);

            if (gcd(rand1, rand2) == 1) {  //2 numbers having a greatest common denominator of 1 are coprimes
                count += 1;
            }
        }
        update_count(size, count);
    }
}

int main() {
    std::mutex kill_process;    
    std::unique_lock<std::mutex> process_life(kill_process);
    std::signal(SIGINT, sigint_handler);

    for(int i = 0; i < NBR_BATCH; ++i) {  //Generate threads
        std::thread(generate_coprimes, BATCH_SIZE).detach();
    }
    wait_sigint.wait(process_life);
}
