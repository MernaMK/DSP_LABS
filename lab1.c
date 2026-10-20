/* Coefficient calculated with sampling frequency of 48kHz and cutoff at 8kHz. See included Matlab file*/
/*This program reads data in from the left and right channels and then sends the data out to the respective outputs. */

// #include "C6416_DSK_FIRcfg.h"
#define CHIP_6416 1

#define N_DFT 128

#include <math.h>

#include <csl.h>
#include "include/dsk6416.h"
#include "include/dsk6416_aic23.h"
#include "include/dsk6416_led.h"
#include "include/dsk6416_dip.h"

/* Codec configuration settings */
DSK6416_AIC23_Config config = {
    0x0017, /* 0 DSK6416_AIC23_LEFTINVOL  Left line input channel volume */
    0x0017, /* 1 DSK6416_AIC23_RIGHTINVOL Right line input channel volume */
    0x01f9, /* 2 DSK6416_AIC23_LEFTHPVOL  Left channel headphone volume */
    0x01f9, /* 3 DSK6416_AIC23_RIGHTHPVOL Right channel headphone volume */
    0x0011, /* 4 DSK6416_AIC23_ANAPATH    Analog audio path control */
    0x0000, /* 5 DSK6416_AIC23_DIGPATH    Digital audio path control */
    0x0000, /* 6 DSK6416_AIC23_POWERDOWN  Power down control */
    0x0043, /* 7 DSK6416_AIC23_DIGIF      Digital audio interface format */
    0x0081, /* 8 DSK6416_AIC23_SAMPLERATE Sample rate control */
    0x0001  /* 9 DSK6416_AIC23_DIGACT     Digital interface activation */
};

short R_in[128]; /* Input samples R_in[0] most recent, R_in[127] oldest */

float R_DFT_real[128];//real part of X(k)
float R_DFT_img[128];//imaginary part of X(k)


float W_real[128]; /* Initialize the rotation factor */
float W_img[128];
void DFT(void)
{
	float tempReal,tempImg;//accumelators

	int k,n;

	for ( k = 0; k < 128; k++) //loop to get the DFT of each bin(k)
    {
		tempReal=0.0f;
		tempImg=0.0f;

		for ( n = 0; n < 128; n++)//loop to perform summation(segma)
		{
			float angle = 2.0f * 3.14159265358979f * (float)n * (float)k /128.0f;// 2*pi*n*k/N

			// x(n) * ( cos(angle) - j sin (angle) )
			tempReal +=R_in[n] * cosf(angle);
        	tempImg -= R_in[n] * sinf(angle);
    	}
		R_DFT_real[k]=tempReal;
		R_DFT_img[k]=tempImg;
    }
}

void set_up(void)
{
 int i;
 for ( i = 0; i < N_DFT; i++)
    {
        W_real[i] = cosf(2.0f * 3.14159265358979f * (float)i / (float)N_DFT);
        W_img[i] = sinf(2.0f * 3.14159265358979f * (float)i / (float)N_DFT);
    }
}

short slidingDFT(short input)
{
	int k;
    float oldest, tempReal, tempImg;
 
    /* sample leaving the window, then shift in the new one */
    oldest = (float)R_in[N_DFT - 1];
    for (k = N_DFT - 1; k > 0; k--)
    {
        R_in[k] = R_in[k - 1];
	}

    R_in[0] = input;
 
    for (k = 0; k < N_DFT; k++)
    {
        tempReal = R_DFT_real[k] - oldest + (float)input;
        tempImg = R_DFT_img[k];
 
        /* complex multiply by (W_real + j*W_img) */
        R_DFT_real[k] = tempReal * W_real[k] - tempImg * W_img[k];
        R_DFT_img[k] = tempReal * W_img[k] + tempImg * W_real[k];
    }
 
    return 0; /* result is stored in R_DFT_real / R_DFT_img */ 
}


short dominantFreq(void)
{
    int k;
    int dominant_bin = 0;
    float max_mag = -1.0f;

    // Search through the positive frequency bins (0 to N/2)
    for (k = 0; k < (128 / 2); k++)
    {
        // Compute magnitude squared: |X(k)|^2 = real^2 + imag^2
        float mag_sq = (R_DFT_real[k] * R_DFT_real[k]) + (R_DFT_img[k] * R_DFT_img[k]);
        
        if (mag_sq > max_mag)
        {
            max_mag = mag_sq;
            dominant_bin = k;
        }
    }

    // Turn off all LEDs first
    DSK6416_LED_off(0);
    DSK6416_LED_off(1);
    DSK6416_LED_off(2);
    DSK6416_LED_off(3);

    // Map the dominant bin range to the 4 LEDs on the DSK6416 board
    if (dominant_bin < 16)
    {
        DSK6416_LED_on(0);
    }
    else if (dominant_bin < 32)
    {
        DSK6416_LED_on(1);
    }
    else if (dominant_bin < 48)
    {
        DSK6416_LED_on(2);
    }
    else
    {
        DSK6416_LED_on(3);
    }

    return (short)dominant_bin;
}

void main(void)
{
DSK6416_AIC23_CodecHandle hCodec;
Int16 OUT_L, OUT_R;
Uint32 IN_L, IN_R;

// Initialize the board
DSK6416_init();
DSK6416_LED_init();
DSK6416_DIP_init();

// Initialize DFT coefficients
set_up();

// Open the audio codec
hCodec = DSK6416_AIC23_openCodec(0, &config);
DSK6416_AIC23_setFreq(hCodec, DSK6416_AIC23_FREQ_48KHZ);

// Turn off all LEDs
DSK6416_LED_off(0);
DSK6416_LED_off(1);
DSK6416_LED_off(2);
DSK6416_LED_off(3);

while (1)
{
// Read audio samples
while (!DSK6416_AIC23_read(hCodec, &IN_L))
;

while (!DSK6416_AIC23_read(hCodec, &IN_R))
;

// DIP switch 0: Pass-through mode
if (DSK6416_DIP_get(0) == 0)
{
OUT_L = (Int16)IN_L;
OUT_R = (Int16)IN_R;

// Turn off all LEDs
DSK6416_LED_off(0);
DSK6416_LED_off(1);
DSK6416_LED_off(2);
DSK6416_LED_off(3);
}
// DIP switch 1: DFT analysis mode
else if (DSK6416_DIP_get(1) == 0)
{
// Update DFT and find the dominant frequency
slidingDFT((short)IN_L);
dominantFreq();

// Send the original audio to the output
OUT_L = (Int16)IN_L;
OUT_R = (Int16)IN_R;
}
else
{
// Mute the audio when no switch is pressed
OUT_L = 0;
OUT_R = 0;

// Turn off all LEDs
DSK6416_LED_off(0);
DSK6416_LED_off(1);
DSK6416_LED_off(2);
DSK6416_LED_off(3);
}

// Write samples to the audio output
while (!DSK6416_AIC23_write(hCodec, OUT_L));

while (!DSK6416_AIC23_write(hCodec, OUT_R));
}
}
