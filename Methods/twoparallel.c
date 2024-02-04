#include "../headers.h"  
#include <stdlib.h>
#include <time.h>
#include <sys/wait.h>

int compressFile(const char* source) {
    char command[1024];  // Adjust size as needed
    snprintf(command, sizeof(command), "gzip -k %s", source);

    int status = system(command);
    if (status != 0) {
        fprintf(stderr, "gzip failed for file: %s\n", source);
        return -1;
    }

    return 0;
}


int cleanupCompressedFiles(const char* directory) {
    char cleanupCommand[1024];
    snprintf(cleanupCommand, sizeof(cleanupCommand), "rm -rf %s/*.gz", directory);
    int status = system(cleanupCommand);
    if (status != 0) {
        fprintf(stderr, "Failed to cleanup compressed files in directory: %s\n", directory);
        return -1;
    }
    return 0;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Provide the folder path containing the data files\n");
        return -1;
    }

    char **filesList = listFiles(argv[1]);
    if (filesList == NULL || *filesList == NULL) {
        printf("No files found in the directory\n");
        return -1;
    }

    char *sourceFile;
    char **filesListStart = filesList;
    
    // Start the timer
    time_t startTime = time(NULL);

    // Loop through each file and compress it
    while ((sourceFile = *filesList) != NULL) {
        pid_t pid = fork(); // Create a new process
        if (pid == -1) {
            // Error handling
            perror("Failed to fork");
            return -1;
        } else if (pid == 0) {
            // Child process
            printf("Compressing: %s\n", sourceFile);
            if (compressFile(sourceFile) != 0) {
                fprintf(stderr, "Failed to compress file: %s\n", sourceFile);
            } else {
                printf("Successfully compressed: %s\n", sourceFile);
            }
            exit(0);  // Exit the child process
        } else {
            // Parent process
            filesList++;  // Move to the next file
        }

        free(sourceFile);  // Free the memory allocated for the file name
    }

    // Wait for all child processes to finish
    while ((waitpid(-1, NULL, 0)) > 0);

    // Stop the timer and calculate the elapsed time
    time_t endTime = time(NULL);
    int totalTime = (int)(endTime - startTime);
    printf("Total time taken to compress all files: %d seconds\n", totalTime);

    free(filesListStart);  // Free the list itself

    // Cleanup compressed files
    if (cleanupCompressedFiles(argv[1]) != 0) {
        fprintf(stderr, "Cleanup of compressed files failed\n");
    } else {
        printf("Cleanup of compressed files successful\n");
    }

    return 0;
}