#include "stdio.h"  
int partition(int arr[], int l, int r) {
    int k = l, pivot = arr[r];
    for (int i = l; i < r; ++i)
        if (arr[i] <= pivot){
		int tmp = arr[i];
		arr[i] = arr[k];
		arr[k] = tmp;
		k++;
	}
// std::swap(arr[i], arr[k++]);
   // std::swap(arr[k], arr[r]);
	int t = arr[k];
	arr[k] = arr[r];
	arr[r] = t;
    return k;
}
void quicksort(int arr[], int l, int r) {
    if (l < r) {
        int pivot = partition(arr, l, r);
        quicksort(arr, l, pivot-1);
        quicksort(arr, pivot+1, r);
    }
}
int main(int argc, char* argv[])  
{  
	int looptimes = 1;
    //show(array, maxlen, 0, maxlen-1); /* 打印初始顺序 */  
    //show(array, maxlen, 0, maxlen-1); /* 打印最终结果 */  
    //getchar();   
	for(int i = 0;i < looptimes;i++){
    int array[10] = {49, 38, 65, 97, 48, 13, 27, 11, 56, 45};  
    int maxlen = sizeof(array) / sizeof(int);  
    quicksort(array, 0, maxlen-1);  }
    return 0;  
}  
