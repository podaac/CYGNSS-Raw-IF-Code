%
% Load_CYGNSS_netCDF_Level1.m
% January 2017, STG
% Eliminated a few obsolete variables (Aug 2020)
% Loads netCDF variables from CYGNSS PO-DAAC Level 1 netcdf
%
%close all
%clear all

pkg load netcdf

%%%%%%%%%%%%%%%%%%%
% L1 Data
%%%%%%%%%%%%%%%%%%%

sample = double(ncread(filename,'sample'));
sc_id = double(ncread(filename,'spacecraft_id'));
sc_num = double(ncread(filename,'spacecraft_num'));
prn_code = double(ncread(filename,'prn_code'));
ddm_ant = double(ncread(filename,'ddm_ant'));
ddm_chan = double(ncread(filename,'ddm'));

utc_time = double(ncread(filename,'ddm_timestamp_utc')); 
gpsweek_ddm = double(ncread(filename,'ddm_timestamp_gps_week')); 
gpssecs_ddm = double(ncread(filename,'ddm_timestamp_gps_sec')); 

sc_pos_x = double(ncread(filename,'sc_pos_x')); 
sc_pos_y = double(ncread(filename,'sc_pos_y')); 
sc_pos_z = double(ncread(filename,'sc_pos_z')); 
sc_vel_x = double(ncread(filename,'sc_vel_x')); 
sc_vel_y = double(ncread(filename,'sc_vel_y')); 
sc_vel_z = double(ncread(filename,'sc_vel_z')); 
SC_longitude = double(ncread(filename,'sc_lon'));
SC_latitude = double(ncread(filename,'sc_lat'));
SC_altitude = double(ncread(filename,'sc_alt'));
rx_clk_bias = double(ncread(filename,'rx_clk_bias'));
rx_clk_bias_rate = double(ncread(filename,'rx_clk_bias_rate'));
  
tx_pos_x = double(ncread(filename,'tx_pos_x')); 
tx_pos_y = double(ncread(filename,'tx_pos_y')); 
tx_pos_z = double(ncread(filename,'tx_pos_z')); 
tx_vel_x = double(ncread(filename,'tx_vel_x')); 
tx_vel_y = double(ncread(filename,'tx_vel_y')); 
tx_vel_z = double(ncread(filename,'tx_vel_z')); 

sc_roll = double(ncread(filename,'sc_roll')); 
sc_pitch = double(ncread(filename,'sc_pitch')); 
sc_yaw = double(ncread(filename,'sc_yaw')); 

sp_pos_x = double(ncread(filename,'sp_pos_x')); 
sp_pos_y = double(ncread(filename,'sp_pos_y')); 
sp_pos_z = double(ncread(filename,'sp_pos_z')); 
sp_lon = double(ncread(filename,'sp_lon'));
sp_lat = double(ncread(filename,'sp_lat'));
sp_inc_angle =  double(ncread(filename,'sp_inc_angle'));
sp_theta_orbit =  double(ncread(filename,'sp_theta_orbit'));

gps_eirp = 10.*log10(double(ncread(filename,'gps_eirp')));  % in dB
ddm_noise_floor = double(ncread(filename,'ddm_noise_floor'));
inst_gain = double(ncread(filename,'inst_gain'));
sp_rx_gain = double(ncread(filename,'sp_rx_gain'));
quality_flags = double(ncread(filename,'quality_flags'));
status_flags_one_hz = double(ncread(filename,'status_flags_one_hz'));
rx_to_sp_range = double(ncread(filename,'rx_to_sp_range'));
tx_to_sp_range = double(ncread(filename,'tx_to_sp_range'));
path_total = rx_to_sp_range + tx_to_sp_range;

lna_temp_nadir_starboard = double(ncread(filename,'lna_temp_nadir_starboard'));
lna_temp_nadir_port = double(ncread(filename,'lna_temp_nadir_port'));

% SOC L1a L1b and area values ... needed after E2ES for comparison
raw_counts = double(ncread(filename,'raw_counts'));
power_analog = double(ncread(filename,'power_analog'));
power_digital = double(ncread(filename,'power_digital'));
eff_scatter = double(ncread(filename,'eff_scatter'));
brcs = double(ncread(filename,'brcs'));

sp_theta_orbit = double(ncread(filename,'sp_theta_orbit'));
sp_az_orbit = double(ncread(filename,'sp_az_orbit'));
sp_theta_body = double(ncread(filename,'sp_theta_body'));
sp_az_body = double(ncread(filename,'sp_az_body'));

sp_ddmi_dopp = double(ncread(filename,'sp_ddmi_dopp'));

zenith_code_phase = double(ncread(filename,'zenith_code_phase'));
add_range_to_sp = double(ncread(filename,'add_range_to_sp'));

lna_noise_figure = double(ncread(filename,'lna_noise_figure'));
inst_gain = double(ncread(filename,'inst_gain'));
radiometric_antenna_temp = double(ncread(filename,'radiometric_antenna_temp'));

bit_ratio_hi_lo_starboard = double(ncread(filename,'bit_ratio_hi_lo_starboard'));
bit_ratio_hi_lo_port = double(ncread(filename,'bit_ratio_hi_lo_port'));
bit_null_offset_starboard = double(ncread(filename,'bit_null_offset_starboard'));
bit_null_offset_port = double(ncread(filename,'bit_null_offset_port'));

%ideal_scatter = double(ncread(filename,'ideal_scatter')); 
box_ddma_scatter = double(ncread(filename,'nbrcs_scatter_area'));
%eaf_ddma_scatter = double(ncread(filename,'eaf_ddma_scatter'));

ddm_spec_pt_delay_bin = double(ncread(filename,'brcs_ddm_peak_bin_delay_row'));
ddm_spec_pt_dopp_bin = double(ncread(filename,'brcs_ddm_peak_bin_dopp_col'));
ddm_les = double(ncread(filename,'ddm_les'));
ddm_ddma = double(ncread(filename,'ddm_nbrcs'));
ddm_brcs_uncert = double(ncread(filename,'ddm_brcs_uncert'));
