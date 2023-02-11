#include <algorithm>
#include <cassert>
#include <iostream>
#include <thread>
#if defined(_OPENMP)
#include <omp.h>
#endif
#include "ProdMatMat.hpp"

namespace {
void prodSubBlocks(int iRowBlkA, int iColBlkB, int iColBlkA, int szBlock,
                   const Matrix& A, const Matrix& B, Matrix& C) {
      //#pragma omp parallel for //Inutile de le mettre sur les autres for : 8 coeurs déjà saturés
      for (int j = iColBlkB; j < std::min(B.nbCols, iColBlkB + szBlock); j++)
        for (int k = iColBlkA; k < std::min(A.nbCols, iColBlkA + szBlock); k++)
          for (int i = iRowBlkA; i < std::min(A.nbRows, iRowBlkA + szBlock); ++i)
              C(i, j) += A(i, k) * B(k, j);
      
}


const int szBlock = 256;
}  // namespace

Matrix operator*(const Matrix& A, const Matrix& B) {
  Matrix C(A.nbRows, B.nbCols, 0.0);
  //const int BlockSize = 64;

  for (int iRowBlkA = 0; iRowBlkA < A.nbRows; iRowBlkA += szBlock) {
    for (int iColBlkB = 0; iColBlkB < B.nbCols; iColBlkB += szBlock) {
      for (int iColBlkA = 0; iColBlkA < A.nbCols; iColBlkA += szBlock) {
        prodSubBlocks(iRowBlkA, iColBlkB, iColBlkA, szBlock, A, B, C);
      }
    }
  }
  //prodSubBlocks(0, 0, 0, szBlock, A, B, C);
  return C;
}
