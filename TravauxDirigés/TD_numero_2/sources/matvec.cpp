// Produit matrice-vecteur
# include <cassert>
# include <vector>
# include <iostream>
#include <mpi.h>

// To run : mpirun --oversubscribe  -np  8 ./matvec.exe

// ---------------------------------------------------------------------
class Matrix : public std::vector<double>
{
public:
    Matrix (int dim);
    Matrix( int nrows, int ncols );
    Matrix( const Matrix& A ) = delete;
    Matrix( Matrix&& A ) = default;
    ~Matrix() = default;

    Matrix& operator = ( const Matrix& A ) = delete;
    Matrix& operator = ( Matrix&& A ) = default;
    
    double& operator () ( int i, int j ) {
        return m_arr_coefs[i + j*m_nrows];
    }
    double  operator () ( int i, int j ) const {
        return m_arr_coefs[i + j*m_nrows];
    }
    
    std::vector<double> operator * ( const std::vector<double>& u ) const;
    
    std::ostream& print( std::ostream& out ) const
    {
        const Matrix& A = *this;
        out << "[\n";
        for ( int i = 0; i < m_nrows; ++i ) {
            out << " [ ";
            for ( int j = 0; j < m_ncols; ++j ) {
                out << A(i,j) << " ";
            }
            out << " ]\n";
        }
        out << "]";
        return out;
    }
private:
    int m_nrows, m_ncols;
    std::vector<double> m_arr_coefs;
};
// ---------------------------------------------------------------------
inline std::ostream& 
operator << ( std::ostream& out, const Matrix& A )
{
    return A.print(out);
}
// ---------------------------------------------------------------------
inline std::ostream&
operator << ( std::ostream& out, const std::vector<double>& u )
{
    out << "[ ";
    for ( const auto& x : u )
        out << x << " ";
    out << " ]";
    return out;
}
// ---------------------------------------------------------------------
std::vector<double> 
Matrix::operator * ( const std::vector<double>& u ) const
{
    const Matrix& A = *this;
    assert( u.size() == unsigned(m_ncols) );
    std::vector<double> v(m_nrows, 0.);
    for ( int i = 0; i < m_nrows; ++i ) {
        for ( int j = 0; j < m_ncols; ++j ) {
            v[i] += A(i,j)*u[j];
        }            
    }
    return v;
}

// =====================================================================
Matrix::Matrix (int dim) : m_nrows(dim), m_ncols(dim),
                           m_arr_coefs(dim*dim)
{
    for ( int i = 0; i < dim; ++ i ) {
        for ( int j = 0; j < dim; ++j ) {
            (*this)(i,j) = (i+j)%dim;
        }
    }
}
// ---------------------------------------------------------------------
Matrix::Matrix( int nrows, int ncols ) : m_nrows(nrows), m_ncols(ncols),
                                         m_arr_coefs(nrows*ncols)
{
    int dim = (nrows > ncols ? nrows : ncols );
    for ( int i = 0; i < nrows; ++ i ) {
        for ( int j = 0; j < ncols; ++j ) {
            (*this)(i,j) = (i+j)%dim;
        }
    }    
}
// =====================================================================
void prod(const Matrix& A, int iPack,std::vector<double> u, double* buffer, int N){

    
    assert(u.size()==unsigned(N));
    for(int i = 0; i<N; ++i){
        buffer[i]+= A(i, iPack)*u[iPack];
    }  
}

std::vector<double> somme(std::vector<double> result, std::vector<double> buffer, int N){
    std::vector<double> v(N);
    for(int i =0; i<N; ++i){
        v[i] = result[i]+buffer[i];
    }
    return v;
}

// =====================================================================
int main( int nargs, char* argv[] )
{
    const int N = 120;
    Matrix A(N);
    std::vector<double> u( N );
    for ( int i = 0; i < N; ++i ) u[i] = i+1;
    

    // Maitre esclave :
    int rank, nbp;
    MPI_Init(&nargs, &argv);
    MPI_Comm globComm;
    MPI_Comm_dup(MPI_COMM_WORLD, &globComm);
    MPI_Comm_rank(globComm, &rank);
    MPI_Comm_size(globComm, &nbp);

    

    std::vector<double> buffer(N);
    std::vector<double> result(N);

    // Maitre
    if(rank==0){
        //iPack est le numero de colonne de A à faire intervenir dans le calcul, iPack in [0, N]
        int iPack;
        for(int i=1; i<nbp; i++){
            iPack = i-1;
            MPI_Send(&iPack, 1, MPI_INT, i, 101, globComm);
        }

        iPack = nbp-1;
        MPI_Status status;

        // On parcourt tous les packs
        while(iPack<N){
            //On recupere le message de n'importe quelle source
            MPI_Recv(buffer.data(), N, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, globComm,&status);

            // On recupere le rang de la source
            int slaveRk = status.MPI_SOURCE;

            // On somme dans le vecteur final :
            somme(result, buffer, N);
            // On envoie un message à cette source
            MPI_Send(&iPack, 1, MPI_INT, slaveRk, 101, globComm);
            iPack ++;
            
        }
        iPack = -1;

        // Pour tous les processus : 
        for(int i=1; i<nbp; i++){
            //On recupere le message de n'importe quelle source
            MPI_Recv(buffer.data(), N, MPI_DOUBLE, MPI_ANY_SOURCE, MPI_ANY_TAG, globComm,&status);

            // On recupere le rang de la source
            int slaveRk = status.MPI_SOURCE;

            // On somme dans le vecteur final :
            result = somme(result, buffer, N);

            // On envoie un message à cette source
            MPI_Send(&iPack, 1, MPI_INT, slaveRk, 101, globComm);
        }

         // A la fin, on test le resultat :
        std::cout  << "A : " << A << std::endl;
        std::cout << " u : " << u << std::endl;
        std::cout<<"\nTest du resultat :\n"<<std::endl;
        std::vector<double> v = A*u;
        std::cout<<"Produit classique : "<< v <<"\n"<<std::endl;
        std::cout<<"Produit parallelisé : "<<result<<"\n"<<std::endl;

        if(v==result){
            std::cout<<"\nTest validé\n"<<std::endl;
        }
        else{
            std::cout<<"\nTest non validé\n"<<std::endl;
        }




    }

    // Esclaves :
    else{
        MPI_Status status;
        int iPack;
        do{
            MPI_Recv(&iPack, 1, MPI_INT, 0, 101, globComm, &status);
            if(iPack!=-1){
                // Do
                // Produit de la colonne A_iPack avec u:
                prod(A, iPack, u, buffer.data(), N);


                // Renvoyer une info (numero de ligne du vecteur) en TAG
                MPI_Send(buffer.data(), N, MPI_DOUBLE, 0, iPack,globComm);
            }
        }while(iPack!=-1);
    }



    MPI_Finalize();
    
    return EXIT_SUCCESS;
}
