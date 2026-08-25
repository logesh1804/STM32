#include<stdio.h>
int main(){
	int a=4;
	__asm volatile("ADD %0,%0,#56":"=r"(a));
	printf("%d",a);

}
