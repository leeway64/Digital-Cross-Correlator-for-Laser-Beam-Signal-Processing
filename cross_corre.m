% By Lee-Way Wang
% Inputs: vector x is the signal data, and vector y is the wave
% that is being stepped through time.
% Input vector length
function correlation_array = cross_corre(x, y)
    
    size = length(y);

    % Essentially, I am making each array three times larger

    % Array that is storing x values in larger array. The x values do not
    % move, and so they are placed exactly in the middle of the larger array.
    temp_length = 3 * size;
    temp_array_x = zeros(1, temp_length);
    temp_array_x(size + 1: 2 * size) = x;

    % Array that is storing y values in larger array. The y values are being
    % stepped through time, so the y values are placed at the very beginning
    % this larger array.
    temp_array_y = zeros(1, temp_length);
    temp_array_y(1:size) = y;

    % Temporary array that is storing correlated values with an extra
    % zero at both ends
    temp_correlation_array = zeros(1, (2 * size) + 1);

    % Each temporary array's values is multiplied with each other, and the 
    % sum is stored in the temp correlation array. At the end of this
    % operation, the temp y array is circular shifted, so the last value
    % is moved to the front. In this way, y is stepped through time.
    for i = 1:2*size+1
        temp_correlation_array(i) = sum(temp_array_y .* temp_array_x);
        temp_array_y = circshift(temp_array_y,1);
    end

    % Actual, final array that is storing correlated values. The superfluous
    % zeroes at the end of  the array are removed.
    correlation_array = temp_correlation_array(2:end-1);
end