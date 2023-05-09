
Turns out that some Macs (mine and at least one other) come with GCC version 4.2.1, which for some reason had a problem with 
the complex.h calls in the CYGNSS_DDM_Processor code.

To get around this, a C vesion of the Makefile Build was created (in directory "Makefile_Build_C") with the flag (-std=c99) needed
to handle the complex variables in the code. This vesion was tested on a Mac with the above version of GCC (and fftw-3.3.8). 
It also compiled and  ran on Linux with GCC 5.4.0.

Oct 23, 2018. Updated with bug fix by Giuseppe Grieco (8th-Oct-2018) in fftw_acquire.c 

* This routine has been modified by Giuseppe Grieco (8th-Oct-2018)
* in order to solve a bug that was present in the previous version.
* All modifications cab be tracked with the word "GG".
* Modifications have been necessary because of a bug in the projection
* of the satellite PRN codes in the frequency domain for the convolution.
* In the previous version, the projection of the real array sampled_code
* was done through the function fftw_plan_dft_r2c_1d which projects a
* real array in a complex fft array. The problem is that this function
* 0-pads all the values from half size of the real array until the end
* of the array. In order to overcome this problem, the sampled_code
* array has been declared as a complex one, by assining 0 to its 
* imaginary part. The fft has been done through the function
* fftw_plan_dft_1d that is used also to project the echo. This way
* the output fft is hermitian symmetric and results are more reasonable

