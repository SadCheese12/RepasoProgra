#include <iostream>
#include <chrono>
#include <list>
#include <mpi.h>
#include <vector>
#include<stdlib.h>
#include <ctime>
#include <cmath>

std::vector<int> isParallel(std::vector<int> A, std::vector<int> C){
    std::vector<int> areParallels(A.size()/3);
    int x1 = C[0];
    int y1 = C[1];
    int z1 = C[2];

    //#pragma omp parallel for
    for(int i = 0; i < A.size(); i++){
        int x2 = A[i];
        int y2 = A[i+1];
        int z2 = A[i+2];
        float prop1 = (float)x1/x2;
        float prop2 = (float)y1/y2;
        float prop3 = (float)z1/z2;
        bool r = (prop1 == prop2 && prop1 == prop3);
        areParallels[i/3] = r ? 1 : 0;
        i+=2;
        
    }
    return areParallels;

}

int main(int argc, char** argv){
    int rank;
    int size;

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    MPI_Request request;

    if(rank == 0){
        std::vector<int> A = {1,3,2,6,18,12,6,5,5,7,9,6,5,3,6,3,8,1, //pa 0
        5,5,8,8,3,1,6,5,9,9,2,2,1,9,3,1,8,7,7,4,5,4,7,1,//pa 1
        2,2,8,6,7,8,3,8,5,1,9,9,4,5,1,5,6,4,7,5,3,9,1,2,//pa 2
        6,4,8,1,4,5,8,8,7,8,9,6,8,2,7,4,5,6,3,6,3,4,12,8};//pa 3
        std::vector<int> C = {2,6,4};

        //Envio datos
        MPI_Isend( &A[18], 24, MPI_INT, 1, 0, MPI_COMM_WORLD, &request);
        MPI_Isend( &A[42], 24, MPI_INT, 2, 0, MPI_COMM_WORLD, &request);
        MPI_Isend( &A[66], 24, MPI_INT, 3, 0, MPI_COMM_WORLD, &request);
    
        //Envio centros
        /*for(int i = 1; i<size; i++){
            MPI_Send(C.data(), C.size(), MPI_INT, i, 0, MPI_COMM_WORLD);
        }*/
        MPI_Bcast(C.data(), C.size(), MPI_INT, 0, MPI_COMM_WORLD);

        //Proceso lo mio
        std::vector<int> miParte(18);
        //miParte.insert(miParte.begin(), A.begin(), 17);
        for(int i = 0; i < 18; i++){
            miParte[i] = A[i];
        }
        //printf("El rank %d tiene desde %d hasta %d\n", rank, miParte[0], miParte[17]);

        std::vector<int> paralelosTmp0(miParte.size()/3); 
        paralelosTmp0 = isParallel(miParte, C);

        //recibo de los otros ranks
        std::vector<int> paralelosTmp1(8);
        std::vector<int> paralelosTmp2(8);
        std::vector<int> paralelosTmp3(8);

        MPI_Wait(&request, MPI_STATUS_IGNORE);

        MPI_Irecv(paralelosTmp1.data(), 8, MPI_INT, 1, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
        MPI_Irecv(paralelosTmp2.data(), 8, MPI_INT, 2, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
        MPI_Irecv(paralelosTmp3.data(), 8, MPI_INT, 3, MPI_ANY_TAG, MPI_COMM_WORLD, &request);

        std::vector<int> paralelosBools;
        //printf("D %ld\n", paralelosBools.size());
        paralelosBools.insert(paralelosBools.end(), paralelosTmp0.begin(), paralelosTmp0.end());
        //printf("D %d\n", paralelosBools[0]);
        paralelosBools.insert(paralelosBools.end(), paralelosTmp1.begin(), paralelosTmp1.end());
        paralelosBools.insert(paralelosBools.end(), paralelosTmp2.begin(), paralelosTmp2.end());
        paralelosBools.insert(paralelosBools.end(), paralelosTmp3.begin(), paralelosTmp3.end());

        //impresion

        for(int i = 0; i< A.size()/3; i++){
            printf("El vector (%d,%d,%d) es paralelo con (%d,%d,%d) ? %d\n", A[3*i],A[3*i+1],A[3*i+2],C[0],C[1],C[2],paralelosBools[i]);
        }

        

    }else{
        //Proceso lo de los demas ranks
        std::vector<int> datos(24);
        MPI_Irecv(datos.data(), 24, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &request);
        std::vector<int> centro(3);
        MPI_Bcast(centro.data(), centro.size(), MPI_INT, 0, MPI_COMM_WORLD);
        //MPI_Recv(centro.data(), 3, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //printf("El rank %d recibio desde %d hasta %d\n", rank, datos[0], datos[23]);

        //Calculo lo mio
        MPI_Wait(&request, MPI_STATUS_IGNORE);
        std::vector<int> aux(datos.size()/3);
        aux = isParallel(datos, centro);

        //devuelvo
        MPI_Isend(aux.data(), 8, MPI_INT, 0, 0, MPI_COMM_WORLD, &request);
    }
    MPI_Finalize();
    return 0;
}