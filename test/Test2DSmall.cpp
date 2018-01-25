#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>

#include "../include/AlignedMemory.hpp"
#include "../include/SafeFFT.hpp"
#include "Timer.hpp"

#define TOTALSIZE 30
#define WORKNUMBER 10000

void benchSafeFFT() {
    safefft::SafeFFT myFFT;
    const int workNumber = WORKNUMBER;
    // a list of FFTs to run
    std::random_device rd;  // Will be used to obtain a seed for the random number engine
    std::mt19937 gen(rd()); // Standard mersenne_twister_engine seeded with rd()
    std::uniform_int_distribution<> dis(1, TOTALSIZE);

    struct FFT {
        safefft::PlanFFT myPlan;
        safefft::SafeFFT myFFT;
    };

    std::vector<FFT> work(workNumber);

    // prepare
    for (int i = 0; i < workNumber; i++) {
        const int workSize = 4 * dis(gen); // from 4 to 4*TOTALSIZE
        work[i].myPlan.n0 = workSize;
        work[i].myPlan.n1 = workSize;
        work[i].myPlan.nThreads = dis(gen) % 4 + 1; // 1 to 4 threads, nested omp threads
        work[i].myPlan.sign = 1;
    }

    // run
    Timer mytimer;
    mytimer.start();
#pragma omp parallel for
    for (int i = 0; i < workNumber; i++) {
        int tid = omp_get_thread_num();
        safefft::ComplexPtrT in = nullptr, out = nullptr;
        // printf("%u,%u\n", in, out);
        work[i].myFFT.fitBuffer(work[i].myPlan, in, out); // contain garbage data
        // printf("%u,%u\n", in, out);

        // run 10 times
        for (int c = 0; c < 10; c++) {
            work[i].myFFT.runFFT(work[i].myPlan, in, out);
        }
    }
    mytimer.stop("FFT finished");
    mytimer.dump();
}

int main(int argc, char **argv) {
    safefft::SafeFFT::init();
    benchSafeFFT();
    safefft::SafeFFT::finalize();
    return 0;
}