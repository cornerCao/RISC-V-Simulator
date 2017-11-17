#include "stdio.h"  
void multi(int N, int A[3][3], int B[3][3], int C[3][3]){
	for(int i=0;i<N;i++){
		for(int j = 0; j < N; j++){
			C[i][j] = 0;
			for(int p = 0;p < N;p++){
				for(int q = 0;q<N;q++){
					C[i][j] += A[i][p] * B[q][j];
				}			}
	//		printf("%d",C[i][j]);
		}
	}
}
int main(){
	int looptimes = 1;
	for(int i =0;i<looptimes;i++){
	int A[3][3] = {{1,2,3},{3,4,5},{5,6,7}};
	int B[3][3] = {{2,3,4},{4,5,6},{6,7,8}};
	int C[3][3];
	multi(3,A,B,C);}
	return 0;
}
