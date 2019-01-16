#include <stdio.h>
#include <math.h>
#include <string.h>
#include <stdlib.h>

#define FFT_SIZE 32
#define SUCCESS 0
#define FAILED  1

#define PI 3.141592653

typedef struct complex{
    double real;
    double imag;
}complex;

/*
 * complex operation
 */

static complex _add(complex num1, complex num2){
   complex _t;
    _t.real = num1.real+num2.real;
    _t.imag = num1.imag+num2.imag;
    return _t;
}

static complex _mul(complex num1, complex num2){
    complex _t;
    _t.real = (num1.real*num2.real)-(num1.imag*num2.imag);
    _t.imag = (num1.real*num2.imag)+(num1.imag*num2.real);
    return _t;
}

static double _abs(complex num) {
    return sqrt(num.real*num.real+num.imag*num.imag);
}

/*A,B,C=(WN)^k*/
static complex X_NODE(complex num1, complex num2, int N, int k, unsigned char first_or_second){
    complex _t;
    complex Wnk = {
            .real = cos((2*PI*k)/N),
            .imag = -sin((2*PI*k)/N),
    };
    complex CB = _mul(num2, Wnk);
    if(first_or_second == 1) {
        _t = _add(num1, CB);
    }else {
        //CB = -CB
        CB.real = -CB.real;
        CB.imag = -CB.imag;
        _t = _add(num1, CB);
    }
    return _t;
}

/* DIT_FFT_ALGORITHM
 * input: [double*:input] length = FFT_SIZE
 * output: [complex*:output] length = FFT_SIZE
 * abs: [double*:abs] length = FFT_SIZE
 * return: void
 */

void dit_fft(double* input, complex* output, double* abs, int8_t layer){
    /*invert sequence*/
    complex inv_input[FFT_SIZE];
    unsigned char bitmark[layer];

    /*loop param:_i, _j, inv_i, _t*/
    int _i, _j, inv_i;

    for(_i=0; _i<FFT_SIZE; _i++) {
        inv_i=0;
        for (_j = 0; _j < layer; _j++)
            bitmark[_j] = (unsigned char) (_i & (0x01 << _j) ? 1 : 0);
        for (_j = layer-1; _j >=0; _j--){
            inv_i |= bitmark[_j]?0x01<<(layer-1-_j):0x00;
        }

        inv_input[inv_i].real = input[_i];
        inv_input[inv_i].imag = 0;
    }

    /*dit_fft loop*/

    /*initialize 2 result container*/
    complex *const result = inv_input;
    complex _result[FFT_SIZE] = {0};

    /*FFT_LAYER_NUMBER*/
    const int8_t LAYER = layer;
    int select = -1;
    /*loop param: _i = current layer, _j = base*/
    int8_t _offset;
    int _addr1, _addr2;
    for(_i=1; _i<=LAYER; _i++){
        /*select in each layer*/
        printf("LAYTER:%d\n", _i);

        for(_j=0; _j<pow(2,LAYER-_i); _j++) {
            /*select N_DFT*/

            for(_offset=0; _offset<pow(2, _i-1); _offset++){
                /*select X_NODE*/

                _addr1 = _j * (int) pow(2, _i) + _offset;
                _addr2 = _addr1 + (int) pow(2, _i - 1);
                
                if(select == -1) {
                    _result[_addr1] = X_NODE(result[_addr1], result[_addr2],
                                             (int) pow(2, _i), _offset, 1);
                    _result[_addr2] = X_NODE(result[_addr1], result[_addr2],
                                             (int) pow(2, _i), _offset, 2);
                }else{
                    result[_addr1] = X_NODE(_result[_addr1], _result[_addr2],
                                            (int) pow(2, _i), _offset, 1);
                    result[_addr2] = X_NODE(_result[_addr1], _result[_addr2],
                                            (int) pow(2, _i), _offset, 2);
                }
            }

        }
        //select flip over
        select = -select;
    }

    if(select == -1)
        memcpy(output, result, sizeof(complex[FFT_SIZE]));
    else
        memcpy(output, _result, sizeof(complex[FFT_SIZE]));

    for(_i=0; _i<FFT_SIZE; _i++)
        abs[_i] = _abs(output[_i]);
}


int main() {
    double _input[FFT_SIZE] = {
            2, 4, 6, 8, 10, 2, 4, 6, 8, 10, \
            2, 4, 6, 8, 10, 2, 4, 6, 8, 10, \
            2, 4, 6, 8, 10, 2, 4, 6, 8, 10, \
            1, 2};
    complex _output[FFT_SIZE];
    double _abs[FFT_SIZE];

    dit_fft(_input, _output, _abs, 5);
    int _i;
    for(_i=0; _i<FFT_SIZE; _i++)
        printf("%d: %lf\n", _i, _abs[_i]);

    return SUCCESS;
}