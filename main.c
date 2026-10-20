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