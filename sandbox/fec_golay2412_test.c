/*
 * Copyright (c) 2011 Joseph Gaeddert
 * Copyright (c) 2011 Virginia Polytechnic Institute & State University
 *
 * This file is part of liquid.
 *
 * liquid is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * liquid is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with liquid.  If not, see <http://www.gnu.org/licenses/>.
 */

//
//  Golay(24,12) code test
//

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "liquid.internal.h"

#define DEBUG_FEC_GOLAY 1

// P matrix [12 x 12]
unsigned char P[144] = {
    1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0,  1,
    0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1,  1,
    0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 0,  1,
    0, 1, 1, 1, 0, 1, 1, 0, 1, 0, 0,  1,
    1, 1, 1, 0, 1, 1, 0, 1, 0, 0, 0,  1,
    1, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1,  1,
    1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 1,  1,
    0, 1, 1, 0, 1, 0, 0, 0, 1, 1, 1,  1,
    1, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0,  1,
    1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1,  1,
    0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1,  1,

    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  0};

// generator matrix [12 x 24]
unsigned char G[288];
void golay_init_G();

// parity check matrix [12 x 24]
unsigned char H[288];
void golay_init_H();

unsigned int fec_golay2412_encode_symbol(unsigned int _sym_dec)
{
    // validate input
    if (_sym_dec >= (1<<11)) {
        fprintf(stderr,"error, fec_golay2412_encode(), input symbol too large\n");
        exit(1);
    }
    return 0;
}

unsigned int fec_golay2412_decode_symbol(unsigned int _sym_enc)
{
    // validate input
    if (_sym_enc >= (1<<15)) {
        fprintf(stderr,"error, fec_golay2412_decode(), input symbol too large\n");
        exit(1);
    }

    return 0;
}

void print_bitstring(unsigned int _x,
                     unsigned int _n)
{
    unsigned int i;
    for (i=0; i<_n; i++)
        printf("%1u", (_x >> (_n-i-1)) & 1);
}

void matrix2_print(unsigned char * _x,
                   unsigned int _m,
                   unsigned int _n)
{
    unsigned int i;
    unsigned int j;
    for (i=0; i<_m; i++) {
        printf("    ");
        for (j=0; j<_n; j++)
            printf("%c ", _x[i*_n+j] ? '1' : '0');
        printf("\n");
    }
}

// multiply two matrices modulo 2
void matrix2_mul(unsigned char * _x, unsigned int _mx, unsigned int _nx,
                 unsigned char * _y, unsigned int _my, unsigned int _ny,
                 unsigned char * _z, unsigned int _mz, unsigned int _nz)
{
    // ensure lengths are valid
    if (_mz != _mx || _nz != _ny || _nx != _my ) {
        fprintf(stderr,"error: matrix2_mul(), invalid dimensions\n");
        exit(1);
    }

    unsigned int r, c, i;
    for (r=0; r<_mz; r++) {
        for (c=0; c<_nz; c++) {
            unsigned int sum = 0;
            for (i=0; i<_nx; i++) {
                sum += matrix_access(_x,_mx,_nx,r,i) *
                       matrix_access(_y,_my,_ny,i,c);
            }
            matrix_access(_z,_mz,_nz,r,c) = sum % 2;
        }
    }
}

unsigned int vector_weight(unsigned char * _x,
                           unsigned int _n)
{
    unsigned int w = 0;
    unsigned int i;
    for (i=0; i<_n; i++)
        w += _x[i] ? 1 : 0;
    return w;
}

int main(int argc, char*argv[])
{
    // initialize matrices
    golay_init_G();
    golay_init_H();

    printf("P:\n");
    matrix2_print(P,12,12);

    printf("G:\n");
    matrix2_print(G,12,24);

    printf("H:\n");
    matrix2_print(H,12,24);

    unsigned int i;
    
    // error vector
    unsigned char err[24] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        1, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0};

    // arrays
    unsigned char m[12];    // original message
    unsigned char v[24];    // encoded/transmitted message
    unsigned char e[24];    // error vector
    unsigned char r[24];    // received vector
    unsigned char s[12];    // syndrome vector
    unsigned char e_hat[24];// estimated error vector
    unsigned char v_hat[24];// estimated transmitted message
    //unsigned char m_hat[24];// estimated original message

    // compute (random) original message
    for (i=0; i<12; i++)
        m[i] = rand() % 2;
    printf("m (original message):\n");
    matrix2_print(m,1,12);

    // compute encoded/transmitted message: v = m*G
    matrix2_mul(m,1,12, G,12,24, v,1,24);
    printf("v (encoded/transmitted message):\n");
    matrix2_print(v,1,24);

#if 0
    // generate random error vector
    unsigned int e0 = rand() % 24;
    for (i=0; i<24; i++) e[i] = (i==e0) ? 1 : 0;
#else
    // use pre-determined error vector
    for (i=0; i<24; i++) e[i] = err[i];
#endif
    printf("e (error vector):\n");
    matrix2_print(e,1,24);

#if 0
    // from Lin & Costello, p. 128,
    // error locations at indices: 3, 10, 23
    r[ 0] = 1;    r[ 1] = 0;    r[ 2] = 0;    r[ 3] = 0;
    r[ 4] = 0;    r[ 5] = 0;    r[ 6] = 1;    r[ 7] = 1;
    r[ 8] = 0;    r[ 9] = 1;    r[10] = 0;    r[11] = 0;
    r[12] = 1;    r[13] = 1;    r[14] = 0;    r[15] = 0;
    r[16] = 0;    r[17] = 0;    r[18] = 0;    r[19] = 0;
    r[20] = 0;    r[21] = 0;    r[22] = 0;    r[23] = 1;
#else
    // compute received vector: r = v + e
    for (i=0; i<24; i++) r[i] = (v[i] + e[i]) % 2;
    printf("r (received vector):\n");
    matrix2_print(r,1,24);
#endif 

    // compute syndrome vector, s = r*H^T = ( H*r^T )^T
    matrix2_mul(H,12,24, r,24,1, s,12,1);
    printf("s (syndrome vector):\n");
    matrix2_print(s,1,12);

    // compute weight of s
    unsigned int ws = vector_weight(s,12);
    printf("w(s) = %u\n", ws);

    // step 2:
    if (ws <= 3) {
        printf("    w(s) <= 3: estimating error vector as [s, 0(12)]\n");
        // set e_hat = [s 0(12)]
        for (i=0; i<24; i++)
            e_hat[i] = i < 12 ? s[i] : 0;
    } else {
        // step 3: search for p[i] s.t. w(s+p[i]) <= 2
        printf("    searching for w(s + p_i) <= 2...\n");
        unsigned int j;
        unsigned char spj[12];
        int flag3 = 0;
        unsigned int p3_index = 0;
        for (j=0; j<12; j++) {
            for (i=0; i<12; i++)
                spj[i] = (s[i] + P[12*j+i]) % 2;
            unsigned int wj = vector_weight(spj,12);
            printf("    w(s + p[%2u]) = %2u%s\n", j, wj, wj <= 2 ? " *" : "");
            if (wj <= 2) {
                flag3 = 1;
                p3_index = j;
                break;
            }
        }

        if (flag3) {
            // vector found!
            printf("    w(s + p[%2u]) <= 2: estimating error vector as [s+p[%2u],u[%2u]]\n", p3_index, p3_index, p3_index);
            for (i=0; i<12; i++)
                e_hat[i] = (s[i] + P[12*p3_index + i]) % 2;
            for (i=0; i<12; i++)
                e_hat[i+12] = (i == p3_index) ? 1 : 0;

        } else {
            // step 4: compute s*P
            unsigned char sP[12];
            matrix2_mul(s,1,12, P,12,12, sP,1,12);
            printf("s*P:\n");
            matrix2_print(sP,1,12);

            unsigned int wsP = vector_weight(sP,12);
            printf("w(s*P) = %u\n", wsP);

            if (wsP == 2 || wsP == 3) {
                // step 5: set e = [0, s*P]
                printf("    w(s*P) in [2,3]: estimating error vector as [0(12), s*P]\n");
                for (i=0; i<24; i++)
                    e_hat[i] = i < 12 ? 0 : sP[i-12];
            } else {
                // step 6: search for p[i] s.t. w(s*P + p[i]) == 2...

                printf("    searching for w(s*P + p_i) == 2...\n");
                unsigned int j;
                unsigned char sPpj[12];
                int flag6 = 0;
                unsigned int p6_index = 0;
                for (j=0; j<12; j++) {
                    for (i=0; i<12; i++)
                        sPpj[i] = (sP[i] + P[12*j + i]) % 2;
                    unsigned int wj = vector_weight(sPpj,12);
                    printf("    w(s*P + p[%2u]) = %2u%s\n", j, wj, wj == 2 ? " *" : "");
                    if (wj == 2) {
                        flag6 = 1;
                        p6_index = j;
                        break;
                    }
                }

                if (flag6) {
                    // vector found!
                    printf("    w(s*P + p[%2u]) == 2: estimating error vector as [u[%2u],s*P+p[%2u]]\n", p6_index, p6_index, p6_index);
                    for (i=0; i<12; i++)
                        e_hat[i] = (i == p6_index) ? 1 : 0;
                    for (i=0; i<12; i++)
                        e_hat[i+12] = (sP[i] + P[12*p6_index+i]) % 2;
                } else {
                    // step 7: decoding error
                    printf("  **** decoding error\n");
                }
            }
        }
    }

    // step 8: compute estimated transmitted message: v_hat = r + e_hat
    printf("e-hat (estimated error vector):\n");
    matrix2_print(e_hat,1,24);
    for (i=0; i<24; i++) v_hat[i] = (r[i] + e_hat[i]) % 2;
    printf("v-hat (estimated transmitted vector):\n");
    matrix2_print(v_hat,1,24);
    matrix2_print(v,    1,24);

    // compute errors between v, v_hat
    unsigned int num_errors = 0;
    for (i=0; i<24; i++)
        num_errors += v[i] == v_hat[i] ? 0 : 1;
    printf("\n");
    printf("decoding errors : %2u / 24\n", num_errors);

    return 0;
}

// initialize generator matrix as G = [P I_12]
void golay_init_G()
{
    unsigned int i;
    unsigned int j;
    for (i=0; i<12; i++) {
        for (j=0; j<12; j++)
            G[i*24 + j] = P[i*12 + j];

        for (j=0; j<12; j++)
            G[i*24 + j + 12] = (i==j) ? 1 : 0;
    }
}

// initialize generator matrix as H = [I_12 P]
void golay_init_H()
{
    unsigned int i;
    unsigned int j;
    for (i=0; i<12; i++) {
        for (j=0; j<12; j++)
            H[i*24+j] = (i==j) ? 1 : 0;

        for (j=0; j<12; j++)
            H[i*24 + j + 12] = P[i*12 + j];
    }
}

