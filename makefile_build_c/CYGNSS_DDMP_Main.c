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

#include "CYGNSS_DDMP_Main.h"

extern SharedVariables ControlVars;
extern double averaged_fft_output[MAX_INTDOPP_BINS][FFT_MAX_SIZE];
extern DataStruct RawData;

char CYGNSS_Data_Binned_1sec[ASCII_BLOCK_SIZE];

FILE *logfileFFT;
FILE *infile_DDMI;

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

int InitProcessing(void)
{
int retval = SUCCESS;
unsigned long tempul;
double tempd;
//double sampling_slide_array[] = {0.2,0.2,0.1,0.4,0.05,0.24,0.7,0.885,0.525};
//double sampling_slide_array[] = {0.2,0.2,0.18,0.46,0.105,0.24,0.7,0.885,0.56};  // screwball cpt_ar20 data

if(ControlVars.first_DDM == 0){

    ControlVars.samples_ms = (unsigned) ControlVars.sampling_freq/1000;
    ControlVars.coherent_int_samples = (unsigned int) (ControlVars.samples_ms/ControlVars.DownSampleNumber);
    ControlVars.fft_samples = ControlVars.coherent_int_samples;
    ControlVars.LogDelaySpacing_Chips = ((double)L1_CHIPS_PER_MS/ (double)ControlVars.coherent_int_samples);

    // calculate sampling slide based on sampling_freq
    tempd = 0.001 - (double) (1/ControlVars.sampling_freq * (double)ControlVars.DownSampleNumber * (double)ControlVars.coherent_int_samples);
    ControlVars.sampling_slide = tempd*(ControlVars.sampling_freq/(double)ControlVars.DownSampleNumber);

// STG Test
//    ControlVars.sampling_slide = fmod(ControlVars.sampling_freq, 1000)/1000;
//      ControlVars.sampling_slide = sampling_slide_array[ControlVars.DownSampleNumber];
//    ControlVars.DownSampleNumber = 2;
//    ControlVars.sampling_slide = 0.1;

    // pre-calculate FFT's of L1 C/A Codes
    init_fft_acq();

    if(!logfileFFT) {
        logfileFFT = fopen("Processed_DDMs.bin","wb");
        if(!logfileFFT) {
            retval = FAILURE;
            return(retval);
        }
        // write number of DDM entries there will be in total to start of file
        fwrite(&ControlVars.Number_of_DDMs, sizeof(unsigned int), 1, logfileFFT);
    }

        // Read raw data header and set file offset to start of sampled IF
        retval = ConvertRawDDMI2Int8(0, FIRST_DATAREAD, 0);  // no data read in
        ControlVars.first_DDM = 1;
 }

    // clear arrays
    memset(&RawData,0,sizeof(RawData));
    memset(averaged_fft_output,0,sizeof(averaged_fft_output));

    // Skip ahead in raw data file to start second
    retval = ConvertRawDDMI2Int8(0, JUMP2OFFSET, ControlVars.Current_second);  // no data read in

    // Read in block of raw IF data, convert to -3,-1,1,3 char format
    tempul = ASCII_BLOCK_SIZE/4;
    retval = ConvertRawDDMI2Int8(tempul, READDATA, 0);  // data read in and converted

return(retval);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

int StartProcessing(void)
{
int retval = SUCCESS;
unsigned int j;
unsigned long tempul;

    // Extract 1 ms of samples
    for(j=0;j< ControlVars.coherent_int_samples;j++){

        if(ControlVars.Input_Buffer_Place > ControlVars.Input_Buffer_DataSize){
            // Read in block of raw IF data, convert to -3,-1,1,3 char format
            tempul = ASCII_BLOCK_SIZE/4;
            retval = ConvertRawDDMI2Int8(tempul, READDATA, 0);  // data read in and converted
        }

        RawData.data_fft[j] = CYGNSS_Data_Binned_1sec[ControlVars.Input_Buffer_Place];
        ControlVars.Input_Buffer_Place += ControlVars.DownSampleNumber;

    }

    if(retval == FAILURE){
        return(retval);  // problem reading data from file
    }

    // *********************************
    // Process 1ms look DDM
    // *********************************
    ProcessData3();
    ControlVars.ms_count++;
    // **********************************

    // clear fft data
    for(j=0;j<ControlVars.coherent_int_samples;j++){
        RawData.data_fft[j] = 0;
    }

return(retval);
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void ReadConfigFile(void)
{
int	testi=0;
double testd = 0;
int i;

FILE *infile;
char tempc=0;

//infile = fopen("CYGNSS_DDMP_config.dat", "r" );
infile = fopen(ControlVars.Config_infilename, "r" );

if(!infile) {
        printf("\nUnable to Open Config File.\n");
        return;
}
else{

    rewind(infile);  // make sure we are at the beginning

	while(!feof(infile))  /* until end of file */
	{

		/* Read in script file */
		fread(&tempc,1,1,infile);

		if(tempc == '^'){

		fread(&tempc,1,1,infile);

        if(tempc == 'C'){

                        // 32 PRN Cold Search
                        fscanf(infile," %d",&testi);
                        ControlVars.Cold_Search_Mode = testi;
	}

        if(tempc == 'D'){

                        // User specified center Dopplers in config file
                        fscanf(infile," %d",&testi);
                        ControlVars.PRN = testi;
                        fscanf(infile," %d",&testi);
                        ControlVars.antenna2process = testi;
                        fscanf(infile," %lf",&testd);
                        ControlVars.File_Doppler_Range = testd;
                        fscanf(infile," %lf",&testd);
                        ControlVars.File_Doppler_Step = testd;
                        fscanf(infile," %lf",&testd);
                        ControlVars.Doppler_Start = testd;
                        fscanf(infile," %lf",&testd);
                        ControlVars.Doppler_D1 = testd;
                        fscanf(infile," %lf",&testd);
                        ControlVars.Doppler_D2 = testd;

         }

        if(tempc == 'F'){

                        // Read in raw data filename
                        fscanf(infile," %200s",&ControlVars.DDMI_infilename[0]);
                        ControlVars.FilenameLoaded = 1;

         }

        if(tempc == 'P'){

                        // TIme processing parameters
                        fscanf(infile," %lf",&testd);
                        ControlVars.sampling_freq = testd;
                        fscanf(infile," %lf",&testd);
                        ControlVars.IF_freq = testd;
                        fscanf(infile," %d",&testi);
                        ControlVars.num_chans = testi;
                        fscanf(infile," %d",&testi);
                        ControlVars.NonCoherent_Interval = testi;
                        fscanf(infile," %d",&testi);
                        ControlVars.DownSampleNumber = testi;

         }  // 'P'

        if(tempc == 'T'){

                        // TIme processing parameters
                        fscanf(infile," %lf",&testd);
                        ControlVars.DDMProcessing_StartRelativeTime = testd;
                        fscanf(infile," %lf",&testd);
                        ControlVars.DDMProcessing_EndRelativeTime = testd;
                        fscanf(infile," %lf",&testd);
                        ControlVars.DDMProcessing_TimeStep = testd;

         }  // 'T'

	}  // if '^'

  }  // end while


  printf("\nConfig File Successfully Loaded\n");

}  // end else

return;
}

// %%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

int ConvertRawDDMI2Int8(unsigned long num_bytes2read, int flag, double seconds_offset)
{
int retval = SUCCESS;
unsigned char tempc=0;
unsigned int i,j,num_bytes;
unsigned int bytes[4];
double temp_bytes_double, temp_bits_double, tempd;
unsigned long long temp_bytes;
char tempb;
//char temp_chars[100];
unsigned long bytes_read = 0;
unsigned long values_added = 0;


if(flag == FIRST_DATAREAD){

    // Open file and read in/jump over initial header info

    infile_DDMI = fopen(ControlVars.DDMI_infilename, "rb" );

    if(infile_DDMI == NULL){
        retval = FAILURE;
	printf("\nRaw IF File Not Opened\n");
	printf("\n %s \n.",ControlVars.DDMI_infilename);
        return(retval);
    }

    // clear the (very large) input data array
    memset(CYGNSS_Data_Binned_1sec,0,sizeof(CYGNSS_Data_Binned_1sec));

    if(ControlVars.antenna2process == 1){
        // read off first 35 bytes
        ControlVars.bytes2skip = 35;
    }else if(ControlVars.antenna2process == 2){
        // read off first 36 bytes
        ControlVars.bytes2skip = 36;
    }else if(ControlVars.antenna2process == 3){
        // read off first 37 bytes
        ControlVars.bytes2skip = 37;
    }else if(ControlVars.antenna2process == 4){
        // read off first 38 bytes
        ControlVars.bytes2skip = 38;
    }else{
        // ERROR
        retval = FAILURE;
        return(retval);
    }

    // Jump ahead in the file
    fseek(infile_DDMI,ControlVars.bytes2skip,SEEK_SET);   // from start of file

}else if(flag == JUMP2OFFSET){

        if(infile_DDMI == NULL){
            retval = FAILURE;
            return(retval);
        }

        // calculate offset
        // jump ahead, and stay aligned!
        temp_bits_double = seconds_offset*(double)ControlVars.sampling_freq*(double)ControlVars.num_chans*(double)BITS_PER_CHANNEL;
        temp_bytes_double = temp_bits_double/8.0;
        tempd = floor(temp_bytes_double);

        if(ControlVars.num_chans == 1){
                // will always be byte aligned, just floor the offset
                temp_bytes = (unsigned long long) floor(temp_bytes_double);
        }else if(ControlVars.num_chans == 2){
                // int bytes must be divisible by 2
                temp_bytes = (unsigned long long) (tempd - fmod(tempd,2));
        }else if(ControlVars.num_chans == 3){
                // int bytes must be divisible by 3
                temp_bytes = (unsigned long long) (tempd - fmod(tempd,3));
        }else if(ControlVars.num_chans == 4){
                // int bytes must be divisible by 4
                temp_bytes = (unsigned long long) (tempd - fmod(tempd,4));
        }else{
                retval = FAILURE;
                return(retval);
        }

        temp_bytes += ControlVars.bytes2skip;
        ControlVars.FileOffset = (unsigned long long) temp_bytes;
        // Jump ahead in the file
        fseek(infile_DDMI,temp_bytes,SEEK_SET);   // from start of file

}else if(flag == READDATA){

    // Read in data block to chars
    // sign and mag, 2 bits per channel

    if(infile_DDMI == NULL){
        retval = FAILURE;
        return(retval);
    }

    // loop over requested number of bytes
    for(i=0;i<num_bytes2read;i++){

            // read in 1 byte from infile
            num_bytes = fread(&tempc,sizeof(unsigned char),1,infile_DDMI);
            if(num_bytes != 1){
                retval = FAILURE;
                return(retval);
            }
            bytes_read += num_bytes;

            // unpack the bits
            bytes[0] = (tempc >> 6) & 0x03;
            bytes[1] = (tempc >> 4) & 0x03;
            bytes[2] = (tempc >> 2) & 0x03;
            bytes[3] = (tempc) & 0x03;

            // write bytes to new file
            for(j=0;j<4;j++){

                if(bytes[j] == 0){
                    tempb = -1;
                }else if(bytes[j] == 1){
                    tempb = -3;
                }else if(bytes[j] == 2){
                    tempb = 1;
                }else if(bytes[j] == 3){
                    tempb = 3;
                }

                CYGNSS_Data_Binned_1sec[i*4+j] = tempb;
                values_added++;

            }

            // read in and don't process bit packet bytes from other channels
            if(ControlVars.num_chans == 2){
                // skip ahead 1 byte get to next value for this chan
                fread(&tempc,sizeof(unsigned char),1,infile_DDMI);
            }else if(ControlVars.num_chans == 3){
                // skip ahead 2 bytes get to next value for this chan
                fread(&tempc,sizeof(unsigned char),1,infile_DDMI);
                fread(&tempc,sizeof(unsigned char),1,infile_DDMI);
            }else if(ControlVars.num_chans == 4){
                // skip ahead 3 bytes get to next value for this chan
                fread(&tempc,sizeof(unsigned char),1,infile_DDMI);
                fread(&tempc,sizeof(unsigned char),1,infile_DDMI);
                fread(&tempc,sizeof(unsigned char),1,infile_DDMI);
            }
    }  // bytes loop

    ControlVars.Input_Buffer_Place = 0;
    ControlVars.Input_Buffer_DataSize = values_added-1;

//    memcpy(temp_chars,&CYGNSS_Data_Binned_1sec[0],100); // debug


}else{
    retval = FAILURE;
    return(retval);
}

return(retval);
}
