#include<stdio.h>
int main(){
	int a=9;
	__asm volatile("MOV R0,%0" ::"r"(a));
}
