/*

% Copyright 2016, Scott Gleason and Nicholas Othieno
% License: GNU GPLv3
%
% sgleason@stanfordalumni.org
%
% This program is free software; you can redistribute it and/or modify
% it under the terms of the GNU General Public License as published by
% the Free Software Foundation; either version 2 of the License, or
% (at your option) any later version.
%
% This program is distributed in the hope that it will be useful, but
% WITHOUT ANY WARRANTY; without even the implied warranty of
% MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
% General Public License for more details.
%
% You should have received a copy of the GNU General Public License
% along with this program; if not, write to the Free Software
% Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307
% USA

*/

#include "CYGNSS_DDMP_Main.h"

extern double fft_output[FFT_MAX_SIZE];
double averaged_fft_output[MAX_INTDOPP_BINS][FFT_MAX_SIZE];

SharedVariables ControlVars;
DataStruct RawData;

extern FILE *infileX;
extern FILE *logfileFFT;

// ***************************************************************************************
//  void ProcessData3(void)
//
//  FFT processing method for Doppler loop
//
// ***************************************************************************************
void ProcessData3(void){
double dopp_freq;
int i;

ControlVars.NumDopplerBins = (int) fabs(ControlVars.EndDoppler - ControlVars.StartDoppler)/ControlVars.DopplerStep + 1;

// Start of Doppler Loop
for(i=0;i < ControlVars.NumDopplerBins;i++){


    dopp_freq = ControlVars.StartDoppler + ControlVars.DopplerStep*i;

    // *********************************
    // FFT delay processing
    // *********************************
    // clear output array
    memset(fft_output,0,sizeof(fft_output));

    // check if samples_ms should be fft_samples (power of 2), STG
    fft_acquire(RawData.data_fft,ControlVars.coherent_int_samples,dopp_freq);

    // Average outputs
    non_coherent_average(i, dopp_freq); // i is dopp_bin idx

}  // end of Doppler loop

}

// ***************************************************************************************
//  void non_coherent_average(void)
//
//  shifts (based on sampling overflow and reflection dynamics) fft output
//  and adds it to the accumulating result.
//
// ***************************************************************************************
void non_coherent_average(int dopp_bin, double dopp_freq){
double shift_rate;
double shift_mag;
unsigned int temp_index;
unsigned int i;
double CodeDynamics_samples_per_coint, tempd;

tempd = (double) ControlVars.coherent_int_samples / (double) NAV_C;
CodeDynamics_samples_per_coint = (dopp_freq*L1_WAVELENGTH)*tempd;;
// coherent_int_samples already accounts for downsampling
//CodeDynamics_samples_per_coint /= (double) ControlVars.DownSampleNumber;

//shift_rate = ControlVars.sampling_slide + CodeDynamics_samples_per_coint;
shift_rate = ControlVars.sampling_slide - CodeDynamics_samples_per_coint;

    // calculate shift magnitude
//    shift_mag = ControlVars.ms_count*shift_rate;
//    ControlVars.shift_start_index = floor(fabs(shift_mag));
    shift_mag = -1.0*ControlVars.ms_count*shift_rate;
    if(shift_mag < 0){
        ControlVars.shift_start_index = ControlVars.coherent_int_samples + round(shift_mag);
    }else{
        ControlVars.shift_start_index = floor(fabs(shift_mag));
    }

    // copy fft output to average
//    temp_index = 0;
//    for(i=ControlVars.shift_start_index;i<ControlVars.shift_start_index+ControlVars.coherent_int_samples;i++){
//            averaged_fft_output[dopp_bin][i] += fft_output[temp_index];
//            temp_index++;
//    }

    // new way, slide fft_output not averaged_fft_output
    for(i=0;i<ControlVars.shift_start_index;i++){
        averaged_fft_output[dopp_bin][ControlVars.coherent_int_samples - ControlVars.shift_start_index + i] += fft_output[i];
    }
    temp_index = 0;
    for(i=ControlVars.shift_start_index;i<ControlVars.coherent_int_samples;i++){
        averaged_fft_output[dopp_bin][temp_index++] += fft_output[i];
    }


}

// ***************************************************************************************
//  logs DDMs to file for all averaged delays and dopplers
// ***************************************************************************************
void log_output(void){
unsigned int i,j,tempi;
unsigned int num_dopp_bins;
double tempd;
double max_val = 0;
unsigned int max_delay_idx = 0;
unsigned int max_dopp_idx = 0;
unsigned int start_delay_idx,end_delay_idx;
char ddm_hdr[5] = "DDMX";  // size 5 to include null terminator, QT thing

num_dopp_bins = ControlVars.NumDopplerBins;

if(logfileFFT != NULL){

            // write DDM header
            fwrite(&ddm_hdr, sizeof(char), 4, logfileFFT);
            // increment and write DDM log counter
            ControlVars.DDM_log_counter++;
            fwrite(&ControlVars.DDM_log_counter, sizeof(unsigned int), 1, logfileFFT);

            // write GPS reference week
            fwrite(&ControlVars.GPSweek, sizeof(int), 1, logfileFFT);
            // write GPS reference second, for linking raw data to normal netCDF files
            fwrite(&ControlVars.GPSseconds, sizeof(double), 1, logfileFFT);
            // write relative second into the file for this DDMS
            fwrite(&ControlVars.Current_second, sizeof(double), 1, logfileFFT);
            // write file offset in bytes
            fwrite(&ControlVars.FileOffset, sizeof(unsigned long long), 1, logfileFFT);
            // write PRN
            fwrite(&ControlVars.PRN, sizeof(unsigned short), 1, logfileFFT);
            // write first Doppler
            fwrite(&ControlVars.StartDoppler, sizeof(double), 1, logfileFFT);
            // write Doppler step
            fwrite(&ControlVars.EndDoppler, sizeof(double), 1, logfileFFT);
            // write number of Dopplers
            fwrite(&ControlVars.DopplerStep, sizeof(double), 1, logfileFFT);
            // write number of delay bins
            tempi = ControlVars.coherent_int_samples;
            fwrite(&tempi, sizeof(unsigned int), 1, logfileFFT);
            // write Delay bin step
            fwrite(&ControlVars.LogDelaySpacing_Chips, sizeof(double), 1, logfileFFT);

            // write DDM data
//            for(j=0;j<num_dopp_bins;j++){
//                for(i=ControlVars.shift_start_index;i<ControlVars.shift_start_index+ControlVars.coherent_int_samples;i++){
//                    fwrite(&averaged_fft_output[j][i], sizeof(double), 1, logfileFFT);
//                }
//            }  // next Doppler

            for(j=0;j<num_dopp_bins;j++){
                for(i=0;i<ControlVars.coherent_int_samples;i++){
                    fwrite(&averaged_fft_output[j][i], sizeof(double), 1, logfileFFT);
                }
            }  // next Doppler

            // write antenna at end (to keep Octave script backwards compatible)
            // plus 1 to match 1,2,3 antenna numbering (1=zenith, 2=nadir 1, 3=nadir 2), while antenna2process is 0,1,2
            fwrite(&ControlVars.antenna2process, sizeof(unsigned int), 1, logfileFFT);


}  // if file OK

}

/*************************************************************************/
