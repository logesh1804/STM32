#include<stdio.h>
int main(){
	int src=1000;
	int dest;
	__asm  volatile(

			"LDR R0, [%1]\n\t"
			"STR R0, [%0]"

			:
			: "r"(&dest), "r"(&src)
			:  "r0", "memory"
			   );
	  printf("%d", dest);
}
