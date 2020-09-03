# Digital Cross Correlator for Laser Beam Signal Processing

This repository includes the most important work from my senior Capstone project. Biomedical applications, particularly in the realm of neuroscience, are this project's main focus.

I would like to give credit to my teammates Alexander Ha, Muhammad Rezuna, and Sidrick Salcedo, and to our faculty advisor Dr. Wayne Kimura.


# Project overview
Certain frequencies of laser light are able to pass through skin, bone, and brain tissue. In analyzing the reflected light from the brain, neuron activity can be analyzed, as neuron activity alters the properties of the reflected light. To enhance laser light reflection, two ultrasound beams of differing frequencies are directed into the brain region of interest. Because the ultrasound beams have different frequencies, the region of interest vibrates at the beat frequency (difference in frequencies) of the ultrasound beams, which "tags" the laser light being reflected from the region of interest with a specific frequency. Tagging the laser light with a specific frequency means that we are able to only detect scatter light from the region of interest.

The main purpose of the project is to devise a method to find the frequency of the reflected light, after it has been tagged by the ultrasound.

To do so, we employed a mathematical operation called cross-correlation. The most important idea behind cross-correlation is that the maximum cross-correlation value is achieved when two waveforms of the same frequency are cross-correlated with each other. So, by varying the frequency of an analyzing signal and cross-correlating that signal with the laser signal, the frequency of the laser signal can be found by examining the largest cross-correlation value.

This Capstone project serves as a proof of concept, so we did not use any ultrasound equipment. However, to simulate the laser light frequency, we used a chopper wheel to split the laser light into a specific frequency.

The main control unit for the digital correlator is a Texas Instruments microcontroller. The chopped laser light is sent to a photodiode, which feeds into an analog-to-digital converter inside the microcontroller. This digitized laser signal is then cross-correlated with an analyzing signal (created through software inside the microcontroller). The maximum cross-correlation value is then displayed on a simple computer GUI program.

![Microcontroller setup](https://github.com/leeway64/Digital-Cross-Correlator-for-Laser-Beam-Signal-Processing/blob/master/Hardware%20components/Digital%20correlator%20system%201.JPG)

Check out [this YouTube video](https://www.youtube.com/watch?v=RO8s1TrElEw&list=WL&index=52&t=1s) for more information on cross-correlation.

For more information on the project, refer to the [abridged project report](https://github.com/leeway64/Digital-Cross-Correlator-for-Laser-Beam-Signal-Processing/blob/master/Abridged%20project%20report.md).
