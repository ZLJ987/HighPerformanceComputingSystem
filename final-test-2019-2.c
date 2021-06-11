// 假设这里A是stride*stride的矩阵
// 2(a) 构造一个新的类型，每次传输对角线上m*m的矩阵
void type_diag_matrix_block(int m, int n, int stride, MPI_Datatype oldtype, MPI_Datatype* newtype) {
  MPI_Datatype submatrix;
  MPI_Type_vector(m, n, stride, oldType, &submatrix);

  int blocklens[] = {1, 1};
  MPI_Aint indices[] = {0, (m * stride + n) * sizeof(oldtype)};
  MPI_Datatype oldtypes[] = {submatrix, MPI_UB};
  MPI_Type_struct(2, blocklens, indices, oldtypes, newtype);
}

// 2(b) 还可以将两个submatrix构成一个新的类型
void type_diag_matrix_block(int m, int n, int stride, MPI_Datatype oldtype, MPI_Datatype* newtype) {
  MPI_Datatype submatrix;
  MPI_Type_vector(m, n, stride, oldType, &submatrix);
  MPI_Type_vector(2, 1, stride + 1, submatrix, newtype);
}