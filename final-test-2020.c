#define n 1000
void matrix_mul(void* A, void *B, int x, int y, int z, void *C, MPI_Datatype type) {
  for (int i = 0; i < x; i++) {
    for (int j = 0; j < z; j++) {
      // C[i * z + j] = 0;
      for (int k = 0; k < y; k++) {
        C[i * z + j] += A[i * y + k] * B[k * z + j];
      }
    }
  }
}

void matrix_add(void *A, void *B, void *C, int x, int y, MPI_Datatype type) {
  for (int i = 0; i < x; i++)
    for (int j = 0; j < y; j++)
      C[i * y + j] = A[i * y + j] + B[i * y + j];
}

void linear_map(void *A, void *x, void *b, int n, int np, int iam, MPI_Datatype type, MPI_Comm com) {
  float result[n], sendbuf[n], recvbuf[n];
  int m = n / np;
  MPI_Status status;
  int prev = (np + iam - 1) % np;
  int next = (iam + 1) % np;
  for (int i = 0; i < n; i++)
    result[i] = 0.0;
  // 1. Scatter b to each process
  // MPI_Scatter(b, m, type, (void*)&result[iam * m], recvcnt, type, 0, com);
  // or
  if (iam == 0) {
    for (int i = 1; i < np; i++) {
      MPI_Send(b, m, type, i, MPI_ANY_TAG, com);
    }
  } else {
    MPI_Recv((void*)&result[iam * m], m, type, 0, MPI_ANY_TAG, com, &status);
  }
  // 2. Calculate partial sum = Ax + partial b
  matrix_mul(A, x, n, m, 1, (void*)result, type);
  // 3. Reduce to x in process 0
  // MPI_Reduce((void*)result, x, n, type, MPI_SUM, 0, com);
  // or
  // Each process calculate part of the reduced result, and send to process 0 at the end
  memcpy(sendbuf, result, n * sizeof(float));
  for (int i = 1; i < np; i++) {
    MPI_Sendrecv((void*)sendbuf, n, type, prev, MPI_ANY_TAG,
      (void*)recvbuf, n, type, next, MPI_ANY_TAG,
      com, &status);
    matrix_add((void*)&result[iam * m], (void*)&recvbuf[iam * m], (void*)&result[iam * m], m, 1, type);
    if (i < np - 1) { memcpy(sendbuf, recvbuf, n * sizeof(float)); }
  }

  if (iam == 0) {
    memcpy(x, (void*)result, m * sizeof(float));
    for (int i = 1; i < np; i++) {
      MPI_Recv((void*)(x + i * m * sizeof(float)), m, type, i, MPI_ANY_TAG, com, &status);
    }
  } else {
    MPI_Send((void*)&result[iam * m], m, type, 0, MPI_ANY_TAG, com);
  }
}

void init_array(void *A, void *x, int n, int m) {
  for (int i = 0; i < n; i++)
    for (int j = 0; j < m; j++)
      A[i][j] = i + j;
  
  for (int i = 0, idx = iam * m; i < m; i++, idx++) {
    x[i] = 1 - (idx % 2) * 2 // 1, -1, 1, -1, ...
  }
}

int sign(int x) {
  if (x % 2 == 0)
    return 1;
  else
    return -1;
}

int test(float *x, int n) {
  if (sign(n) == 1) {
    for (int i = 0; i < n; i++) {
      if (x[i] != -n / 2 + sign(i))
        return 0;
    }
  } else {
    for (int i = 0; i < n; i++) {
      if (x[i] != (n - 1) / 2 + i + sign(i))
        return 0;
    }
  }
  return 1;
}

void main(int argc, char **argv) {
  int iam, np;

  MPI_Init(&argc, &argv);
  MPI_Comm_dup(MPI_COMM_WORLD, &comm);
  MPI_Comm_rank(comm, &iam);
  MPI_Comm_size(comm, &np);

  int m = n / np;
  float A[n][m];
  float *x, *b;

  if (iam == 0) {
    b = (float*)malloc(n * sizeof(float));
    x = (float*)malloc(n * sizeof(float));
  } else {
    b = (float*)malloc(m * sizeof(float));
    x = (float*)malloc(m * sizeof(float));
  }
  // Allocate A and x
  init_array(A, x, n ,m);
  // Allocate b in process 0
  for (int i = 0; i < n; i++)
    b[i] = 1 - (i % 2) * 2;

  linear_map(A, x, b, n, np, iam, MPI_FLOAT, comm);

  if (iam == 0) {
    int is_correct = test(x, n);
    printf("%s!\n", is_correct ? "SUCCESS" : "FAIL");
  }

  MPI_Finalize();

}