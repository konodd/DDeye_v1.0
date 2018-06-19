data = importdata("auto_calibration.txt");
x = data(:, 3);
y = data(:, 4);
figure;
plot(x, y);