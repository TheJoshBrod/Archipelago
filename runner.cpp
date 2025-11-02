#include <mpi.h>
#include <iostream>
#include <vector>
#include <cmath>


int main(int argc, char** argv){

    if (argc < 2){
        cout << "";
        return -1;
    }


    //Initialize the MPI environment.
    MPI_Init(&argc, &argv);

    //Obtain processor id and world size (number of processors)
    int P;
    int ID;
    MPI_Comm_size(MPI_COMM_WORLD, &P);
    MPI_Comm_rank(MPI_COMM_WORLD, &ID);

    

    //Finalize MPI
    MPI_Finalize();


    return 0;
}

