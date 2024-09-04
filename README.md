
---

# CS 566 Speech Processing | IIT Guwahati

## Introduction

Welcome to the repository for the CS 566 Speech Processing course at IIT Guwahati. This project explores various aspects of speech analysis using C++ and machine learning algorithms. The assignments cover fundamental techniques like feature extraction, word classification, and vowel analysis using Linear Predictive Coding (LPC). Each assignment builds on the previous one, contributing to a deeper understanding of speech processing and its applications.

## Assignments

### [Assignment 1: Yes/No Speech Detection using C++](./Assignment1)
This assignment focuses on classifying a recorded speech segment as either "Yes" or "No." The process involves reading a stored audio file in .wav format, the program calculates essential features like Energy and Zero Crossing Rate (ZCR) to determine the spoken word. The approach includes:
- **Noise Floor Calculation** using RMS method to estimate ambient noise.
- **Word Segmentation** based on the noise floor to identify the start and end of the speech.
- **Feature Extraction** involving the computation of Energy and ZCR for the segmented word.
- **Classification** using a heuristic that differentiates between "Yes" and "No" based on the average ZCR and Energy.

### [Assignment 2: Yes/No Speech Detection using C++](./Assignment2)
Similar to Assignment 1, this assignment also deals with classifying speech as "Yes" or "No" with real-time speech signal processing.
- **DC Shift Calculation** to eliminate bias from the recorded signal.
- **Normalization** to adjust the amplitude of the speech signal.

### [Assignment 3: Vowel Analysis using Linear Predictive Coding (LPC)](./Assignment3)
This assignment delves into the analysis of vowel sounds (a, e, i, o, u) using Linear Predictive Coding (LPC). The main tasks include:
- **Recording and Trimming** vowel sounds for analysis.
- **Preprocessing** steps like DC Shift correction and normalization.
- **Steady Frame Selection** to identify frames with consistent energy levels.
- **LPC Coefficient Calculation** to extract features that represent the vocal tract's characteristics during vowel production.

Each vowel is processed to compute LPC coefficients and auto-correlation values, providing insights into the phonetic structure of the sounds.

## Conclusion

This repository is a comprehensive collection of speech processing assignments that explore fundamental and advanced techniques in the field. The projects demonstrate a blend of signal processing, feature extraction, and classification, paving the way for more complex speech analysis and recognition systems.

---