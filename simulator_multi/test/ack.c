#include "stdio.h"
int ack(int m,int n){
	if(m==0)
		return n+1;
	if(n==0)
		return ack(m-1,1);
	else
		return ack(m-1,ack(m,n-1));
}
int main(){
	int looptimes = 1;
	for(int i = 0;i<looptimes;i++){
		ack(2,2);
	}
	return 0;
}
