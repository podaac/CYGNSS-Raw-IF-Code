%
% process_rawIF_metadata.m
%
% STG, Sept 2017, GPLv3
%
satid_hex2FM = [247 249 43 44 47 54 55 73];
%0xF7 (247): CYGNSS 1
%0xF9 (249): CYGNSS 2
%0x2B ( 43): CYGNSS 3
%0x2C ( 44): CYGNSS 4
%0x2F ( 47): CYGNSS 5
%0x36 ( 54): CYGNSS 6
%0x37 ( 55): CYGNSS 7
%0x49 ( 73): CYGNSS 8

%fid = fopen('../../../OnOrbitData/FM8/cyg08_raw_if_20170825_141629_meta.bin');
%fid = fopen('../../../CYGNSS/RawIF/May15_Australia/RawIF_Data/cyg08_raw_if_20180515_215055_meta.bin');
fid = fopen('/media/gleason/Elements/CYGNSS_Data/rawIF/136/cyg08_raw_if_20200515_231925_meta.bin');

satIDhex = dec2hex(fread(fid,1,'uint8')); % hex ID of satellite (need "decoder ring" to match to FM#)
satIDdec = hex2dec(satIDhex);
satID_FM = find(satid_hex2FM == satIDdec);

hdr = num2str(fread(fid,4,'char'),'%s'); % "DRT0" header
gpsweek = fread(fid,1,'uint16','ieee-be'); % GPS week (read in Big Endian order)
gpssecs = fread(fid,1,'uint32','ieee-be'); % GPS seconds (as 4 byte uint)
dataformat = fread(fid,1,'uint8'); % decimal

% See 148-0354-2 CYGNSS Raw IF Data File Format.pdf for rest of data entries
% The gpssecs and dataformat are the 2 most important

%satIDhex
satID_FM
gpsweek
gpssecs

if(dataformat == 0)
  disp('Channel 1, I Only ... double check, unusual')
elseif(dataformat == 1)   
  disp('Channel 1 and 2, I Only ... double check, unusual')
elseif(dataformat == 2)   
  disp('Channel 1,2,3, I Only  ... default for most collections')
elseif(dataformat == 3)   
  disp('Channel 1,2,3,4 I Only  ... double check, there is no channel 4 connected')
elseif(dataformat == 4)   
  disp('Channel 1,2 I and Q ... double check  unusual, will not work with processor')
else
  disp('Invalid format type')
end  


%fclose(fid);
