#include <features.h>
#include <stdlib.h>
/* TEA based crypt(), version 0.0 <ndf@linux.mit.edu>
 * It looks like there are problems with key bits carrying through
 * to the encryted data, and I want to get rid of that libc call..
 * This is just so rob could see it ;) */
char *
crypt(const char * key, const char * salt)
{
  /* n is the number of rounds, delta is a golden # derivative,
   k is the key, v is the data to be encrypted. */
  unsigned long v[2], sum=0, delta=0x9e3779b9, n=64, k[4];
  static char rkey[16];
  unsigned char i;

  /* Our constant string will be a string of zeros .. */
  v[0]=v[1]=k[0]=k[1]=k[2]=k[3]=0;
  for(i=0;i<16;i++) rkey[i]=0;
  rkey[0]=*salt;
  rkey[1]=salt[1];
  for (i=0;key[i];i++) rkey[i+1]=key[i];
  memcpy(k, rkey, 4*sizeof(long));

  while (n-->0)  {
    sum += delta;
    v[0] += (v[1]<<4)+k[0] ^ v[1]+sum ^ (v[1]>>5)+k[1];
    v[1] += (v[0]<<4)+k[2] ^ v[0]+sum ^ (v[0]>>5)+k[3];
  }

  *rkey=*salt; rkey[1]=salt[1];

  /* Now we need to unpack the bits and map it to "A-Za-z0-9./" for printing
     in /etc/passwd */
  for (i=2;i<13;i++)
    {
      /* This unpacks the 6 bit data, each cluster into its own byte */
      if (i==8) v[0]|=v[1]>>28;
      rkey[i]=v[(i-2)/6]&0x3F;
      v[(i-2)/6]>>=6;

      /* Now we map to the proper chars */
      if (rkey[i]>=0 && rkey[i]<12) rkey[i]+=46;
      else if (rkey[i]>11 && rkey[i]<38) rkey[i]+=53;
      else if (rkey[i]>37 && rkey[i]<64) rkey[i]+=59;
      else return NULL;
    }

  rkey[13]='\0';
  return rkey;
}
