/*
* Copyright (c) 2013, Morgan Quigley, Pieter Abbeel and Scott Gleason
* All rights reserved.
*
* Originally written by Morgan Quigley, Pieter Abbeel and Scott Gleason
* FOTON (fftw) Version created by Scott Gleason, Dec 2015
*
* This file is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 2 of the License, or
* (at your option) any later version.
*
* Fastgps is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with fastgps.  If not, see <http://www.gnu.org/licenses/>.
*
* This routine has been modified by Giuseppe Grieco (8th-Oct-2018)
* Modifications have been made to fix a bug in the projection
* of the satellite PRN codes in the frequency domain for the convolution.
*/

#include "CYGNSS_DDMP_Main.h"
#include <fftw3.h>

//externs
extern SharedVariables ControlVars;
extern char CODE_TABLE[32][1025];

// internal
double fft_output[FFT_MAX_SIZE];
//static double sampled_code[FFT_MAX_SIZE];
static fftw_complex sampled_code[FFT_MAX_SIZE]; // Giuseppe Grieco (8th-Oct-2018)

// fftw variables
fftw_plan FFTWPLAN,FFTWPLAN2,IFFTWPLAN;
static fftw_complex sample_buf[FFT_MAX_SIZE];
static fftw_complex sample_fft[FFT_MAX_SIZE];
static fftw_complex inverse_fft[FFT_MAX_SIZE];
static fftw_complex mult_result[FFT_MAX_SIZE];
static fftw_complex code_fft[MAX_SATELLITES][FFT_MAX_SIZE];

void fft_acquire(char *samples, unsigned samples_len, double dopp_freq)
{
  unsigned s;
  double dd1,dd2;
  unsigned prn_num = ControlVars.PRN;
  double nco_sin2,nco_cos2,i2,q2,sample2;
  double car_phase,car_phase_inc;
  unsigned fft_size = ControlVars.fft_samples;
  //int nco_sin_int,nco_cos_int,i2_int,q2_int,sample_int;

  if((prn_num < 1) || (prn_num > MAX_SATELLITES)){
    return;
  }
//                    tempul = (unsigned long) averaged_fft_output[j][i];

    // 2 because each element of fftw_complex contains 2 doubles
    // could try to skip this to increase speed ... STG XXX
    for (s = 0; s < fft_size; s++){
        sample_buf[s] = 0 + 0*I;
        sample_fft[s] = 0 + 0*I;
        inverse_fft[s] = 0 + 0*I;
        mult_result[s] = 0 + 0*I;
        fft_output[s] = 0;
    }

    ControlVars.carrier_freq = (ControlVars.IF_freq + dopp_freq);
    car_phase_inc = (2*PI*ControlVars.carrier_freq) / ControlVars.sampling_freq;
    car_phase_inc *= (double) ControlVars.DownSampleNumber;

    // load samples, mix down to baseband I and Q, store I and Q in sample buffer
    car_phase = 0;
  	for (s = 0; s < samples_len; s++)
    {

      char sample = samples[s];
      sample2 = (double) sample;

//      sample2 = (double) samples[s];
      nco_sin2 = sin(car_phase);
      nco_cos2 = cos(car_phase);
      i2 =  sample2 * nco_cos2;
      q2 = -sample2 * nco_sin2;
      sample_buf[s] = (double) i2 + (double) q2*I;
      car_phase += car_phase_inc;
      UNWRAP_ANGLE(car_phase);
    }

    // Perform FFT on data samples
    fftw_execute(FFTWPLAN2);

    // Complex conjugate of sample FFT
    for (s = 0; s < fft_size; s++){
      dd1 = (double) -1*cimag(sample_fft[s]);
      sample_fft[s] = creal(sample_fft[s]) + dd1*I;
    }

    // Multiply sample_fft by C/A code FFT
    for (s = 0; s < fft_size; s++){
//    for (s = 0; s < fft_size/2; s++){
      dd1 = (double) creal(sample_fft[s]) * (double) creal(code_fft[prn_num-1][s]) -
                         (double) cimag(sample_fft[s]) * (double) cimag(code_fft[prn_num-1][s]);
      dd2 = (double) creal(sample_fft[s]) * (double) cimag(code_fft[prn_num-1][s]) +
                         (double) cimag(sample_fft[s]) * (double) creal(code_fft[prn_num-1][s]);

      mult_result[s] = dd1 + dd2*I;

    }

    // Perform Inverse FFT of frequency domain multiplication
    fftw_execute(IFFTWPLAN);

    // store fft output
    for (s = 0; s < fft_size; s++){

        // Could log I and Q correlations here to post-process into longer coherent integrations ...

    	dd1 = (double) creal(inverse_fft[s]);
        dd2 = (double) cimag(inverse_fft[s]);

        fft_output[s] = (dd1*dd1 + dd2*dd2)/((double) fft_size);
//        fft_output[s] = (dd1*dd1)/((double) fft_size); // look at real part only
//        fft_output[s] = (dd2*dd2)/((double) fft_size); // look at imag part only

    }

return;
}

// **********************************************************************
// FFT initialization routine
// Takes FFTs of all the PRN codes once
// **********************************************************************
void init_fft_acq(void)
{
unsigned sv_num;
double code_time, code_time_inc;
unsigned s;
int index;
unsigned fft_size = ControlVars.fft_samples;
index = 0;

     // calculate time step based on sampling freq
     code_time_inc = (double) ((double) CODERATE / (double) ControlVars.sampling_freq);
     code_time_inc *= (double) ControlVars.DownSampleNumber;

     for (sv_num = 0; sv_num < MAX_SATELLITES; sv_num++){

        // Clear code_fft and sampled code arrays
        memset(code_fft[sv_num], 0, sizeof(fftw_complex)*fft_size);
//        memset(sampled_code, 0, sizeof(double)*fft_size);
        memset(sampled_code, 0, sizeof(complex)*fft_size); // Giuseppe Grieco (8th-Oct-2018)

        // define fftw plan for this C/A code
//        FFTWPLAN = fftw_plan_dft_r2c_1d(fft_size,sampled_code, code_fft[sv_num], FFTW_ESTIMATE);
        FFTWPLAN = fftw_plan_dft_1d(fft_size,sampled_code, code_fft[sv_num],FFTW_FORWARD, FFTW_MEASURE); // Giuseppe Grieco (8th-Oct-2018)

        // sample the C/A code
        code_time = 0;
        for (s = 0; s < (unsigned int) (ControlVars.coherent_int_samples); s++){
            index = (int) code_time;
//            sampled_code[s] = -1*CODE_TABLE[sv_num][index + 1];
            sampled_code[s] = -1*CODE_TABLE[sv_num][index + 1] + 0*I; // Giuseppe Grieco (8th-Oct-2018)
            code_time += code_time_inc;
        }

        // Take FFT of sampled C/A code
        fftw_execute(FFTWPLAN);

     }  // end GPS satellite loop

    // configure normal acquisition FFT and IFFT, // FFTW_ESTIMATE FFTW_PATIENT FFTW_MEASURE
    FFTWPLAN2 = fftw_plan_dft_1d(fft_size,sample_buf, sample_fft,FFTW_FORWARD, FFTW_MEASURE);
    IFFTWPLAN = fftw_plan_dft_1d(fft_size,mult_result, inverse_fft,FFTW_BACKWARD, FFTW_MEASURE);

}

// **********************************************************************
//  Clean up FFT plans
// **********************************************************************
void shutdown_fft_acq()
{

    fftw_destroy_plan(FFTWPLAN);
    fftw_destroy_plan(FFTWPLAN2);
    fftw_destroy_plan(IFFTWPLAN);

}

