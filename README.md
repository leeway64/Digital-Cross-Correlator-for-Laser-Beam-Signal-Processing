# Digital-Cross-Correlator-for-Laser-Beam-Signal-Processing

# Final results
The digital correlator successfully displayed the average maximum correlation value over several samples. With the analyzing frequency set to 1300 Hz and the number of samples set to 6, the input photodiode signal was varied and 10 trials were taken for each. The SNR was calculated by dividing the average maximum correlation of the 0.5 mV and 20 mV signal by the average maximum correlation without an input signal.


We first compared the maximum correlation with no input signal to the maximum correlation using the high gain amplifier (20 mV input signal). The SNR was 4.08, meaning that, with the high gain amplifier, there is greater signal than noise. Using the high gain amplifier, the signal is amplified 1000 times from the microvolts to the millivolts range. More meaningful data is taken when using the high gain amplifier at the input.

![20 mV](https://github.com/leeway64/Digital-Cross-Correlator-for-Laser-Beam-Signal-Processing/blob/master/Final%20results/20%20mV%20input%20results.jpg)


Figure 9.1 shows the maximum correlation value along with a plot of the ADC data. Notice that, although one can see the peaks and troughs of the square wave, it is much obscured by noise.

Then, we compared the maximum correlation with no input signal to the maximum correlation without the high gain amplifier (0.5 mV input signal). The SNR was 1.07, meaning that without the high amplifier, the signal is improved by 7% compared to the noise. In other words, the signal is almost equal to the noise.

![0.5 mV](https://github.com/leeway64/Digital-Cross-Correlator-for-Laser-Beam-Signal-Processing/blob/master/Final%20results/0.5%20mV%20input%20results.jpg)

Notice how, in Figure 9.2, that a square wave is completely invisible inside the ADC data plot. One cannot see any trace of the square wave.

![No input](https://github.com/leeway64/Digital-Cross-Correlator-for-Laser-Beam-Signal-Processing/blob/master/Final%20results/No%20input%20results.jpg)

Figure 9.2 is almost indistinguishable from Figure 9.3, which shows the data plot when there is zero input.

We can conclude that, with the high gain amplifier, the signal is improved greatly by a factor of 4 compared to the noise. The high gain amplifier ensures that more meaningful data is taken.

![Strong input](https://github.com/leeway64/Digital-Cross-Correlator-for-Laser-Beam-Signal-Processing/blob/master/Final%20results/Strong%20input%20signal%20results.jpg)

Based on Figure 9.4, we also found that the ADC can digitize negative values, to a certain degree. The analog values less than 0 are digitized to become 0.
