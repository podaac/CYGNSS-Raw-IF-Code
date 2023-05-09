function [DDM_cropped,max_dopp_idx,max_delay_idx] = crop_DDM(DDM,buffer_delay_size,buffer_dopp_size,numDelays,numDopplers)


% find max, and center plot around it
[val1 idx1] = max(DDM); 
[val2 idx2] = max(val1); 
x_center_idx = idx2;
y_center_idx = idx1(idx2);
%y_center_idx = 25;

% Crop DDM around the center
x_idx1 = (x_center_idx-buffer_delay_size);
x_idx2 = (x_center_idx+buffer_delay_size);
if(x_idx1 < 1); x_idx1 = 1; end;
if(x_idx2 > numDelays); x_idx2 = numDelays; end;

y_idx1 = (y_center_idx-buffer_dopp_size);
y_idx2 = (y_center_idx+buffer_dopp_size);
if(y_idx1 < 1); y_idx1 = 1; end;
if(y_idx2 > numDopplers); y_idx2 = numDopplers; end;

DDM_cropped = DDM(y_idx1:y_idx2,x_idx1:x_idx2);
DDM_cropped = DDM_cropped';

max_dopp_idx = y_center_idx;
max_delay_idx = x_center_idx;

return;

endfunction