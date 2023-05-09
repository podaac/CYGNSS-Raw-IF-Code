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

#include "CYGNSS_DDMP_Main.h"

extern SharedVariables ControlVars;
extern FILE *infileX;
extern FILE *logfileFFT;
extern FILE *infile_DDMI;

void init_all()
{
unsigned int time_idx, num_times, prn_idx, num_prns;
int retval, tempi, sec_idx, flag;
unsigned int i, tempui;
char temp_str[100];
unsigned int down_samples[] = {4,1,2,3,8};
double Dopp_temp;

    memset(&ControlVars,0,sizeof(SharedVariables));
//    strcpy(ControlVars.Config_infilename,"../Configuration_Files/CYGNSS_DDMP_config.dat");
    strcpy(ControlVars.Config_infilename,"CYGNSS_DDMP_config.dat");

    // From Advanced Path
    ControlVars.GPSweek = 1234;
    ControlVars.GPSseconds = 5678;
    ControlVars.Cold_Search_Mode = 0;

    // read in the processing config file
    ReadConfigFile();

// *****************************************
// Cold Search Mode, loop over all PRNs
// Doppler range should be large (+/- ~40k)
// May want to get a cup of coffee for this option ...
// *****************************************

if(ControlVars.Cold_Search_Mode == 1){
	num_prns = 32;
}else{
	num_prns = 1;
}

// PRN loop, usually 1
for(prn_idx=0;prn_idx < num_prns; prn_idx++){

    // for now, its either all PRNs or only 1.  May want to have options to define multple PRNs in config file.
    // My idea is: Search for them all once.  Then process them one at a time over narrower Dopp range over time.
    if(ControlVars.Cold_Search_Mode == 1){
        ControlVars.PRN = prn_idx + 1;
    }else{
        // we are only doing one PRN, defined in GUI
    }

    // calculate number of time loops	
    num_times = (ControlVars.DDMProcessing_EndRelativeTime - ControlVars.DDMProcessing_StartRelativeTime)/ControlVars.DDMProcessing_TimeStep;
//    printf("time loops: %d \n",num_times);

    ControlVars.Number_of_DDMs = 0;
    for(time_idx=0;time_idx<=num_times;time_idx++){

        ControlVars.Current_second = ControlVars.DDMProcessing_StartRelativeTime + ControlVars.DDMProcessing_TimeStep*time_idx;
        ControlVars.Current_Interval = time_idx;

        ControlVars.GPSseconds += ControlVars.Current_second;

        sec_idx = floor(ControlVars.Current_second);
        Dopp_temp = ControlVars.Doppler_Start + ControlVars.Doppler_D1*ControlVars.Current_second + ControlVars.Doppler_D2*ControlVars.Current_second*ControlVars.Current_second;
        ControlVars.StartDoppler =  Dopp_temp - ControlVars.File_Doppler_Range;
        ControlVars.EndDoppler = Dopp_temp + ControlVars.File_Doppler_Range;
        ControlVars.DopplerStep = ControlVars.File_Doppler_Step;

        // Initailize DDM processing
        retval = InitProcessing();

        ControlVars.ms_count = 0;
        for(i=0;i<ControlVars.NonCoherent_Interval;i++){

            // Process 1 look of the DDM
            retval = StartProcessing();

            if(flag == 1){
                break;
            }

            if(retval == FAILURE){
                break;
            }

        } // end ms loop

        // log whole averaged DDM
        log_output();
        ControlVars.Number_of_DDMs++;

        if((flag == 1) || (retval == FAILURE)){
            break;
        }

     }

}  // end of PRN loop

    // write actual number of DDM entries there will be in total to start of file
    rewind(logfileFFT);
    tempui = ControlVars.Number_of_DDMs*num_prns;
    fwrite(&tempui, sizeof(unsigned int), 1, logfileFFT);

    // clean up fft code
    shutdown_fft_acq();

    if(retval == SUCCESS){
        // Finished Successfully
    }else{
        // Finished with Errors.
    }

    if(logfileFFT != NULL){ fclose(logfileFFT);}
    if(infile_DDMI != NULL){ fclose(infile_DDMI);}

}


