#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
int main(){
   int pid1=fork();
   printf("**1**\n");
   if(pid1==0){int pid2=fork();printf("**2**\n");}
   else printf("**3**\n");
   return 0;
}
