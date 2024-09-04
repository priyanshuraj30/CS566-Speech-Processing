#include "stdafx.h"
#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <cmath>
#include <climits>
using namespace std;

#pragma comment(lib, "winmm.lib")

// Constants for recording and processing
const int NUMPTS = 16025 * 3; // Number of points for 3 seconds of recording
short int waveIn[NUMPTS]; // Array to hold the recorded audio data
const string AMBIENT_NOISE_FILE = "ambient_noise.txt"; // File to store ambient noise data
const string WORD_FILE = "word.txt"; // File to store the word data
const string OUTPUT_FILE = "output_file.txt"; // File to store the calculated energy and ZCR
const int NOISE_CAPTURE_DURATION = 10000; // Duration to capture ambient noise
const int FRAME_SIZE = 300; // Frame size for processing
const int MIN_WORD_LENGTH = 2000; // Minimum word length to consider as valid word
const int NOISE_THRESHOLD_DURATION = 50; // Threshold duration to consider silence between words

// Variables for processing
int noiseLevel = INT_MIN; // To store the maximum ambient noise level
int normalizationValue; // Maximum amplitude in the word data for normalization
long double dcShift; // DC shift value calculated from ambient noise
int frameNumber = 1; // Frame counter for energy and ZCR calculation
long double totalEnergySum = 0; // Total energy sum across all frames
long double totalZCR = 0; // Total Zero Crossing Rate (ZCR) across all frames

// Function declarations
void StartRecord(); // Function to start recording audio
void processAmbientNoise(); // Function to process and store ambient noise data
void extractWordData(); // Function to extract the word data from the recorded audio
int calculateNormalizationValue(); // Function to calculate the normalization value (max amplitude)
long double calculateDCShift(); // Function to calculate the DC shift from ambient noise data
void calculateEnergyAndZCR(); // Function to calculate energy and ZCR for each frame
void classifyWord(); // Function to classify the recorded word as "Yes" or "No"

// Function to start recording audio
void StartRecord()
{
    int sampleRate = 16025; // Sampling rate for the recording
    HWAVEIN hWaveIn; // Handle for wave input
    MMRESULT result; // To store the result of waveIn functions
    WAVEFORMATEX pFormat; // Format for recording
    pFormat.wFormatTag = WAVE_FORMAT_PCM; // PCM format
    pFormat.nChannels = 1; // Mono channel
    pFormat.nSamplesPerSec = sampleRate; // Sample rate
    pFormat.nAvgBytesPerSec = sampleRate * 2; // Average bytes per second
    pFormat.nBlockAlign = 2; // Block alignment
    pFormat.wBitsPerSample = 16; // 16-bit samples
    pFormat.cbSize = 0; // Size of extra format information

    // Open the wave input device with the specified format
    result = waveInOpen(&hWaveIn, WAVE_MAPPER, &pFormat, 0L, 0L, WAVE_FORMAT_DIRECT);
    WAVEHDR WaveInHdr; // Wave header for recording
    WaveInHdr.lpData = (LPSTR)waveIn; // Pointer to the buffer where audio data will be stored
    WaveInHdr.dwBufferLength = NUMPTS * 2; // Buffer length in bytes
    WaveInHdr.dwBytesRecorded = 0;
    WaveInHdr.dwUser = 0L;
    WaveInHdr.dwFlags = 0L;
    WaveInHdr.dwLoops = 0L;

    // Prepare the header and start recording
    waveInPrepareHeader(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
    result = waveInAddBuffer(hWaveIn, &WaveInHdr, sizeof(WAVEHDR));
    result = waveInStart(hWaveIn);
    Sleep(3 * 1000); // Record for 3 seconds
    waveInClose(hWaveIn); // Close the wave input device
}

// Function to process and store ambient noise data
void processAmbientNoise()
{
    ofstream outputFile(AMBIENT_NOISE_FILE); // Open file to store ambient noise data
    int sampleIndex = 0;
    for (int i = 0; i < NUMPTS && sampleIndex < NOISE_CAPTURE_DURATION; ++i)
    {
        outputFile << waveIn[i] << endl; // Write each sample to the file
        noiseLevel = max(noiseLevel, abs(waveIn[i])); // Update noise level with the maximum value
        sampleIndex++;
    }
    outputFile.close(); // Close the file
}

// Function to extract the word data from the recorded audio
void extractWordData()
{
    ofstream outputFile(WORD_FILE, ios::trunc); // Open file to store word data, truncating existing content
    bool wordDetected = false; // Flag to indicate if a word is detected
    int wordLength = 0; // Length of the detected word
    int noiseLength = 0; // Length of noise/silence between words

    for (int i = 0; i < NUMPTS; ++i)
    {
        int value = waveIn[i]; // Get the current sample value
        if (wordDetected)
        {
            if (abs(value) < noiseLevel) // If the current value is less than noise level
            {
                if (wordLength < MIN_WORD_LENGTH) // If the detected word is too short
                {
                    wordDetected = false; // Reset word detection
                    outputFile.close(); // Close and reopen the file to truncate it
                    outputFile.open(WORD_FILE, ios::trunc);
                }
                else
                {
                    if (noiseLength < NOISE_THRESHOLD_DURATION) // Check if the silence duration is within threshold
                    {
                        outputFile << value << endl; // Write the value to the word file
                        wordLength++;
                        noiseLength++;
                    }
                    else
                    {
                        break; // If silence is too long, stop recording the word
                    }
                }
            }
            else
            {
                noiseLength = 0; // Reset noise length if we get a non-noise value
                outputFile << value << endl; // Write the value to the word file
                wordLength++;
            }
        }
        else
        {
            if (abs(value) > noiseLevel) // If a value greater than noise level is detected
            {
                wordDetected = true; // Start detecting the word
                outputFile << value << endl; // Write the value to the word file
                wordLength++;
            }
        }
    }
    outputFile.close(); // Close the word file
}

// Function to calculate the normalization value (max amplitude) from the word data
int calculateNormalizationValue()
{
    ifstream inputFile(WORD_FILE); // Open the word file for reading
    int maxAmplitude = INT_MIN; // Initialize maximum amplitude
    int value;

    while (inputFile >> value) // Read each value from the word file
    {
        maxAmplitude = max(maxAmplitude, abs(value)); // Update maximum amplitude
    }
    inputFile.close(); // Close the word file
    return maxAmplitude;
}

// Function to calculate the DC shift from ambient noise data
long double calculateDCShift()
{
    ifstream inputFile(AMBIENT_NOISE_FILE); // Open the ambient noise file for reading
    long double sum = 0; // Initialize sum of noise values
    int count = 0; // Initialize count of noise samples
    int value;

    while (inputFile >> value) // Read each value from the noise file
    {
        sum += value; // Add the value to the sum
        count++; // Increment the count
    }
    inputFile.close(); // Close the noise file
    return sum / count; // Return the average (DC shift)
}

// Function to calculate energy and Zero Crossing Rate (ZCR) for each frame of word data
void calculateEnergyAndZCR()
{
    ifstream inputFile(WORD_FILE); // Open the word file for reading
    ofstream outputFile(OUTPUT_FILE); // Open the output file to store energy and ZCR
    int value;
    int previousValue = 0;
    long double modulatedAmplitude;
    int frameIndex = 0; // Index to keep track of frames
    int zcrCount = 0; // Zero Crossing Rate (ZCR) counter
    long double energySum = 0; // Energy sum for the current frame

    while (inputFile >> value) // Read each value from the word file
    {
        frameIndex++;
        // Modulate the amplitude with DC shift and normalization
        modulatedAmplitude = (value - dcShift) * 5000 / normalizationValue;

        // Check for zero crossings
        if ((previousValue < 0 && value > 0) || (previousValue > 0 && value < 0))
        {
            zcrCount++;
            totalZCR++;
        }

        // Calculate instant energy
        int instantEnergy = modulatedAmplitude * modulatedAmplitude;
        energySum += instantEnergy;
        totalEnergySum += instantEnergy;

        // If the frame is complete, write the energy and ZCR to the output file
        if (frameIndex == FRAME_SIZE)
        {
            outputFile << frameNumber << " " << energySum / frameIndex << " " << zcrCount << endl;
            frameNumber++;
            energySum = 0;
            frameIndex = 0;
            zcrCount = 0;
        }
        previousValue = value; // Update previous value
    }

    // Write the remaining data to the output file
    outputFile << frameNumber << " " << energySum / frameIndex << " " << zcrCount << endl;
    inputFile.close(); // Close the word file
    outputFile.close(); // Close the output file
}

// Function to classify the recorded word based on energy and ZCR
void classifyWord()
{
    long double averageEnergy = totalEnergySum / frameNumber; // Calculate average energy
    long double averageZCR = totalZCR / frameNumber; // Calculate average ZCR
    cout << "Average Energy: " << averageEnergy << endl;
    cout << "Average ZCR: " << averageZCR << endl;
    cout << "The Word is: ";
    // Classify the word as "Yes" or "No" based on ZCR
    if (averageZCR < 25)
    {
        cout << "No" << endl;
    }
    else
    {
        cout << "Yes" << endl;
    }
}

// Main function
int _tmain(int argc, _TCHAR* argv[])
{
    cout << "Recording ambient noise..." << endl;
    StartRecord(); // Start recording ambient noise
    processAmbientNoise(); // Process the recorded ambient noise

    cout << "Recording word data..." << endl;
    StartRecord(); // Start recording word data
    extractWordData(); // Extract the word data

    // Calculate normalization value and DC shift
    normalizationValue = calculateNormalizationValue();
    if (normalizationValue < 0) return 0;

    dcShift = calculateDCShift();
    calculateEnergyAndZCR(); // Calculate energy and ZCR
    classifyWord(); // Classify the word

    return 0;
}
