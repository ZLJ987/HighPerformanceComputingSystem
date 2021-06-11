int my_MPI_Allgather(void *sendbuf, int sendcnt, MPI_Datatype sendtype, void *recvbuf, int recvcnt, MPI_Datatype recvtype, MPI_Comm comm) {
  if (!(sizeof(sendtype) == sizeof(recvtype) && sendcnt == recvcnt))
    return 1;
  int iam, np;
  int prev = (np + iam - 1) % np;
  int next = (iam + 1) % np;
  MPI_Status status;

  MPI_Comm_size(comm, &np);
  MPI_Comm_rank(comm, &iam);

  memcpy((void*)((recvtype*)recvbuf)[iam], sendbuf, sendcnt * sizeof(sendtype));
  for (int i = 1; i < np; i++) {
    MPI_Sendrecv(sendbuf, sendcnt, sendtype, prev, 0, (void*)&((recvtype*)recvbuf)[(iam + i) % np],
      recvcnt, recvtype, next, 0, comm, &status);
    memcpy(sendbuf, (void*)&((recvtype*)recvbuf)[(iam + i) % np], sendcnt * sizeof(sendtype));
  }

  return MPI_SUCCESS;
}