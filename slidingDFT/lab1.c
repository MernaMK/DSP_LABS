/* Coefficient calculated with sampling frequency of 48kHz and cutoff at 8kHz. See included Matlab file*/
/*This program reads data in from the left and right channels and then sends the data out to the respective outputs. */

// #include "C6416_DSK_FIRcfg.h"
#define CHIP_6416 1

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
float R_DFT_real[128];
float R_DFT_img[128];

void DFT(void)
{
	float tempReal,tempImg;
	int k,n;
	for ( k = 0; k < 128; k++) /* 128 taps */
    {
		tempReal=0.0f;
		tempImg=0.0f;
		for ( n = 0; n < 128; n++)
		{
			float angle = 2.0f * 3.14159265358979f * (float)n * (float)k /128.0f;
			tempReal +=R_in[n] * cosf(angle);
        	tempImg -= R_in[n] * sinf(angle);
    	}
		R_DFT_real[k]=tempReal;
		R_DFT_img[k]=tempImg;
    }
}
short slidingDFT(short input)
{
   
}

short dominantFreq(void)
{

}


void main()
{
  DSK6416_AIC23_CodecHandle hCodec;
  Int16 OUT_L, OUT_R;
  Uint32 IN_L, IN_R;
  /*	Uint32 b0;
    Uint32 b1;
    Uint32 b2;
    Uint32 b3;*/

  /* Initialize the board support library, must be called first */
  DSK6416_init();
  DSK6416_LED_init();
  DSK6416_DIP_init();

  /* Start the codec */
  hCodec = DSK6416_AIC23_openCodec(0, &config);

  // Set codec frequency to 48KHz (44kHz does not seem to work!)
  DSK6416_AIC23_setFreq(hCodec, DSK6416_AIC23_FREQ_48KHZ);
  DSK6416_LED_off(0);
  DSK6416_LED_off(1);
  DSK6416_LED_off(2);
  DSK6416_LED_off(3);

  while (1)
  {

    // Read sample from the left channel
    while (!DSK6416_AIC23_read(hCodec, &IN_L))
      ;

    // Read sample from the right channel
    while (!DSK6416_AIC23_read(hCodec, &IN_R))
      ;

    if ((DSK6416_DIP_get(0) == 0))
    {
      DSK6416_LED_on(0);
    }
    else if ((DSK6416_DIP_get(1) == 0))
    {

      DSK6416_LED_on(1);
    }
    else if ((DSK6416_DIP_get(2) == 0))
    {

      DSK6416_LED_on(2);
    }
    else if ((DSK6416_DIP_get(3) == 0))
    {

      DSK6416_LED_on(3);
    }

    /* Send the FILTERED sample to the left channel */
    while (!DSK6416_AIC23_write(hCodec, OUT_L))
      ;

    /* Send a NON FILTERED sample to the right channel */
    while (!DSK6416_AIC23_write(hCodec, OUT_R))
      ;
  }

  /* Close the codec */
  DSK6416_AIC23_closeCodec(hCodec);
}
