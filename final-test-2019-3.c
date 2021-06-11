struct newstruct{
  int m[3];
  float a[2];
  char c[5];
}

void main(int argc, char **argv) {
  MPI_Comm com;
  int iam;
  struct newstruct x[10];
  MPI_Datatype newtype;
  MPI_Statis status;

  MPI_Init(&argc, &argv);

  MPI_Comm_dup(MPI_COMM_WORLD, &com);
  MPI_Comm_rank(com, &iam);

  MPI_Datatype newelement;
  int blocklens[] = {3, 2, 5};
  MPI_Aint indices[] = {0, sizeof(x[0].m), sizeof(x[0].m) + sizeof(x[0].a)}
  MPI_Datatype oldtypes[] = {MPI_INTEGER, MPI_FLOAT, MPI_CHARACTER};
  MPI_Type_struct(3, blocklens, indices, oldtypes, &newelement);
  MPI_Type_vector(10, newelement, &newtype);

  MPI_Type_commit(&newtype);

  if (iam == 0) {
    // Initialize array x
    // ...
    MPI_Send(x, 10, newtype, 1, 0, com);
  } else {
    MPI_Recv(x, 10, newtype, 0, 0, com, &status);
  }

  MPI_Type_free(&newtype);

  MPI_Finalize();
  return;
}