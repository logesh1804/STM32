#include<stdio.h>
int main(){
	int a=2;
	int b=3;
	int addition;
	__asm volatile ("ADD %0,%1,%2":"=r"(addition):"r"(a),"r"(b));
	printf("%d",addition);

}
