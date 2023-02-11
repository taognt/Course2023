# include <iostream>
# include <cstdlib>
# include <string>
# include <chrono>
# include <cmath>
# include <vector>
# include <fstream>
#include <mpi.h>

// To run the code :
//mpirun --oversubscribe  -np  8 ./Mandelbrot.exe


/** Une structure complexe est définie pour la bonne raison que la classe
 * complex proposée par g++ est très lente ! Le calcul est bien plus rapide
 * avec la petite structure donnée ci--dessous
 **/
struct Complex
{
    Complex() : real(0.), imag(0.)
    {}
    Complex(double r, double i) : real(r), imag(i)
    {}
    Complex operator + ( const Complex& z )
    {
        return Complex(real + z.real, imag + z.imag );
    }
    Complex operator * ( const Complex& z )
    {
        return Complex(real*z.real-imag*z.imag, real*z.imag+imag*z.real);
    }
    double sqNorm() { return real*real + imag*imag; }
    double real,imag;
};

std::ostream& operator << ( std::ostream& out, const Complex& c )
{
  out << "(" << c.real << "," << c.imag << ")" << std::endl;
  return out;
}

/** Pour un c complexe donné, calcul le nombre d'itérations de mandelbrot
 * nécessaires pour détecter une éventuelle divergence. Si la suite
 * converge, la fonction retourne la valeur maxIter
 **/
int iterMandelbrot( int maxIter, const Complex& c)
{
    Complex z{0.,0.};
    // On vérifie dans un premier temps si le complexe
    // n'appartient pas à une zone de convergence connue :
    // Appartenance aux disques  C0{(0,0),1/4} et C1{(-1,0),1/4}
    if ( c.real*c.real+c.imag*c.imag < 0.0625 )
        return maxIter;
    if ( (c.real+1)*(c.real+1)+c.imag*c.imag < 0.0625 )
        return maxIter;
    // Appartenance à la cardioïde {(1/4,0),1/2(1-cos(theta))}    
    if ((c.real > -0.75) && (c.real < 0.5) ) {
        Complex ct{c.real-0.25,c.imag};
        double ctnrm2 = sqrt(ct.sqNorm());
        if (ctnrm2 < 0.5*(1-ct.real/ctnrm2)) return maxIter;
    }
    int niter = 0;
    while ((z.sqNorm() < 4.) && (niter < maxIter))
    {
        z = z*z + c;
        ++niter;
    }
    return niter;
}

/**
 * On parcourt chaque pixel de l'espace image et on fait correspondre par
 * translation et homothétie une valeur complexe c qui servira pour
 * itérer sur la suite de Mandelbrot. Le nombre d'itérations renvoyé
 * servira pour construire l'image finale.
 
 Sortie : un vecteur de taille W*H avec pour chaque case un nombre d'étape de convergence de 0 à maxIter
 MODIFICATION DE LA FONCTION :
 j'ai supprimé le paramètre W étant donné que maintenant, cette fonction ne prendra plus que des lignes de taille W en argument.
 **/
void computeMandelbrotSetRow( int W, int H, int maxIter, int num_ligne, int* pixels)
{
    // Calcul le facteur d'échelle pour rester dans le disque de rayon 2
    // centré en (0,0)
    double scaleX = 3./(W-1);
    double scaleY = 2.25/(H-1.);
    //
    // On parcourt les pixels de l'espace image :
    for ( int j = 0; j < W; ++j ) {
       Complex c{-2.+j*scaleX,-1.125+ num_ligne*scaleY};
       pixels[j] = iterMandelbrot( maxIter, c );
    }
}

std::vector<int> computeMandelbrotSet( int W, int H, int maxIter )
{

    int premiere_ligne;
    int derniere_ligne;
    int rank, nbp;
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    MPI_Comm_rank(globComm, &rank);
    MPI_Comm_size(globComm, &nbp);

    int H_proc = H/nbp;
    premiere_ligne = (nbp - rank -1)*H_proc;
    derniere_ligne = H_proc*(nbp - rank);

    std::chrono::time_point<std::chrono::system_clock> start, end;
    std::vector<int> pixels(W*H_proc);

    start = std::chrono::system_clock::now();
    // On parcourt les pixels de l'espace image :
    // Boucle sur la hauteur de l'image
    for ( int i = premiere_ligne; i < derniere_ligne; ++i ) {
      computeMandelbrotSetRow(W, H, maxIter, i, pixels.data() + W*(H_proc-i-1) );
    }
    end = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = end-start;
    std::cout << "Temps calcul ensemble mandelbrot : " << elapsed_seconds.count() 
              << std::endl;
    return pixels;
}

/** Construit et sauvegarde l'image finale **/
void savePicture( const std::string& filename, int W, int H, const std::vector<int>& nbIters, int maxIter )
{
    double scaleCol = 1./maxIter;//16777216
    std::ofstream ofs( filename.c_str(), std::ios::out | std::ios::binary );
    ofs << "P6\n"
        << W << " " << H << "\n255\n";
    for ( int i = 0; i < W * H; ++i ) {
        double iter = scaleCol*nbIters[i];
        unsigned char r = (unsigned char)(256 - (unsigned (iter*256.) & 0xFF));
        unsigned char b = (unsigned char)(256 - (unsigned (iter*65536) & 0xFF));
        unsigned char g = (unsigned char)(256 - (unsigned( iter*16777216) & 0xFF));
        ofs << r << g << b;
    }
    ofs.close();
}

int main(int argc, char *argv[] ) 
 { 
    int rank,  nbp;
    MPI_Init( &argc, &argv );
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    MPI_Comm_rank(globComm, &rank);
    MPI_Comm_size(globComm, &nbp);
    const int W = 800;
    const int H = 600;
    // Normalement, pour un bon rendu, il faudrait le nombre d'itérations
    // ci--dessous :
    //const int maxIter = 16777216;

    // Penser en indices locaux et globaux
    const int maxIter = 20*65536;
    //auto iters = computeMandelbrotSet(W, H, maxIter);

    // Maitre esclave : Meilleur speedup qu'en statique
    std::vector<int> pixels_proc(W);
    std::vector<int> pixels_end(H*W);
    //pixels_end.reserve(W*H);
    std::vector<int> pixels_rcv(W);

    //Time
    auto beg = std::chrono::high_resolution_clock::now();

    if(rank==0){
    
        //MPI_Gather(iters.data(), W*H_proc, MPI_INT, iters.data(), W*H_proc, MPI_INT, 0, globComm);
        // On écrit sur le disque dur : Non parallélisable
        // On ne peut pas espérer du speedup sur la sauvegarde

        // Send / Receive
        int iPack;
        for(int i=1; i<nbp; i++){
            iPack = i-1;
            // i est le rank dur proc
            MPI_Send(&iPack, 1, MPI_INT, i, 101, globComm);
        }

        iPack = nbp-1;
        MPI_Status status;
        // On parcourt tous les packs
        while(iPack<H){
            // a definir
            // On recupere le message de n'importe quelle source
            MPI_Recv(pixels_rcv.data(), W, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, globComm, &status);
      
            // On recupere le rang de la source
            int slaveRk = status.MPI_SOURCE;
            // Slave renvoie en tag le numero de la ligne qu'il a traité
            int slaveTag = status.MPI_TAG;

            // On rajoute l'information dans le vecteur de pixel global
            std::copy( pixels_rcv.begin(), pixels_rcv.end(),&pixels_end[slaveTag*W]);

            // On envoie un message à cette source
            MPI_Send(&iPack, 1, MPI_INT, slaveRk, 101, globComm);
            iPack ++;
        }

        iPack = -1;

        // pour tous les proc ...
        for(int i=1; i<nbp; i++){
            // On recupere le message de n'importe quelle source
            MPI_Recv(pixels_rcv.data(), W, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, globComm, &status);

            // On rajoute l'information dans le vecteur de pixel global
            // Slave renvoie en tag le numero de la ligne qu'il a traité
            int slaveTag = status.MPI_TAG;
            std::copy( pixels_rcv.begin(), pixels_rcv.end(),&pixels_end[slaveTag*W]);

            // On recupere le rang de la source
            int slaveRk = status.MPI_SOURCE;

            // On envoie un message à cette source
            MPI_Send(&iPack, 1, MPI_INT, slaveRk, 101, globComm);


        }
        // On sauvegarde
        savePicture("mandelbrot.tga", W, H, pixels_end, maxIter);
        std::cout<<"Image saved\n"<<std::endl;

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duree = end - beg;
        std::cout << "Temps de calcul : " << duree.count()<<" seconds"<< std::endl;




    }

    else{
        //MPI_Gather(iters.data(), W*H_proc, MPI_INT, NULL, W*H_proc, MPI_INT, 0, globComm);
        MPI_Status status;
        int iPack;
        do{
            // On recoit l'ordre du maitre
            MPI_Recv(&iPack, 1, MPI_INT, 0, 101,globComm, &status);
            if(iPack!=-1){
                computeMandelbrotSetRow(W, H, maxIter, iPack, pixels_proc.data());
                //renvoyer numero ligne dans TAG
                MPI_Send(pixels_proc.data(), W, MPI_INT, 0, iPack, globComm);
            }
        } while(iPack!=-1); 

    }

    MPI_Finalize();
    return EXIT_SUCCESS;
 }
    
