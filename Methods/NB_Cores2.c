#include "../headers.h" // Ensure this header contains prototypes for getNumCPUs, listFiles, and countFiles
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>

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
        exit(-1);
    }

    int nbCores = getNumCPUs();
    printf("You have %d cores on your machine\n", nbCores);

    char **filesList = listFiles(argv[1]);
    int totalFiles = countFiles(argv[1]);

    if (totalFiles <= 0) {
        printf("No files found in the directory\n");
        exit(-1);
    }

    int filesPerProcess = totalFiles / nbCores;
    int remainingFiles = totalFiles % nbCores;

    // Start the timer
    time_t startTime = time(NULL);

    for (int i = 0; i < nbCores; i++) {
        pid_t pid = fork();
        if (pid == 0) {  // Child process
            int start = i * filesPerProcess + (i < remainingFiles ? i : remainingFiles);
            int end = start + filesPerProcess + (i < remainingFiles ? 1 : 0);

            for (int j = start; j < end && filesList[j] != NULL; j++) {
                printf("Compressing: %s\n", filesList[j]);
                if (compressFile(filesList[j]) != 0) {
                    fprintf(stderr, "Failed to compress file: %s\n", filesList[j]);
                }
            }
            exit(0);  // Child exits after completing its assigned files
        }
    }

    // Parent waits for all children to finish
    int status;
    while (wait(&status) > 0);

    // Stop the timer and calculate the elapsed time
    time_t endTime = time(NULL);
    printf("Total time taken to compress all files: %.2f seconds\n", difftime(endTime, startTime));

    // Cleanup compressed files
    if (cleanupCompressedFiles(argv[1]) != 0) {
        fprintf(stderr, "Cleanup of compressed files failed\n");
    } else {
        printf("Cleanup of compressed files successful\n");
    }

    // Free allocated memory for the file list
    for (int i = 0; filesList[i] != NULL; i++) {
        free(filesList[i]);
    }
    free(filesList);

    return 0;
}
