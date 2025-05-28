#include <stdio.h>
#include <syscall.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv){
  int i;
  int arr[4];
  char* ptr;
  arr[0] = atoi(argv[1]);
  arr[1] = atoi(argv[2]);
  arr[2] = atoi(argv[3]);
  arr[3] = atoi(argv[4]);
  printf("%d %d\n", fibonacci(arr[0]), 
  max_of_four_int(arr[0], arr[1], arr[2], arr[3]));
}