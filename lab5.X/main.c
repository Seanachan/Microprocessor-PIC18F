#include <xc.h>

extern unsigned int is_prime(unsigned char n);
extern unsigned int count_primes(unsigned int n, unsigned int m);
extern unsigned long mul_extended(int n, int m);

void main(void){
    volatile unsigned int ans = is_prime(79);
  //  volatile unsigned int ans = count_primes(1, 256);
//    volatile long ans = mul_extended(-32768, 32767); //0x004F *0x03E5
    while(1);
    return;
}
