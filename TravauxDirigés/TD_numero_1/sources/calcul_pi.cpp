# include <chrono>
# include <random>
# include <cstdlib>
# include <sstream>
# include <string>
# include <fstream>
# include <iostream>
# include <iomanip>
# include <mpi.h>

// Attention , ne marche qu'en C++ 11 ou supérieur :
double approximate_pi( unsigned long nbSamples ) 
{
    typedef std::chrono::high_resolution_clock myclock;
    myclock::time_point beginning = myclock::now();
    myclock::duration d = beginning.time_since_epoch();
    unsigned seed = d.count();
    std::default_random_engine generator(seed);
    std::uniform_real_distribution <double> distribution ( -1.0 ,1.0);
    unsigned long nbDarts = 0;
    // Throw nbSamples darts in the unit square [-1 :1] x [-1 :1]
    for ( unsigned sample = 0 ; sample < nbSamples ; ++ sample ) {
        double x = distribution(generator);
        double y = distribution(generator);
        // Test if the dart is in the unit disk
        if ( x*x+y*y<=1 ) nbDarts ++;
    }
    // Number of nbDarts throwed in the unit disk
    double ratio = double(nbDarts)/double(nbSamples);
    return 4*ratio;
}

int main( int nargs, char* argv[] )
{
	// On initialise le contexte MPI qui va s'occuper :
	//    1. Créer un communicateur global, COMM_WORLD qui permet de gérer
	//       et assurer la cohésion de l'ensemble des processus créés par MPI;
	//    2. d'attribuer à chaque processus un identifiant ( entier ) unique pour
	//       le communicateur COMM_WORLD
	//    3. etc...
	MPI_Init( &nargs, &argv );
	// Pour des raisons de portabilité qui débordent largement du cadre
	// de ce cours, on préfère toujours cloner le communicateur global
	// MPI_COMM_WORLD qui gère l'ensemble des processus lancés par MPI.
	MPI_Comm globComm;
	MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
	// On interroge le communicateur global pour connaître le nombre de processus
	// qui ont été lancés par l'utilisateur :
	int nbp;
	MPI_Comm_size(globComm, &nbp);
	// On interroge le communicateur global pour connaître l'identifiant qui
	// m'a été attribué ( en tant que processus ). Cet identifiant est compris
	// entre 0 et nbp-1 ( nbp étant le nombre de processus qui ont été lancés par
	// l'utilisateur )
	int rank;
	MPI_Comm_rank(globComm, &rank);
	// Création d'un fichier pour ma propre sortie en écriture :
	std::stringstream fileName;
	fileName << "Output" << std::setfill('0') << std::setw(5) << rank << ".txt";
	std::ofstream output( fileName.str().c_str() );

	

	// Rajout de code....
	// rank 0 est le maitre
	int nb_sample = 10;
	// PAS BESOIN DE MAITRE : UTILISER GATHER !
	//
	if (rank == 0)
    {   
        int count_task = 0;
        for (int i=1; i<nbp; i++){
            MPI_send(&count_task, 1, MPI_INT, tag, globCom);
            count_task+=1;
        }

        while(count_task<nb_sample){
            //Status contient le numero du processus ayant envoyé le resultat
            MPI_recv(result, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, globCom, &status);
            MPI_send(&count_task, 1, MPI_INT, status.MPI_SOURCE, MPI_ANY_TAG, globCom);
            count_task+=1;
        }
        // On envoie un signal de terminaison a tous les processus
        count_task = -1;
        for(int i=1; i<nbp; i++) MPI_send(&count_task, 1, MPI_INT, i, MPI_ANY_TAG, globCom, &status);
    }

	// esclaves :
	if(rank>0){
        int num_task = 0;
        // Tant que numero de terminason non reçu:
        while(num_task !=-1){
            MPI_recv(&num_task, 1, MPI_INT, 0, MPI_ANY_TAG, globCom, &status);
            if(num_task >=0){
                //Tache correspondant au numero
                //execute_task(num_task, ...)
                MPI_send(result, 1, MPI_INT, 0, MPI_ANY_TAG, globCom);
            }
        }
    }

	output.close();
	// A la fin du programme, on doit synchroniser une dernière fois tous les processus
	// afin qu'aucun processus ne se termine pendant que d'autres processus continue à
	// tourner. Si on oublie cet instruction, on aura une plantage assuré des processus
	// qui ne seront pas encore terminés.
	MPI_Finalize();
	return EXIT_SUCCESS;
}

