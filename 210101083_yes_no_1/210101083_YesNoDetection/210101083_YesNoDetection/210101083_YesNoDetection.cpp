#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <string>
#include <cmath>
#include <climits>
using namespace std;

// File names for processing
const string AMBIENT_NOISE_FILE = "ambient_noise.txt";
const string WORD_FILE = "word.txt";
const string OUTPUT_FILE = "output_file.txt";

// Processing constants
const int NOISE_CAPTURE_DURATION = 10000; // Duration to capture ambient noise
const int FRAME_SIZE = 300;               // Size of the frame for analysis
const int MIN_WORD_LENGTH = 2000;         // Minimum length of the detected word
const int NOISE_THRESHOLD_DURATION = 50;  // Threshold duration for noise in the word

// Global variables for processing
int noiseLevel = INT_MIN; // To store the maximum noise level
int normalizationValue;   // Normalization factor for amplitude values
long double dcShift;      // DC shift value for amplitude normalization
int frameNumber = 1;      // Frame counter
long double totalEnergySum = 0; // Sum of energy over all frames
long double totalZCR = 0;       // Sum of Zero Crossing Rate over all frames

// Function to extract ambient noise data and determine noise level
void processAmbientNoise(const string& filename) {
    ifstream inputFile(filename);
    ofstream outputFile(AMBIENT_NOISE_FILE);

    if (!inputFile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }
    if (!outputFile.is_open()) {
        cerr << "Error: Could not open output file " << AMBIENT_NOISE_FILE << endl;
        return;
    }

    int value;
    int sampleIndex = 1;

    // Capture ambient noise for the specified duration
    while (inputFile >> value && sampleIndex < NOISE_CAPTURE_DURATION) {
        outputFile << value << endl;
        noiseLevel = max(noiseLevel, abs(value));
        sampleIndex++;
    }

    inputFile.close();
    outputFile.close();
}

// Function to extract the word from the input data
void extractWordData(const string& filename) {
    ifstream inputFile(filename);
    ofstream outputFile(WORD_FILE, ios::trunc);

    if (!inputFile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return;
    }
    if (!outputFile.is_open()) {
        cerr << "Error: Could not open output file " << WORD_FILE << endl;
        return;
    }

    int value;
    bool wordDetected = false;
    int wordLength = 0;
    int noiseLength = 0;

    // Detect and extract the word based on noise level
    while (inputFile >> value) {
        if (wordDetected) {
            if (abs(value) < noiseLevel) {
                if (wordLength < MIN_WORD_LENGTH) {
                    wordDetected = false;
                    outputFile.close();
                    outputFile.open(WORD_FILE, ios::trunc);
                } else {
                    if (noiseLength < NOISE_THRESHOLD_DURATION) {
                        outputFile << value << endl;
                        wordLength++;
                        noiseLength++;
                    } else {
                        break;
                    }
                }
            } else {
                noiseLength = 0;
                outputFile << value << endl;
                wordLength++;
            }
        } else {
            if (abs(value) > noiseLevel) {
                wordDetected = true;
                outputFile << value << endl;
                wordLength++;
            }
        }
    }

    inputFile.close();
    outputFile.close();
}

// Function to calculate normalization value (maximum amplitude) from the word file
int calculateNormalizationValue(const string& filename) {
    ifstream inputFile(filename);

    if (!inputFile.is_open()) {
        cerr << "Error: Could not open file " << filename << endl;
        return -1;
    }

    int value;
    int maxAmplitude = INT_MIN;

    // Find the maximum absolute amplitude value for normalization
    while (inputFile >> value) {
        maxAmplitude = max(maxAmplitude, abs(value));
    }

    inputFile.close();
    return maxAmplitude;
}

// Function to calculate DC shift value from ambient noise data
long double calculateDCShift() {
    ifstream inputFile(AMBIENT_NOISE_FILE);

    if (!inputFile.is_open()) {
        cerr << "Error: Could not open file " << AMBIENT_NOISE_FILE << endl;
        return -1;
    }

    int value;
    long double sum = 0;
    int count = 0;

    // Calculate the mean (DC shift) of the noise samples
    while (inputFile >> value) {
        sum += value;
        count++;
    }

    inputFile.close();
    return sum / count;
}

// Function to calculate energy and Zero Crossing Rate (ZCR) for each frame
void calculateEnergyAndZCR(const string& filename) {
    ifstream inputFile(WORD_FILE);
    ofstream outputFile(OUTPUT_FILE);

    if (!inputFile.is_open()) {
        cerr << "Error: Could not open input file " << WORD_FILE << endl;
        return;
    }
    if (!outputFile.is_open()) {
        cerr << "Error: Could not open output file " << OUTPUT_FILE << endl;
        return;
    }

    int value;
    int previousValue = 0;
    long double modulatedAmplitude;
    int frameIndex = 0;
    int zcrCount = 0;
    long double energySum = 0;

    while (inputFile >> value) {
        frameIndex++;

        // Normalize and modulate the amplitude
        modulatedAmplitude = (value - dcShift) * 5000 / normalizationValue;

        // Calculate Zero Crossing Rate
        if ((previousValue < 0 && value > 0) || (previousValue > 0 && value < 0)) {
            zcrCount++;
            totalZCR++;
        }

        // Calculate energy
        int instantEnergy = modulatedAmplitude * modulatedAmplitude;
        energySum += instantEnergy;
        totalEnergySum += instantEnergy;

        // When a frame is complete, write its energy and ZCR to the output file
        if (frameIndex == FRAME_SIZE) {
            outputFile << frameNumber << " " << energySum / frameIndex << " " << zcrCount << endl;
            frameNumber++;
            energySum = 0;
            frameIndex = 0;
            zcrCount = 0;
        }

        previousValue = value;
    }

    // Write the remaining frame data
    outputFile << frameNumber << " " << energySum / frameIndex << " " << zcrCount << endl;

    inputFile.close();
    outputFile.close();
}

// Function to classify the detected word based on energy and ZCR
void classifyWord() {
    long double averageEnergy = totalEnergySum / frameNumber;
    long double averageZCR = totalZCR / frameNumber;

    cout << "Average Energy: " << averageEnergy << endl;
    cout << "Average ZCR: " << averageZCR << endl;

    cout << "The Word is: ";
    if (averageZCR < 25) {
        cout << "No" << endl;
    } else {
        cout << "Yes" << endl;
    }
}

int main(int argc, char* argv[]) {
    string inputFilename;
    cout << "Enter the input filename: ";
    cin >> inputFilename;

    // Step 1: Compute the ambient noise
    processAmbientNoise(inputFilename);

    // Step 2: Extract the word from the input data
    extractWordData(inputFilename);

    // Step 3: Calculate normalization value based on the word data
    normalizationValue = calculateNormalizationValue(inputFilename);

    // If there was an error in file processing, exit
    if (normalizationValue < 0) return 0;

    // Step 4: Calculate DC shift based on ambient noise
    dcShift = calculateDCShift();

    // Step 5: Calculate energy and ZCR of the word frames
    calculateEnergyAndZCR(inputFilename);

    // Step 6: Classify the word based on energy and ZCR
    classifyWord();

    return 0;
}
