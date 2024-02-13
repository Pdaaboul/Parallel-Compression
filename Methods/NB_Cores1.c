#include "../headers.h"
#include <stdlib.h>
#include <sys/wait.h>  // For waitpid
#include <time.h>      // For timing

int compressFile(const char* source) {
    char command[1024];
    snprintf(command, sizeof(command), "gzip -k %s", source);
    int status = system(command);
    if (status != 0) {
        fprintf(stderr, "gzip failed for file: %s\n", source);
        return -1;
    }
    return 0;
}

int cleanupCompressedFiles(const char* directory) {
    // First attempt to remove all .gz files
    char cleanupCommand[1024];
    snprintf(cleanupCommand, sizeof(cleanupCommand), "rm -rf %s/*.gz", directory);
    int status = system(cleanupCommand);
    if (status != 0) {
        fprintf(stderr, "Failed to cleanup compressed files in directory: %s\n", directory);
    }
    
    // Specifically target .DS_Store.gz for removal
    char dsStoreCleanupCmd[1024];
    snprintf(dsStoreCleanupCmd, sizeof(dsStoreCleanupCmd), "rm -f %s/.DS_Store.gz", directory);
    status = system(dsStoreCleanupCmd);
    if (status != 0) {
        fprintf(stderr, "Failed to remove .DS_Store.gz in directory: %s\n", directory);
        return -1;  // Return error if unable to remove
    }

    return 0;  // Return success if all cleanup commands executed
}



int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Provide the folder path containing the data files\n");
        return -1;
    }

    int nbCores = getNumCPUs();
    printf("You have %d cores on your machine\n", nbCores);

    char **filesList = listFiles(argv[1]);
    if (filesList == NULL || *filesList == NULL) {
        printf("No files found in the directory\n");
        return -1;
    }

    char *sourceFile;
    char **filesListStart = filesList;
    pid_t pids[nbCores];  // Array to keep track of child process IDs
    int activeChildren = 0;

    // Start the timer
    time_t startTime = time(NULL);

    while (*filesList != NULL) {
        for (int i = 0; i < nbCores && *filesList != NULL; i++) {
            sourceFile = *filesList;
            pid_t pid = fork();

            if (pid == -1) {
                perror("Failed to fork");
                return -1;
            } else if (pid == 0) {
                // Child process
                printf("Compressing: %s\n", sourceFile); // Print which file is being compressed
                compressFile(sourceFile);
                exit(0);  // Exit after compression
            } else {
                // Parent process
                pids[i] = pid;  // Store the child PID
                filesList++;  // Move to the next file
                activeChildren++;
            }

            free(sourceFile);  // Free the memory allocated for the file name
        }

        // Wait for all child processes in the current batch to finish
        for (int i = 0; i < activeChildren; i++) {
            waitpid(pids[i], NULL, 0);
        }
        activeChildren = 0;  // Reset the count for the next batch
    }

    // Stop the timer and calculate the elapsed time
    time_t endTime = time(NULL);
    double totalTime = difftime(endTime, startTime);
    printf("Total time taken to compress all files: %.2f seconds\n", totalTime);

    free(filesListStart);  // Free the list itself

    // Cleanup compressed files
    if (cleanupCompressedFiles(argv[1]) != 0) {
        fprintf(stderr, "Cleanup of compressed files failed\n");
    } else {
        printf("Cleanup of compressed files successful\n");
    }

    return 0;
}
