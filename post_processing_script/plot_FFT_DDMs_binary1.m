% Script to plot DDM output of CYGNSS_DDMP (aka Daaxa)
%
% plot_FFT_DDMs_binary1.m
%

close all
clear all
colors = ['k' 'b' 'g' 'r' 'm' 'c'];

sc_id = 8;

plot_full_ddms = 0;
%plot_full_ddms = 1;

%plot_cropped_ddms = 0;
plot_cropped_ddms = 1;

%plot_delay_waveforms = 0;
plot_delay_waveforms = 1;

binaryFilename = sprintf('../Makefile_Build/Processed_DDMs.bin');

% set cropped DDM size
buffer_delay_size = 200;  % samples
buffer_dopp_size = 20; 
%buffer_dopp_size = 48; 

fid = fopen(binaryFilename);

% read in dimensions of the DDM
numDDMentries = fread(fid, 1, 'int');

if(plot_cropped_ddms == 1)
  % cropped DDM
  figure(100);
  colorbar;
end

if(plot_full_ddms == 1)
  % Full DDMs
  figure(200);
  colorbar;
end

if(plot_delay_waveforms == 1)
  % Delay Waveforms
  figure(1);
  hold on
  ylabel('Signal Magntitude (power)');
  xlabel('Delay (Code Phase)');
  title('Delay Waveforms');
end

for DDM_idx = 1:numDDMentries

  % read in meta data for this DDM
  DDM_hdr = char(fread(fid, 4, 'char'));
  DDM_log_counter(DDM_idx) = fread(fid, 1, 'int');
  GPSweek(DDM_idx) = fread(fid, 1, 'int');
  GPSSecond(DDM_idx) = fread(fid, 1, 'double');
  CurrentSecond(DDM_idx) = fread(fid, 1, 'double');
  FileOffset(DDM_idx) = fread(fid, 1, 'unsigned long long');
  PRN(DDM_idx) = fread(fid, 1, 'short');
  StartDoppler(DDM_idx) = fread(fid, 1, 'double');
  EndDoppler(DDM_idx) = fread(fid, 1, 'double');
  DopplerStep(DDM_idx) = fread(fid, 1, 'double');
  numDopplers(DDM_idx) = floor((EndDoppler(DDM_idx) - StartDoppler(DDM_idx))/DopplerStep(DDM_idx)) + 1;
  numDelays(DDM_idx) = fread(fid, 1, 'int');
  DelayStep_Chips(DDM_idx) = fread(fid, 1, 'double');
  Doppler_axis = [StartDoppler(DDM_idx):DopplerStep(DDM_idx):EndDoppler(DDM_idx)];
  Delay_axis = [0:DelayStep_Chips(DDM_idx):numDelays(DDM_idx)*DelayStep_Chips(DDM_idx)-DelayStep_Chips(DDM_idx)];

  % read in DDM
  doubles2read = numDopplers(DDM_idx)*numDelays(DDM_idx);
  DDM_data = fread(fid,doubles2read,'double');
  DDM = reshape(DDM_data,numDelays(DDM_idx),numDopplers(DDM_idx));
  DDM = DDM';  % transpose it

  antenna(DDM_idx) = fread(fid, 1, 'unsigned int');

  [DDM2,max_dopp_idx,max_delay_idx] = crop_DDM(DDM,buffer_delay_size,buffer_dopp_size,numDelays,numDopplers);
%  Doppler_axis2 = Doppler_axis(max_dopp_idx-buffer_dopp_size:max_dopp_idx+buffer_dopp_size);
%  Delay_axis2 = Delay_axis(max_delay_idx-buffer_delay_size:max_delay_idx+buffer_delay_size);

  noise_temp = mean(mean(DDM2(1:10,:))); % first rows of cropped DDM
  signal_max(DDM_idx) = max(max(DDM2)); % max
  SNR_dB(DDM_idx) = 10*log10(signal_max(DDM_idx)/noise_temp);
  Max_Doppler(DDM_idx) = Doppler_axis(max_dopp_idx);
  Max_Delay(DDM_idx) = Delay_axis(max_delay_idx);
  
  if(SNR_dB(DDM_idx) > 3)
    PRN(DDM_idx) 
    Max_Doppler(DDM_idx)
    SNR_dB(DDM_idx)
  end
  
if(plot_delay_waveforms == 1)
  figure(1);
  color_idx = mod(DDM_idx,length(colors));
  if(color_idx == 0)
    color_idx = 6;
  end
 	plot(Delay_axis,DDM(max_dopp_idx,:),Color=colors(color_idx));
% 	plot(DDM(max_dopp_idx,:),Color=colors(color_idx));
end
  
if(plot_cropped_ddms == 1)
  % plot cropped DDM
  figure(100);
  imagesc(flipud(DDM2));
%  imagesc(Doppler_axis2,Delay_axis2,flipud(DDM2));
  ylabel('Delay Bins');
  xlabel('Doppler Bins');
%  ylabel('Delay (chips)');
%  xlabel('Doppler (Hz)');
  str = sprintf("Raw IF Processed DDM, S/C %d, PRN %d, DDM Number %d",sc_id, PRN(DDM_idx),DDM_idx);
  title(str)
  drawnow;
%  axis([3000 8000 470 490])

if(0)
      filename = ['images/' num2str(DDM_idx,"%03d") '_imageframe.jpg'];
      print(filename,'-djpg')
end


end

if(plot_full_ddms == 1)
  % plot full DDM
  figure(200);
%  imagesc(Doppler_axis,Delay_axis,DDM');
  imagesc(DDM);
  ylabel('Delay (Code Phase)');
  xlabel('Doppler (Hz)');
  str = sprintf("Full DDM for PRN %d, DDM Number %d",PRN(DDM_idx),DDM_idx);
  title(str)
  colorbar;
%  axis([startDoppler(DDM_idx) endDoppler(DDM_idx) 1 numDelays(DDM_idx) 0 6e12])
end  
  
  pause(1);
 
end  % end DDM loop

%PRN
%Max_Doppler
%Max_Delay
  
  
