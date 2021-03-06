/**************************************************************/
/* latsieve.c                                                 */
/* Copyright 2004, Chris Monico.                              */
/**************************************************************/
/*  This file is part of GGNFS.
*
*   GGNFS is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 2 of the License, or
*   (at your option) any later version.
*
*   GGNFS is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with GGNFS; if not, write to the Free Software
*   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ggnfs.h"





int reduceBasis(s32 *v1, s32 *v2)
/*********************************************************/
/* Input: Two vectors v1, v2\in Z^2.                     */
/* Output: v1 and v2 will be modified to be short vectors*/
/*   spanning the lattice generated by the original      */
/*   v1 & v2. They will also satisfy |v1| >= |v2|.       */
/*********************************************************/
{ s32   x1=v1[0], y1=v1[1], x2=v2[0], y2=v2[1], q;
  double s;

  q=1;
  while (q != 0) {
    s = x2*(double)x2 + y2*(double)y2;
    q = (s32)((x1*(double)x2 + y1*(double)y2)/s);
    x1 -= q*x2; y1 -= q*y2;

    s = x1*(double)x1 + y1*(double)y1;
    q = (s32)((x1*(double)x2 + y1*(double)y2)/s);
    x2 -= q*x1; y2 -= q*y1;
  }
  if ( (x2*(double)x2 + y2*(double)y2) > s) {
    v1[0]=x2; v1[1]=y2;
    v2[0]=x1; v2[1]=y1;
  } else {
    v1[0]=x1; v1[1]=y1;
    v2[0]=x2; v2[1]=y2;
  }
  return 0;  
}

s32 debugA1, debugB1, debugA2, debugB2;

/**********************************************************************/
int doSieve(short *A, s32 C, s32 E0, s32 E1, s32 *U, s32 numU, char *logs)
/**********************************************************************/
/* This is the sieve-by-vectors function. A is an already initialized */
/* array for the rectangle C0<=c<=C1, E0<=e<=E1.                      */
/* U contains the vectors of the `prepared primes', and there are     */
/* numU of them. `logs' has the logs of the corresponding primes.     */
/**********************************************************************/
{ s32   i;
  s32   x1, y1, x2, y2, s0, s1, s, t0, t1, t;
  s32   c, e, Clength = 2*C+1, cC;
  s32   dOff, dOff_even;
  register s32 offset;
  s32   *tmpU;
  s32   ds0, ds1, dt, d, tmp1, tmp2;
  double z1, z2, z3, z4, dsx, dsy;
  char logP;


  tmpU = U;
  for (i=0; i<numU; i++) {
    x1 = *tmpU++; y1 = *tmpU++;
    x2 = *tmpU++; y2 = *tmpU++;
    logP = logs[i];

    /* Now we have to find all linear combinations of (x1,y1) and (x2,y2) */
    /* that are in the given rectangle, then sieve them out.              */
    
    /* This can still be sped up quite a bit. At the very least, 
       some of the common operations can be factored out.
    */
    d = x1*y2 - x2*y1;
    tmp1 = y2*C;
    if (x2 > 0) {
      s0 = (-tmp1 - x2*E1)/d;
      s1 = (tmp1 - x2*E0)/d;
      cC=C;
    } else {
      s0 = (-tmp1 - x2*E0)/d;
      s1 = (tmp1 - x2*E1)/d;
      cC=-C;
    }
    if (s1 < s0) {
      d=s0; s0=s1; s1=d;
    }    
//    tmp1=s0*y1; tmp2=s0*x1;
//    if ((s0 > 0x0000FFFF) || (y1 > 0x0000FFFF) || (x1 > 0x0000FFFF)) {
//      printf("Possible overflow condition!\n");
//    }
    z1 = (E0 - s0*(double)y1)/(double)y2;
    z2 = (-cC - s0*(double)x1)/(double)x2;
    z3 = (E1 - s0*(double)y1)/(double)y2;
    z4 = (cC - s0*(double)x1)/(double)x2;
    dsx = x1/(double)x2;
    dsy = y1/(double)y2;
    dOff = Clength*y2 + x2;
    dOff_even = dOff+dOff;
    for (s=s0; s<=s1; s++) {
      t0 = (s32)ceil(MAX(z1, z2));
      z1 -= dsy; z2 -= dsx;
      t1 = (s32)floor(MIN(z3,z4));
      z3 -= dsy; z4 -= dsx;

      c = s*x1 + t0*x2;
      e = s*y1 + t0*y2;
      offset = Clength*(e-E0) + c + C;
      if (s%2==0) {
        /* If s is even, we need only consider odd values of t. */
        if (t0%2==0) {
          t0++; offset += dOff; 
        }
        for (t=t0; t<=t1; t+=2) {
          A[offset] += logP;
          offset += dOff_even;
        }
      } else {
        for (t=t0; t<=t1; t++) {
          A[offset] += logP;
          offset += dOff;
        }
      }
    }
  }        
    
  return 0;
}

/******************************************************************/
int doSieveE(short *A, s32 C0, s32 C1, s32 E0, s32 E1, s32 *U, s32 numU, char *logs)
/******************************************************************/
/* This is the sieve function for exceptional primes.             */
/* A is an already initialized array for the rectangle            */
/*         C0<=c<=C1, E0<=e<=E1.                                  */
/* U contains the vectors of the `prepared primes', and there are */
/* numU of them. `logs' has the logs of the corresponding primes. */
/******************************************************************/
{ s32 i;
  s32 c1, e1, c2, e2;
  s32 e, c, offset, dOff;
  char logP;

  for (i=0; i<numU; i++) {
    c1 = U[4*i]; e1 = U[4*i+1];
    c2 = U[4*i+2]; e2 = U[4*i+3];
    logP = logs[i];

//printf("Sieving with exceptional prime: basis is (%ld, %ld), (%ld, %ld), logP = %d\n",c1,e1,c2,e2, logP);

    if (c1==1) {
      /* Then the vectors are (1,0), (0,p). */
//      e = E0 + ((e2-E0)%e2);
      e = E0 + (e2+ ((e2-E0)%e2))%e2;
      offset = (C1-C0+1)*(e-E0);
      dOff = (C1-C0+1)*e2;

      while (e <= E1) {
        for (c=C0; c<=C1; c++) 
          A[offset+c-C0] += logP;
        offset += dOff;
        e += e2;
      }
    } else {
      /* The vectors are (p,0) and (0,1). */
      c = C0 + ((c1-C0)%c1);
      while (c <= C1) {
        offset = c - C0;
        dOff = (C1-C0+1);
        for (e=E0; e<=E1; e++) {
          A[offset] += logP;
          offset += dOff;
        }
        c += c1;
      }
    }
  }  
  return 0;
}






//#define _TEST
#ifndef _TEST


#define MAX_R_EXC 500
#define MAX_A_EXC 2000
#define VERYSMALL_SHORT -32767


/************************************************************************/
int latSieve(s32 *candidates, int maxCands, nfs_fb_t *FB,
             s32 qIndex, double sieveArea, s32 cacheSize)    
/***************************************************************************/
/* We will not actually test for smoothness - instead, we will just return */
/* the list of candidates and allow the caller to test them. They will be  */
/* returned as: (a_i, b_i) = (candidates[2*i], candidates[2*i+1]).         */
/* Return value: negative on error. Otherwise, the number of candidates.   */
/***************************************************************************/
/* This function should be modified, to accept an area instead of a range  */
/* on the c, d values. It should compute the range of values from the area */
/* after we've obtained (short) generators for the lattice.                */
/***************************************************************************/
{ s32 q, s;           /* Special q prime. */
  char logQ;
  s32 a1, b1, a2, b2; /* Short vectors spanning the lattice L_q. */
  s32 v1[2], v2[2];   /* temp garbage. */
  short *Array, entry, ratInit, normApprox;
  s32 C, E;   /* Defining the total rectangle in the (c,e)-plane to be sieved. */
  s32 Clength; 
  s32 E0, E1; /* A particular sub-rectangle being sieved. */
  int  numPieces, pieceESize; /* Number of sub-rectangles and the size. */
  s32 *primeVec; /* Primes prepared for sieving (Master index) */
  char *logs, *rlogs, *alogs, *rElogs, *aElogs; /* Logs of those primes. */
  s32 *rU, *aU, numRU, numAU; /* Ordinary primes to be vector-sieved with. */
  s32 *rExc, *aExc, numRExc, numAExc; /* `Exceptional primes'. */
  s32 numVecs, rfbSize, afbSize;
  s32 p, r, t1, t2, c, e; /* temp stuff. */
  s32 rD, aD; /* Crossover from sieve-by-rows to sieve-by-vectors. */
  s32 i, j, a, b, g, index;
  int  numCands=0, rfb_log_ff, afb_log_ff;

  rfb_log_ff = (int)(FB->rfb_lambda * fplog(FB->rfb[FB->rfb_size-1], FB->log_rlb));
  afb_log_ff = (int)(FB->afb_lambda * fplog(FB->afb[FB->afb_size-1], FB->log_alb));

#ifdef Q_FROM_RFB
  if (qIndex >= FB->rfb_size) {
    fprintf(stderr, "latSieve() Error - invalid qIndex!\n");
     return -1;
  }
  q = FB->rfb[2*qIndex]; s = FB->rfb[2*qIndex+1]; logQ = FB->rfb_log[qIndex];
  rfbSize = qIndex; afbSize = FB->afb_size;
#else
  if (qIndex >= FB->afb_size) {
    fprintf(stderr, "latSieve() Error - invalid qIndex!\n");
     return -1;
  }
  q = FB->afb[2*qIndex]; s = FB->afb[2*qIndex+1]; logQ = FB->afb_log[qIndex];
  rfbSize = FB->rfb_size; afbSize = qIndex;
#endif

  v1[0]=q; v1[1]=0;
  v2[0]=s; v2[1]=1;
  reduceBasis(v1, v2);
  a1=v1[0]; b1=v1[1]; a2=v2[0]; b2=v2[1];
  if (b1 < 0) { a1=-a1; b1=-b1;}
  if (b2 < 0) { a2=-a2; b2=-b2;}
//printf("For (q,s)=(%ld,%ld), L_q has basis: (%ld, %ld), (%ld, %ld)\n",q,s,a1,b1,a2,b2);

  /************************************************************************/
  /* We should compute the shape of the rectangle in the (c,e) plane.     */
  /* That is, assume a fixed amount of memory A, and find the rect.       */
  /* defined by -C<=c<=C, 0 < e <= E so that the average resulting 'b'    */
  /* value, |c*b1 + e*b2|, is minimal (or close to it).                   */
  /* Equivalently, minimize:                                              */
  /*  sum_R |c*b1 + e*b2|, where the sum is over all -C<=c<=C, 0 < e <= E */
  /* subject to the constraint that (2C+1)E = A.                          */
  /************************************************************************/
  /* We will approximate the solution to the above constrained optimization */
  /* problem later. But I think this should be pretty close (or even correct). */
  C = (s32)1*(sqrt(sieveArea*b2/(double)(2.0+2.0*b1)));

//  if (C<1000) C = 1000; /* Arbitrary, but seems reasonable. */
  E = (s32)(sieveArea/C);
  Clength = 2*C+1;
  numPieces = (int)((sizeof(short)*sieveArea/cacheSize));
  if (numPieces*cacheSize < sieveArea)
    numPieces++;
  pieceESize = (E/numPieces);
//if (numPieces > 1) 
// printf("q=(%ld, %ld), C=[%ld, %ld], E=%ld, numPieces=%d\n",q,s,-C,C,E,numPieces);
  

  Array = (short *)malloc(pieceESize*(Clength)*sizeof(short));


  /*********************************/
  /*    Prepare the primes.        */
  /*********************************/
  primeVec = (s32 *)malloc((rfbSize+afbSize + MAX_R_EXC + MAX_A_EXC)*4*sizeof(s32));
  logs = (char *)malloc((rfbSize+afbSize + MAX_R_EXC + MAX_A_EXC)*sizeof(char));
  rExc = &primeVec[4*(rfbSize+afbSize)];
  aExc = &primeVec[4*(rfbSize+afbSize + MAX_R_EXC)];
  
  rElogs = &logs[rfbSize+afbSize];
  aElogs = &logs[rfbSize+afbSize + MAX_R_EXC];
  numVecs = numRExc = numAExc = 0;
  rD = aD = 0; /* Arbitrary for now, but should be fixed later, when we can treat
                  the small primes differently. */

  for (i=rD; i<rfbSize; i++) {
    p = FB->rfb[2*i]; r = FB->rfb[2*i+1];

    t2 = (a2 - mulmod32(r, b2, p) + p)%p;
    t1 = (mulmod32(r, b1, p) - a1%p + p)%p;
    if (t1&&t2) {
      primeVec[4*numVecs]=p; primeVec[4*numVecs+1]=0;
      primeVec[4*numVecs+2] = mulmod32(t2, inverseModP(t1, p), p); primeVec[4*numVecs+3]=1;
      reduceBasis(&primeVec[4*numVecs], &primeVec[4*numVecs+2]);
      if (primeVec[4*numVecs+1]<0) { primeVec[4*numVecs]=-primeVec[4*numVecs]; primeVec[4*numVecs+1]=-primeVec[4*numVecs+1];}
      if (primeVec[4*numVecs+3]<0) { primeVec[4*numVecs+2]=-primeVec[4*numVecs+2]; primeVec[4*numVecs+3]=-primeVec[4*numVecs+3];}
      logs[numVecs++] = FB->rfb_log[i];
    } else if (t1==0) {
      rExc[4*numRExc] = 1; rExc[4*numRExc+1] = 0;
      rExc[4*numRExc+2] = 0; rExc[4*numRExc+3] = p;
      rElogs[numRExc++] = FB->rfb_log[i];
    } else {
      rExc[4*numRExc] = p; rExc[4*numRExc+1] = 0;
      rExc[4*numRExc+2] = 0; rExc[4*numRExc+3] = 1;
      rElogs[numRExc++] = FB->rfb_log[i];
    }
  }
  rU = primeVec; numRU = numVecs;
  rlogs = logs;

  for (i=aD; i<afbSize; i++) {
    p = FB->afb[2*i]; r = FB->afb[2*i+1];

    t2 = (a2 - mulmod32(r, b2, p) + p)%p;
    t1 = (mulmod32(r, b1, p) - a1%p + p)%p;
    if (t1&&t2) {
      primeVec[4*numVecs]=p; primeVec[4*numVecs+1]=0;
      primeVec[4*numVecs+2] = mulmod32(t2, inverseModP(t1, p), p); primeVec[4*numVecs+3]=1;
      reduceBasis(&primeVec[4*numVecs], &primeVec[4*numVecs+2]);
      if (primeVec[4*numVecs+1]<0) { primeVec[4*numVecs]=-primeVec[4*numVecs]; primeVec[4*numVecs+1]=-primeVec[4*numVecs+1];}
      if (primeVec[4*numVecs+3]<0) { primeVec[4*numVecs+2]=-primeVec[4*numVecs+2]; primeVec[4*numVecs+3]=-primeVec[4*numVecs+3];}

      logs[numVecs++] = FB->afb_log[i];
    } else if (t1==0) {
      aExc[4*numAExc] = 1; aExc[4*numAExc+1] = 0;
      aExc[4*numAExc+2] = 0; aExc[4*numAExc+3] = p;
      aElogs[numAExc++] = FB->afb_log[i];
    } else {
      aExc[4*numAExc] = p; aExc[4*numAExc+1] = 0;
      aExc[4*numAExc+2] = 0; aExc[4*numAExc+3] = 1;
      aElogs[numAExc++] = FB->afb_log[i];
    }
  }
  aU = &primeVec[4*numRU]; numAU = numVecs - numRU;
  alogs = &logs[numRU];

  /* Ok - let's sieve. */

  E0 = 1; E1 = pieceESize;
  for (i=0; i<numPieces; i++) {

    /* Initialize the sieve array for the rational sieve. */
    ratInit = (short)(rfb_log_ff - fplog_mpz(FB->m, FB->log_rlb));
    for (j=Clength*(E1-E0+1)-1; j>=0; j--)
      Array[j]=ratInit;

    /* We need to sieve by rows with the first rD primes here! */

    doSieve(Array, C, E0, E1, rU, numRU, rlogs);
    doSieveE(Array, -C, C, E0, E1, rExc, numRExc, rElogs);



    /* Initialize for the algebraic sieve. */
 /*******************************************************************/
 /* This needs to be implemented:                                   */
 /* We should use Lenstra's trick here: Instead of computing the    */
 /* norm for all candidates which survived the rational sieve,      */
 /* simply use the same really rough approximation for all of them. */
 /* Then, when we are pulling candidates out of the array, we can   */
 /* compute the real norm and look at it a bit more closely.        */
 /*******************************************************************/
    normApprox = fplog_evalF(a1+a2, b1+b2, FB) - 7; /* -3 for luck :) */
    entry = rfb_log_ff + logQ - normApprox;
    index=0;
    for (e=E0; e<=E1; e++) {
      for (c=-C; c<=C; c++) {
        if (Array[Clength*(e-E0)+c+C] >= 0) {
          b = c*b1 + e*b2; if (b<0)b=-b;
          if (Array[Clength*(e-E0)+c+C] >= fplog(b, FB->log_rlb))          
            Array[Clength*(e-E0)+c+C] = entry;
          else
            Array[Clength*(e-E0)+c+C] = VERYSMALL_SHORT;
        } else
          Array[Clength*(e-E0)+c+C] = VERYSMALL_SHORT;
      }
    }

    /* We need to sieve by rows with the first aD primes here! */
 
    doSieve(Array, C, E0, E1, aU, numAU, alogs);
    doSieveE(Array, -C, C, E0, E1, aExc, numAExc, aElogs);


    /* Find candidates. */
//    entry = fplog_evalF(a1+a2, b1+b2, FB);
    for (j=Clength*(E1-E0+1)-1; j>=0; j--) {
      if ((Array[j] >= 0)&&(numCands < maxCands)) {
        e = E0 + j/Clength;
        c = -C + (j%Clength);
        a = c*a1 + e*a2;
        b = c*b1 + e*b2;
        if (b<0) { a=-a; b=-b;}
        if ((Array[j] + normApprox - fplog_evalF(a,b, FB))>=0) {
          g = gcd(a,b); 
          if ((g==1)||(g==-1)) {
            candidates[2*numCands] = a;
            candidates[2*numCands+1] = b;
            numCands++;
          }
        }
      }
    }
    /* Prepare to do the next sub-rectangle. */
    E0 += pieceESize; E1 += pieceESize;
  }

  free(Array); 
  free(primeVec); 
  free(logs);
  return numCands;
}

#else

int main()
{ s32 v1[2], v2[2], r, p;

  printf("Enter r,p: ");
  scanf("%ld %ld", &r, &p);
  getLattice(v1, v2, r, p);
  printf("Spanning vectors of L_(r,p):\n");
  printf("v1 = (%ld, %ld)\n", v1[0], v1[1]);
  printf("v2 = (%ld, %ld)\n", v2[0], v2[1]);

  v1[0]=p; v1[1]=0;
  v2[0]=r; v2[1]=1;
  reduceBasis(v1,v2);
  printf("Doing reducedBasis() on (p,0) and (1,r) gives:\n");
  printf("v1 = (%ld, %ld)\n", v1[0], v1[1]);
  printf("v2 = (%ld, %ld)\n", v2[0], v2[1]);
 

} 

#endif
