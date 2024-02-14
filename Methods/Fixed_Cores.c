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

    char **filesList = listFiles(argv[1]);
    int totalFiles = countFiles(argv[1]);

    if (totalFiles <= 0) {
        printf("No files found in the directory\n");
        exit(-1);
    }

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
        cleanupCompressedFiles(argv[1]);
    }

    // Free allocated memory for the file list
    for (int i = 0; filesList[i] != NULL; i++) {
        free(filesList[i]);
    }
    free(filesList);

    return 0;
}
