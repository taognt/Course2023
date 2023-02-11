#include <iostream>
#include <mpi.h>

// mpirun -np 4 ./PointTwoPointExchangesBetweenTwoProcesses.exe


int main(int nargs, char* argv[] )
{
    int token, rank, nbp; // token, rank of the process, number of process
    MPI_Comm globCom;

    MPI_Init(&nargs, &argv);
    MPI_Comm_dup(MPI_COMM_WORLD, &globCom);
    MPI_Comm_size(globCom, &nbp);
    MPI_Comm_rank(globCom, &rank);


    int tag = 404;
    if (rank == 0)
    {   
        std::cout<<"\nrank : "<<rank<<"\n"<<std::endl;
        token = 1;// Token has universe's answer as value
        // Send the token (one int) to process 1 tagged with the tag variable
        MPI_Send(&token, 1, MPI_INT, 1, tag, globCom);
        //rank +=1;  rank figé
        MPI_Status status;
        MPI_Recv(&token, 1, MPI_INT, nbp-1, MPI_ANY_TAG, globCom, &status);
        std::cout << "Process " << rank << " has token " << token << " in memory." << std::endl;
    }
    else if (rank != 0 && rank<nbp-1)
    {
        std::cout<<"\nrank : "<<rank<<"\n"<<std::endl;
        MPI_Status status;
        // Receive the token (one int) from process 0 with no tag to care.
        MPI_Recv(&token, 1, MPI_INT, rank-1, MPI_ANY_TAG, globCom, &status);
        token +=1;
        MPI_Send(&token, 1, MPI_INT, rank+1, tag, globCom);
        //rank +=1;
    }
    else if(rank==nbp-1){
        std::cout<<"\nrank : "<<rank<<"\n"<<std::endl;
        MPI_Status status;
        // Receive the token (one int) from process 0 with no tag to care.
        MPI_Recv(&token, 1, MPI_INT, rank-1, MPI_ANY_TAG, globCom, &status);
        token +=1;
        MPI_Send(&token, 1, MPI_INT, 0 ,tag,globCom);
    }

    std::cout << "Process " << rank << " has token " << token << " in memory." << std::endl;

    MPI_Finalize();
}