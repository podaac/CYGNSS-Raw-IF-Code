/*

% Copyright 2016, Scott Gleason
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

//---------------------------------------------------------------------------

#ifndef DAAXA_H
#define DAAXA_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "complex.h"
#include "math.h"

#define SUCCESS 99
#define FAILURE 100
//#define MAX_INTDOPP_BINS 101
#define MAX_INTDOPP_BINS 501
#define PI 3.14159265358979
#define NAV_C	299792458
#define CODERATE 1.023e6
#define L1_FREQUENCY            1575.42e6      // GPS L1 frequency
#define L1_WAVELENGTH           0.190293672798365   // GPS L1 wavelength
#define EARTH_ROTATION_RATE     7.2921151467e-5	 // Earth's WGS84 rotation rate (rads/s), as specified in ICD-GPS-200.
#define SECONDS_PER_DAY         86400
#define SECONDS_PER_WEEK        SECONDS_PER_DAY*7
#define METERS_PER_L1_CHIP 293.052256109482
#define L1_CHIPS_PER_MS 1024

#define GPS_SIN(x) (x > 0 ? 1 : -1)
#define GPS_COS(x) (x > PI/2 || x < -PI/2 ? -1 : 1)
#define UNWRAP_ANGLE(x) { if (x > PI) x -= 2 * PI; else if (x < -PI) x += 2 * PI; }
#define ACQ_MS        1
#define CHIPS_PER_CODE 1023
#define COARSE_ACQ_THRESH 25
#define CODE_FREQ 1.023e6
#define MAX_SATELLITES 32
#define DEBUG_FILE 1
#define FFT_DDM_FILE 2
#define MAX_METADATA_ENTRIES 200
#define OUTPUT_TYPE_FULL_DDM 1
#define OUTPUT_TYPE_CROPPED_DDM 2
#define OUTPUT_TYPE_IDW 3
#define FFT_MAX_SIZE 32768  // 2^15
#define FFT_SIZE 16384  // 2^14
#define MAX_SATELLITES 32
//#define ASCII_BLOCK_SIZE 4000000  // 4M (1M of bytes, 4 samples per byte)
#define ASCII_BLOCK_SIZE 8000000  // 8M (2M of bytes, 4 samples per byte)
//#define ASCII_BLOCK_SIZE 16000000  //
//#define ASCII_BLOCK_SIZE 32000000  //
#define BITS_PER_CHANNEL 2
#define FIRST_DATAREAD 1
#define JUMP2OFFSET 2
#define READDATA 3
#define NUM_SAMPLES_PER_CHIP 16
#define MAX_FILE_DOPPLERS 300 // 5 minutes

// Function prototypes

// CYGNSS_DDMP_Main
int InitProcessing(void);
int StartProcessing(void);
void ReadConfigFile(void);
int ConvertRawDDMI2Int8(unsigned long num_bytes2read, int flag, double seconds_offset);

// CYGNSS_DDMP_Processing
void non_coherent_average(int dopp_bin, double dopp_freq);
void log_output(void);
void ProcessData3(void);

// fftw_acquire.c
void fft_acquire(char *samples, unsigned samples_len, double dopp_freq);
void init_fft_acq(void);
void shutdown_fft_acq();


typedef struct {

        unsigned int WordIndex;
        unsigned int Place;

        char data_fft[FFT_MAX_SIZE];

        double ValueInteg[MAX_INTDOPP_BINS][FFT_MAX_SIZE];
        double ValueIntegIndex[FFT_MAX_SIZE];

        unsigned long PPS_address;

} DataStruct;

typedef struct {

        int GPSweek;
        double GPSseconds;

        unsigned short TimeOffsetFlag;
        unsigned short BlockOffsetFlag;
        unsigned short ProcessDataFlag;
        unsigned short ValueIntegFlag;
        unsigned short LogCorrelationsFlag;
        unsigned short DoppIntegFlag;
        unsigned short StartLogMs;
        unsigned short LogIQFlag;
        unsigned short Cold_Search_Mode;

        double LogTime;
        unsigned short LogBlocks;
        double TimeOffset;
        unsigned long long FileOffset;
        unsigned long Duration;
        unsigned long PPSBlockCount;
        unsigned int ms_count;

        unsigned int FilenameLoaded;
        char Config_infilename[200];

        // Processing parameters
        unsigned short  PRN;
        unsigned short  PRN_file;
        double DopplerStep;
        double StartDoppler;
        double EndDoppler;

        double NoiseFloor;
        int NumDopplerBins;
        double CodeSlide;
        double DopplerSlide;
        double IntegrationDoppler;
        unsigned int DopplerBin;
        unsigned short FE;  // Front End
        unsigned int NonCoherent_Interval;
        double ProcessingDuration;
        unsigned int Number_of_DDMs;
        unsigned int Current_Interval;
        double Current_second;
        int first_DDM;

        unsigned int LogProcessingType;
        unsigned int LogCenteringOption;
        unsigned int LogDelayRange;
        double LogDelaySpacing_Chips;
        unsigned int DownSampleNumber;

        double DDMProcessing_StartRelativeTime;
        double DDMProcessing_EndRelativeTime;
        double DDMProcessing_TimeStep;

        unsigned int Exit_flag;
        unsigned int FirstBytesFudge;

        // FFT Acqusition Variables
        unsigned prn_num;
        unsigned FFT_Acquisition_Flag;
        double car_phase;
        double code_prompt;
        double car_phase_inc;
        double carrier_freq;
        double max_energy;
        double max_ratio;
        double FFT_avg;
        double sampling_freq;
        double IF_freq;
        unsigned samples_ms;
        double   sampling_slide;
        unsigned int fft_samples;
        unsigned int fft_samples_half;
        unsigned int shift_start_index;

        unsigned int debug_flag;
        unsigned int coherent_int_samples;

        // DDMI log conversion
        char DDMI_infilename[200];
        unsigned int num_chans;
        unsigned int antenna2process;
        unsigned int num_bytes;
        unsigned int DDM_log_counter;

        unsigned long Input_Buffer_Place;
        unsigned long Input_Buffer_DataSize;
        unsigned int bytes2skip;

        int File_Flag;
        int num_File_Dopplers;
        double File_Doppler_Range;
        double File_Doppler_Step;
        double File_Dopplers[MAX_FILE_DOPPLERS];
        double Doppler_Start;
        double Doppler_D1;
        double Doppler_D2;
        unsigned int antenna2process_file;

} SharedVariables;

#endif
