// 1(a)
void mesh(int iam, int np, MPI_Comm comm, int p, int q, int* myrow, int* mycol, MPI_Comm* rowcom, MPI_Comm* colcom) {
  int color, key;

  if (np < p * q) return;

  if (iam < p * q)
    color = iam / q;
  else
    color = MPI_UNDEFINED;

  key = iam;
  MPI_Comm_split(comm, color, key, rowcom);

  if (iam < p * q)
    color = iam % q;
  else
    color = MPI_UNDEFINED;

  key = iam;
  MPI_Comm_split(comm, color, key, colcom);

  if (iam < p * q) {
    MPI_Comm_rank(*colcom, myrow);
    MPI_Comm_rank(*rowcom, mycol);
  }
  return;
}

// 1(b)
/*
  P_st进程上的a_kl
  在矩阵A中位于第k*p+s行，第l*q+t列
  值为k*p+s+l*q+t
*/

// 1(c)
void main(int argc, char **argv) {
  int iam, np, myrow, mycol;
  MPI_Comm comm, rowcom, colcom;
  int size = (m + p - 1) / p * (n + q - 1) / q; // size of A in P_00
  // float [(m + p - 1) / p][(n + q - 1) / q] tmpA;
  MPI_Status status;

  MPI_Init(&argc, &argv);
  MPI_Comm_dup(MPI_COMM_WORLD, &comm);
  MPI_Comm_rank(comm, &iam);
  MPI_Comm_size(comm, &np);

  mesh(iam, np, comm, p, q, &myrow, &mycol, &rowcom, &colcom);

  // 首先广播到所有第0列的进程
  if (mycol == 0) {
    // MPI_Send(A, size, MPI_FLOAT, );
    MPI_Bcast((void*)A, size, MPI_FLOAT, 0, colcom);
  }
  
  // 然后由第0列的进程广播给所在行
  MPI_Bcast((void*)A, size, MPI_FLOAT, 0, rowcom);

  MPI_Finalize();
  return;
}
