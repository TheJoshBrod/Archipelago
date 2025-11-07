#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <memory>
#include <functional>

#include "habitant.cpp"

using namespace std;

int main(int argc, char** argv){

    vector<function<double(habitant)>> heuristics;
    // PUT YOUR HEURISTICS HERE
    
    // Example (closeness to line): y = 2x + 1
    heuristics.push_back([](habitant h) -> double {
        double dist = abs(2 * h.x - h.y + 1) / sqrt(5);
        return 1.0 / (1.0 + dist);
    });

    // Example (closeness to quadrant 1): y + x > 0
    heuristics.push_back([](habitant h) -> double {
        if (h.x < 0 || h.y < 0){
            return 0.0;
        }
        return 1.0;
    });


    //Initialize the MPI environment.
    MPI_Init(&argc, &argv);

    //Obtain processor id and world size (number of processors)
    int P;
    int ID;
    MPI_Comm_size(MPI_COMM_WORLD, &P);
    MPI_Comm_rank(MPI_COMM_WORLD, &ID);

    // Obtain the number of threads
    int num_threads;
    #pragma omp parallel
    {
        #pragma omp single
        num_threads = omp_get_num_threads();
    }

    
    int T = P * num_threads;   // total threads over entire cluster
    int t = heuristics.size(); // number of heuristics (tasks)

    // Set up islands for each thread
    vector<shared_ptr<Island>> island_series;
    island_series.resize(num_threads);

    #pragma omp parallel
    {
        // Get island position
        int localThread = omp_get_thread_num();
        int globalThreadID = ID * num_threads + localThread;

        // FAIR DISTRIBUTION:
        // if fewer tasks than threads -> round robin assignment
        int heuristicIndex = globalThreadID % t;

        // Shared resource    
        static vector<vector<habitant>> mailboxes;
        #pragma omp single
            mailboxes.assign(num_threads, {});

        // *****************
        // Initialize Values
        // *****************
        auto island = make_shared<Island>(heuristics[heuristicIndex], heuristicIndex);
        island_series[localThread] = island;
        island->initial_generation();

        // ***********************
        // Evolution/Communication
        // ***********************

        const size_t M = 5; // number of migrants to exchange each step

        // ******************************
        // LOCAL EXCHANGE (same MPI rank)
        // ******************************
        for (int gen = 0; gen < 20; gen++){
            island->generate_next_generation();

            // pick outbound migrants
            auto selected = island->select_immigrants(M);

            vector<habitant> outbound;
            outbound.reserve(selected.size());
            for (auto ptr : selected)
                outbound.push_back(*ptr);

            // write to mailbox
            mailboxes[localThread] = outbound;

            // determine where to receive from (ring topology)
            int receive_from = (localThread == 0 ? num_threads - 1 : localThread - 1);

            // wait for all threads to finish sending
            #pragma omp barrier

            // read migrants from neighbor
            vector<habitant> inbound = mailboxes[receive_from];
            island->receive_immigrants(inbound);
        }


        // Global loop, send your immigrants to same localthread on next processor
        
    }

    //Finalize MPI
    MPI_Finalize();


    return 0;
}

