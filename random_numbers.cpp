//Hello fellow human
//I was watching this video: https://www.youtube.com/watch?v=RZBhSi_PwHU
//About using the probability of 2 random natural numbers having a common factor to approximate a value for pi
//Math tells us this ratio is 6/'ratio of coprimes'
//So I decided to make a multithreaded program to test this with very large number of data
//to compile: g++ random_numbers.cpp -O3 -o random_pi -std=c++11 -lpthread

#include <random>
#include <cmath>
#include <future>
#include <iostream>
#include <climits>
#include <iomanip>

constexpr int BATCH_SIZE = 1000000;
constexpr int NBR_BATCH = 8;

//This is where the bit of probabilistic magic happens
double pi(unsigned long int tries, unsigned long int coprimes) {
    long double prob = coprimes / (tries * 1.0);
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

//Generate @size number of random pair of numbers and find the number of coprimes there are
unsigned long int generate_coprimes(int size) {
    std::random_device rd;
    static thread_local std::mt19937 generator(rd());  //Static threadlocal for performance and safety
    std::uniform_int_distribution<unsigned long int> uniform_dist(1, ULONG_MAX);

    int count = 0;
    unsigned long int rand1;
    unsigned long int rand2;
    for(int i = 0; i < size; ++i) {
        rand1 = uniform_dist(generator);
        rand2 = uniform_dist(generator);

        if (gcd(rand1, rand2) == 1) {  //2 numbers having a greatest common denominator of 1 are coprimes
            count += 1;
        }
    }
    return count;
}

int main() {
    unsigned long int coprimes = 0;

    //Max safe precision for an unsigned long
    //This could theoretically end someday, but you probably won't be around to see it
    for (unsigned long int tries = NBR_BATCH * BATCH_SIZE;
            tries < ULONG_MAX - (NBR_BATCH * BATCH_SIZE);
            tries += NBR_BATCH * BATCH_SIZE)
        {

        std::vector<std::future<unsigned long int>> futures{};  //Thread container
        for(int i = 0; i < NBR_BATCH; ++i) {  //Generate a threads
            futures.emplace_back(std::async(std::launch::async, generate_coprimes, BATCH_SIZE));
        }

        for (size_t i = 0; i < futures.size(); ++i) {  //Get number of coprimes in batch of random numbers
            coprimes += futures.at(i).get();
        }
        
        //approximate a value for Pi using statistical probability of 2 random numbers being coprimes
        std::cout << std::to_string(tries) << ": " << std::setprecision (30) <<  pi(tries, coprimes) << '\n';
    }    
}
