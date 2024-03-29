#include "../headers.h" 
#include <stdlib.h>
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
    char **filesListStart = filesList; // Keep the start of the list for freeing memory later
    
    // Start the timer
    time_t startTime = time(NULL);

    // Loop through each file and compress it
    while ((sourceFile = *filesList) != NULL) {
        printf("Compressing: %s\n", sourceFile);
        
        if (compressFile(sourceFile) != 0) {
            fprintf(stderr, "Failed to compress file: %s\n", sourceFile);
        } else {
            printf("Successfully compressed: %s\n", sourceFile);
        }

        free(sourceFile);  // Free the memory allocated for the file name
        filesList++;
    }

    // Stop the timer and calculate the elapsed time
    time_t endTime = time(NULL);
    int totalTime = (int)(endTime - startTime);
    printf("Total time taken to compress all files: %d seconds\n", totalTime);

    free(filesListStart);  // Free the list itself. Use the original pointer to the start of the array.

    // Cleanup compressed files
    if (cleanupCompressedFiles(argv[1]) != 0) {
        fprintf(stderr, "Cleanup of compressed files failed\n");
    } else {
        printf("Cleanup of compressed files successful\n");
    }

    return 0;
}
