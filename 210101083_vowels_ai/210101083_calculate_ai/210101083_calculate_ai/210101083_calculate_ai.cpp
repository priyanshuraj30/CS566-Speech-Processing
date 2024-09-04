#include "stdafx.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "float.h"
#include <algorithm>
#include <vector>

// Function to compute autocorrelation coefficients (R_i)
double* Compute_Ri(double *sample_data, double *R_ref) {
    int i = 0, j = 0;
    double sum = 0.0;

    // Loop over lags from 0 to 12 (R_0 to R_12)
    for (i = 0; i <= 12; i++) {
        sum = 0.0;

        // Compute the sum of products for autocorrelation
        // R_i = sum of (sample_data[j] * sample_data[j + i])
        for (j = 0; j < 320 - i; j++) {
            sum += (sample_data[j] * sample_data[j + i]);
        }

        // Store the computed value in R_ref array
        R_ref[i] = sum;
    }

    // Return the reference to the autocorrelation coefficients array
    return R_ref;
}

// Function to compute Linear Predictive Coding (LPC) coefficients (A_i) using Durbin's algorithm
double* Compute_Ai(double *A_ref, double *R) {
    double alpha[13][13] = {0};  // Reflection coefficients
    double k[13] = {0};          // Prediction error
    double E[13] = {0};          // Prediction error energy
    double sum = 0.0;
    int i = 0, j = 0;

    // Initialize the energy for lag 0
    E[0] = R[0];

    // Loop to compute LPC coefficients from lag 1 to 12
    for (i = 1; i <= 12; i++) {

        // Compute reflection coefficient for lag 1
        if (i == 1) {
            k[1] = R[1] / R[0];
        } 
        // Compute reflection coefficient for lags 2 to 12
        else {
            sum = 0.0;

            // Calculate the sum of alpha * R values for the current lag
            for (j = 1; j <= i - 1; j++) {
                sum += (alpha[i - 1][j] * R[i - j]);
            }

            // Calculate the reflection coefficient k_i
            k[i] = ((R[i] - sum) / E[i - 1]);
        }

        // Set the diagonal element in the alpha array
        alpha[i][i] = k[i];

        // Update the off-diagonal elements in the alpha array
        for (int j = 1; j <= i - 1; j++) {
            alpha[i][j] = alpha[i - 1][j] - (k[i] * alpha[i - 1][i - j]);
        }

        // Update the prediction error energy
        E[i] = (1 - (k[i] * k[i])) * E[i - 1];
    }

    // Copy the final LPC coefficients from alpha to A_ref
    for (int j = 1; j <= 12; j++) {
        A_ref[j] = alpha[12][j];
    }

    // Return the reference to the LPC coefficients array
    return A_ref;
}

// Following function will be used for computing DC shift values of the recording samples of each vowel
/*double Compute_dc_shift() {
    FILE *fp;
    char ch;
    int count = 0;
    double value = 0, sum = 0.0;
    double shift = 0.0;

    fp = fopen("dc_shift_file.txt", "r");
    if (fp == NULL) {
        printf("Error opening dc_shift_file.txt\n");
        return 0.0;
    }

    while (!feof(fp)) {
        fscanf(fp, "%lf", &value);
        sum += value;
    }

    fseek(fp, 0, SEEK_SET);

    for (ch = getc(fp); ch != EOF; ch = getc(fp)) {
        if (ch == '\n') count++;
    }
    fclose(fp);

    shift = (sum / count);
    return shift;
}*/

// Following function is used to process the 'test.txt' file
void ProcessVowel(const char* filename, const char* ai_filename) {
    double sample_data[320];  // Array to store 320 samples from the input file
    double R_ref[13];         // Array to store autocorrelation coefficients (R_0 to R_12)
    double A_ref[13];         // Array to store LPC coefficients (A_1 to A_12)

    // Open the input file containing the vowel samples
    FILE *file_pointer = fopen(filename, "r");
    if (file_pointer == NULL) {
        printf("Error in opening file: %s\n", filename);  // Print error if file cannot be opened
        return;  // Exit the function
    }

    // Read the first 320 samples from the file into sample_data array
    int i = 0;
    while (i < 320 && !feof(file_pointer)) {
        fscanf(file_pointer, "%lf", &sample_data[i]);
        i++;
    }
    fclose(file_pointer);  // Close the input file

    // Perform DC shift correction
    //double dc_shift = Compute_dc_shift();
    double dc_shift = 0.0;

    // Find the maximum value in the sample data for normalization
    double max_value = *std::max_element(sample_data, sample_data + 320);

    // Normalize the sample data and apply DC shift correction
    for (int i = 0; i < 320; i++) {
        sample_data[i] = ((sample_data[i] - dc_shift) * 5000.0) / max_value;
    }

    // Compute autocorrelation coefficients (R_i) using the sample data
    double *R = Compute_Ri(sample_data, R_ref);

    // Compute LPC coefficients (A_i) using the autocorrelation coefficients (R_i)
    double *A = Compute_Ai(A_ref, R);

    // Open the output file to save the computed LPC coefficients (A_i)
    FILE *ai_file = fopen(ai_filename, "w");
    if (ai_file == NULL) {
        printf("Error in creating AI file: %s\n", ai_filename);  // Print error if file cannot be opened
        return;  // Exit the function
    }

    // Write the LPC coefficients (A_1 to A_12) to the output file
    for (i = 1; i <= 12; i++) {
        fprintf(ai_file, "%lf\n", A[i]);
    }
    fclose(ai_file);  // Close the output file
}


// Following commented part of code will be used for the processing of recording samples of each vowel
/*-----------------------------------------------------------------------------------------------------------------------------------
std::vector<int> findSteadyFrames(double *energy, int frame_count, double threshold) {
    std::vector<int> steadyFrames;
    for (int i = 0; i < frame_count; i++) {
        if (energy[i] > threshold) {
            steadyFrames.push_back(i);
        }
    }
    return steadyFrames;
}

double calculateEnergy(double *frame, int frame_size) {
    double energy = 0.0;
    for (int i = 0; i < frame_size; i++) {
        energy += frame[i] * frame[i];
    }
    return energy;
}

void ProcessVowelUsingSteadyPart(const char* filename, const char* ai_filename) {
    double sample_data[320] = { 0 };
    double R_ref[13] = { 0 };
    double A_ref[13] = { 0 };
    double frame_energy[40] = { 0 };  // Assuming 40 frames (8 samples per frame for 320 samples)

    FILE *file_pointer = fopen(filename, "r");
    if (file_pointer == NULL) {
        printf("Error in opening file: %s\n", filename);
        return;
    }

    int i = 0;
    while (i < 320 && fscanf(file_pointer, "%lf", &sample_data[i]) != EOF) {
        i++;
    }
    fclose(file_pointer);

    if (i < 320) {
        printf("Insufficient data in file: %s\n", filename);
        return;
    }

    // Calculate energy for each frame (40 frames, 8 samples each)
    for (int i = 0; i < 40; i++) {
        frame_energy[i] = calculateEnergy(sample_data + i * 8, 8);
    }

    // Set threshold based on the silence frames (you can tweak this part as needed)
    double threshold = (*std::min_element(frame_energy, frame_energy + 5) + *std::min_element(frame_energy + 35, frame_energy + 40)) / 2;

    // Get the steady frames where energy is higher than the threshold
    std::vector<int> steadyFrames = findSteadyFrames(frame_energy, 40, threshold);

    if (steadyFrames.size() < 5) {
        printf("Not enough steady frames found.\n");
        return;
    }

    // Select 5 middle frames from the steady frames
    int start = std::max(0, static_cast<int>(steadyFrames.size() / 2) - 2);
    std::vector<int> selectedFrames(steadyFrames.begin() + start, steadyFrames.begin() + start + 5);

    // Process selected frames
    FILE *ai_file = fopen(ai_filename, "w");
    if (ai_file == NULL) {
        printf("Error in creating AI file: %s\n", ai_filename);
        return;
    }

    for (int idx = 0; idx < 5; idx++) {
        int frame_idx = selectedFrames[idx];
        double *frame_data = sample_data + frame_idx * 8;

        double *R = Compute_Ri(frame_data, R_ref);
        double *A = Compute_Ai(A_ref, R);

        if (A == nullptr) {
            fclose(ai_file);
            return;  // Stop processing on error
        }

        for (i = 1; i <= 12; i++) {
            fprintf(ai_file, "%lf\n", A[i]);
        }
    }

    fclose(ai_file);
}-----------------------------------------------------------------------------------------------------------------------------------*/

int _tmain(int argc, _TCHAR* argv[]) {
    /*char vowels[5] = { 'a', 'e', 'i', 'o', 'u' };
    
    for (int vowel = 0; vowel < 5; vowel++) {
        for (int utterance = 1; utterance <= 20; utterance++) {
            char filename[50];
            char ai_filename[50];
            sprintf(filename, "210101083_%c_%d.txt", vowels[vowel], utterance);
            sprintf(ai_filename, "210101083_%c_%d_ai.txt", vowels[vowel], utterance);

            ProcessVowelUsingSteadyPart(filename, ai_filename);
        }
    }*/

	// Processing the 'test.txt' file and storing the Ai's in '210101083_ai_test.txt' file
	char filename[50];
    char ai_filename[50];
    sprintf(filename, "test.txt");
    sprintf(ai_filename, "210101083_ai_test.txt");

    ProcessVowel(filename, ai_filename);

    printf("Ai values are computed and stored in '210101083_ai_test.txt'.\n");
    system("PAUSE");
    return 0;
}
