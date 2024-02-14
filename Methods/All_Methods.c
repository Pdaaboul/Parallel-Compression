#include "../headers.h"
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

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
void sequentialCompression(char **filesList, const char* directory) {
    printf("Starting sequential compression \n");
    char *sourceFile;
    // Start the timer
    time_t startTime = time(NULL);

    // Loop through each file and compress it
    for (char **currentFile = filesList; *currentFile != NULL; currentFile++) {
        printf("Compressing: %s\n", *currentFile);
        if (compressFile(*currentFile) != 0) {
            fprintf(stderr, "Failed to compress file: %s\n", *currentFile);
        } else {
            printf("Successfully compressed: %s\n", *currentFile);
        }
    }

    // Calculate the elapsed time
    time_t endTime = time(NULL);
    printf("Total time taken to compress all files: %.2f seconds\n", difftime(endTime, startTime));

    // Cleanup compressed files
    cleanupCompressedFiles(directory); // Using directory from argv[1]
}
void nParallelCompression(char **filesList, const char* directory) {
    printf("Starting N-Parallel compression...\n");
    time_t startTime = time(NULL);

    int numFiles = 0;
    for (char **current = filesList; *current; current++) numFiles++;
    pid_t pids[numFiles];
    
    for (int i = 0; i < numFiles; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {
            compressFile(filesList[i]);
            exit(0);
        }
    }

    while (numFiles > 0) {
        wait(NULL);
        numFiles--;
    }

    time_t endTime = time(NULL);
    printf("N-Parallel compression completed in %.2f seconds.\n", difftime(endTime, startTime));

    cleanupCompressedFiles(directory);
}


void nbCoresBatchCompression(char **filesList, const char* directory) {
    printf("Starting NB_CORES batch compression...\n");
    int nbCores = getNumCPUs();
    printf("Utilizing %d cores...\n", nbCores);

    char *sourceFile;
    pid_t pids[nbCores];
    int activeChildren = 0;

    // Start the timer
    time_t startTime = time(NULL);

    while (*filesList != NULL) {
        for (int i = 0; i < nbCores && *filesList != NULL; i++) {
            sourceFile = *filesList++;
            pids[i] = fork();
            if (pids[i] == -1) {
                perror("Failed to fork");
                continue;
            }
            if (pids[i] == 0) {
                // Child process
                printf("Compressing: %s\n", sourceFile);
                if (compressFile(sourceFile) != 0) {
                    fprintf(stderr, "Failed to compress file: %s\n", sourceFile);
                }
                exit(0);  // Exit after compression
            } else {
                // Parent process continues to fork until nbCores is reached
                activeChildren++;
            }
            // Free the memory allocated for the file name if needed
        }

        // Parent waits for all child processes in the current batch to finish
        while (activeChildren > 0) {
            wait(NULL);  // Wait for any child process to finish
            activeChildren--;
        }
    }

    // Calculate the elapsed time
    time_t endTime = time(NULL);
    printf("NB_CORES batch compression completed in %.2f seconds.\n", difftime(endTime, startTime));

    // Cleanup compressed files
    cleanupCompressedFiles(directory);
}

   

void nbCoresEqualCompression(char **filesList, const char* directory, int totalFiles) {
    printf("Starting NB_CORES equal compression...\n");
    int nbCores = getNumCPUs();
    int filesPerProcess = totalFiles / nbCores;
    int extraFiles = totalFiles % nbCores;

    time_t startTime = time(NULL);
    pid_t pids[nbCores];

    for (int i = 0; i < nbCores; i++) {
        pids[i] = fork();
        if (pids[i] == 0) {  // Child process
            int startIdx = i * filesPerProcess + (i < extraFiles ? i : extraFiles);
            int endIdx = startIdx + filesPerProcess + (i < extraFiles ? 1 : 0);
            for (int j = startIdx; j < endIdx; j++) {
                if (compressFile(filesList[j]) != 0) {
                    fprintf(stderr, "Failed to compress file: %s\n", filesList[j]);
                }
            }
            exit(0);
        }
    }

    // Wait for all children to finish
    while (wait(NULL) > 0);

    time_t endTime = time(NULL);
    printf("NB_CORES equal compression completed in %.2f seconds.\n", difftime(endTime, startTime));

    cleanupCompressedFiles(directory);
}
void fixedCoresParallelCompression(char **filesList, const char* directory, int totalFiles) {
    int nbCores = getNumCPUs();
    for (int numProcesses = 2; numProcesses <= nbCores; numProcesses++) {
        printf("Starting fixed %d-Parallel compression...\n", numProcesses);
        time_t startTime = time(NULL);

        pid_t pids[numProcesses];
        int filesPerProcess = totalFiles / numProcesses;
        int extraFiles = totalFiles % numProcesses;

        for (int i = 0; i < numProcesses; i++) {
            pids[i] = fork();
            if (pids[i] == 0) { // Child process
                int start = i * filesPerProcess + (i < extraFiles ? i : extraFiles);
                int end = start + filesPerProcess + (i < extraFiles ? 1 : 0);
                for (int j = start; j < end; j++) {
                    compressFile(filesList[j]);
                }
                exit(0); // Child process exits after compressing its subset of files
            }
        }

        // Wait for all child processes to finish
        for (int i = 0; i < numProcesses; i++) {
            waitpid(pids[i], NULL, 0);
        }

        time_t endTime = time(NULL);
        printf("Fixed %d-Parallel compression completed in %.2f seconds.\n", numProcesses, difftime(endTime, startTime));

        // Cleanup compressed files
        cleanupCompressedFiles(directory);
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Error: Provide the directory path as an argument.\n");
        return -1;
    }

    char **filesList = listFiles(argv[1]);
    if (!filesList || !*filesList) {
        printf("Error: No files found in the directory.\n");
        return -1;
    }

    int totalFiles = 0;
    for (char **current = filesList; *current != NULL; current++) {
        totalFiles++;
    }

    // Call compression methods
    // sequentialCompression(filesList, argv[1]);
    // nParallelCompression(filesList, argv[1]);
    nbCoresBatchCompression(filesList, argv[1]);
    nbCoresEqualCompression(filesList, argv[1], totalFiles);
    fixedCoresParallelCompression(filesList, argv[1], totalFiles);

    // Cleanup
    for (char **currentFile = filesList; *currentFile; currentFile++) {
        free(*currentFile);
    }
    free(filesList);

    return 0;
}