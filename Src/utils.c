/*
 * utils.c
 *
 *  Created on: 22 sty 2021
 *      Author: NeghM
 */

#include <stdint.h>
#include <stdlib.h>

void insertionSort(uint16_t arr[], int n)
{
    int i, key, j;
    for (i = 1; i < n; i++)
    {
        key = arr[i];
        j = i - 1;

        /* Move elements of arr[0..i-1], that are
        greater than key, to one position ahead
        of their current position */
        while (j >= 0 && arr[j] > key)
        {
            arr[j + 1] = arr[j];
            j = j - 1;
        }
        arr[j + 1] = key;
    }
}

int countDigits(uint32_t n) {
	int count = 0;
	 while (n != 0) {
	        n /= 10;     // n = n/10
	        ++count;
	    }
	 return count;
}



typedef union {
    long    L;
    float   F;
}       LF_t;

int ftoa(float f, char * outbuf)
    {
    long mantissa, int_part, frac_part;
    short exp2;
    LF_t x;
    char *p;

    if (f == 0.0)
        {
        outbuf[0] = '0';
        outbuf[1] = '.';
        outbuf[2] = '0';
        outbuf[3] = 0;
        return 0;
        }
    x.F = f;

    exp2 = (unsigned char)(x.L >> 23) - 127;
    mantissa = (x.L & 0xFFFFFF) | 0x800000;
    frac_part = 0;
    int_part = 0;

    if (exp2 >= 31)
        {
        return 1;
        }
    else if (exp2 < -23)
        {
        return -1;
        }
    else if (exp2 >= 23)
        int_part = mantissa << (exp2 - 23);
    else if (exp2 >= 0)
        {
        int_part = mantissa >> (23 - exp2);
        frac_part = (mantissa << (exp2 + 1)) & 0xFFFFFF;
        }
    else /* if (exp2 < 0) */
        frac_part = (mantissa  & 0xFFFFFF) >> -(exp2 + 1);

    p = outbuf;

    if (x.L < 0)
        *p++ = '-';

    if (int_part == 0)
        *p++ = '0';
    else
        {

        itoa(int_part, p, 10);
        while (*p)
            p++;
        }
    *p++ = '.';

    if (frac_part == 0)
        *p++ = '0';
    else
        {
        char m, max;

        max = sizeof (outbuf) - (p - outbuf) - 1;
        if (max > 7)
            max = 7;
        /* print BCD */
        for (m = 0; m < max; m++)
            {
            /* frac_part *= 10; */
            frac_part = (frac_part << 3) + (frac_part << 1);

            *p++ = (frac_part >> 24) + '0';
            frac_part &= 0xFFFFFF;
            }
        /* delete ending zeroes */
        for (--p; p[0] == '0' && p[-1] != '.'; --p)
            ;
        ++p;
        }
    *p = 0;


    return 0;
    }


float stof(const char* s){
  float rez = 0, fact = 1;
  if (*s == '-'){
    s++;
    fact = -1;
  };
  for (int point_seen = 0; *s; s++){
    if (*s == '.'){
      point_seen = 1;
      continue;
    };
    int d = *s - '0';
    if (d >= 0 && d <= 9){
      if (point_seen) fact /= 10.0f;
      rez = rez * 10.0f + (float)d;
    };
  };
  return rez * fact;
};
