#include <stdio.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <libgen.h>
#include <stdlib.h>

#include "ftree.h"
#include "hash.h"

#define BUFFSIZE 1024


/*
 * This function takes string of a path and a file/dir name as inputs,
 * and adds name to the end of the path, and then returns.
 */
char* generate_path(const char *path, char *name) {
    char *result;
    if (path[strlen(path) - 1] != '/') {
        result = malloc(strlen(path) + strlen(name) + 2);
        strcpy(result, path);
        strcat(result, "/");
        strcat(result, name);
    } else {
        result = malloc(strlen(path) + strlen(name) + 1);
        strcpy(result, path);
        strcat(result, name);
    }
    return result;
}


/*
 * This function takes a src and a dest file pointer as inputs,
 * and then copies src to dest file.
 */
void copy_file(FILE *src, FILE *dest) {
    char buffer[BUFFSIZE];
    int n;
    while ((n = fread(buffer, 1, BUFFSIZE, src)) != 0) {
        if (fwrite(buffer, 1, n, dest) != n) {
            break;
        }
    }
}


/*
 * This function takes two hash arrays as inputs, and returns 1 if they are the
 * same, otherwise returns -1.
 */
int check_hash(const char *hash1, const char *hash2) {
    // Compare every element in the array.
    int i;
    for(i = 0; i < 8; i++) {
        if (hash1[i] != hash2[i]) {
            return -1;
        }
    }
    return 1;
}


/*
 * Returns an integer where the magnitude reflects the number of processes used
 * to complete the copy, and its sign indicates whether an error was encountered.
 */
int copy_ftree(const char *src, const char *dest) {
    // num_process keeps track of the number of processes.
    int num_process = 1;
    int neg_flag = 0;
    struct stat stat_src, stat_dest;
    if (lstat(src, &stat_src) == -1) {
        perror(src);
        return (-num_process);
    }
    if (lstat(dest, &stat_dest) == -1) {
        perror(dest);
        return (-num_process);
    }
    // If destination is not a directory, then exit.
    if (!S_ISDIR(stat_dest.st_mode)) {
        fprintf(stderr, "Error, DEST is not a directory.\n");
        return (-num_process);
    }

    // If destination is a directory, we continue executing.
    // If source is a symbolic link, then we just ignore.
    if (S_ISLNK(stat_src.st_mode)) {
        return num_process;
    } else if (S_ISREG(stat_src.st_mode)) { // If source is a file.
        // Create a str that is the corresponding path in destination.
        char *dest_fpath = generate_path(dest, basename((char *)src));

        // Try to open it.
        FILE *f = fopen(dest_fpath, "rb");
        if (f == NULL) { // If an error occurs when we try to open this file.
            if (errno == ENOENT) { // If the file doesn't exist.
                // We copy the file.
                FILE *dest_file;
                if ((dest_file = fopen(dest_fpath,"wb")) == NULL) {
            		perror("fopen");
            		return (-num_process);
            	}
                FILE *src_file;

                if ((src_file = fopen(src,"rb")) == NULL) {
                    if (errno == EACCES) {
                        // If we don't have read permission for source file.
                        perror("fopen");
                        // Change the permission of destination empty file.
                        if (chmod(dest_fpath, (stat_src.st_mode) & 0777) == -1) {
                            perror("chmod");
                            return (-num_process);
                        }
                        return (-num_process);
                    } else {
                        perror("fopen");
                        return (-num_process);
                    }
                }
                copy_file(src_file, dest_file);

                // After copying the file, we change its permission to match
                // the source file.
                if (chmod(dest_fpath, (stat_src.st_mode) & 0777) == -1) {
                    perror("chmod");
                    return (-num_process);
                }
                if (fclose(src_file) == EOF) {
                    perror("fopen");
            		exit(-num_process);
                }
                if (fclose(dest_file) == EOF) {
                    perror("fopen");
            		exit(-num_process);
                }
            } else if (errno == EACCES) { // If we don't have read permission for dest file.
                perror("fopen");
                // Change its permission if possible.
                if (chmod(dest_fpath, (stat_src.st_mode) & 0777) == -1) {
                    perror("chmod");
                    return (-num_process);
                }
                return (-num_process);
            } else { // If other issues happened.
                perror("fopen");
                return (-num_process);
            }

        } else { // If NO error occurs when we try to open this file.
            // This means that this file exits and we have already opened it.
            struct stat stat_file;
            lstat(dest_fpath, &stat_file);
            long src_size = stat_src.st_size;
            long dest_size = stat_file.st_size;

            FILE *src_file;
            // If an error occurs when we try to open this file.
            if ((src_file = fopen(src,"rb")) == NULL) {
                if (errno == EACCES) {
                    // If we don't have read permission for source file.
                    perror("fopen");
                    // Change the permission of destination empty file.
                    if (chmod(dest_fpath, (stat_src.st_mode) & 0777) == -1) {
                        perror("chmod");
                        return (-num_process);
                    }
                    return (-num_process);
                } else {
                    perror("fopen");
                    return (-num_process);
                }
            }

            // If NO error occurs when we try to open src file for read.
            // First check if their sizes are different.
            if (src_size != dest_size) { // If sizes are different, copy the file.
                FILE *dest_file;
                if ((dest_file = fopen(dest_fpath,"wb")) == NULL) {
                    if (errno == EACCES) {
                        // If we don't have write permission for dest file.
                        perror("fopen");
                        // Change the permission of destination file if possible.
                        if (chmod(dest_fpath, (stat_src.st_mode) & 0777) == -1) {
                            perror("chmod");
                            return (-num_process);
                        }
                        return (-num_process);
                    } else {
                        perror("fopen");
                		return (-num_process);
                    }
            	}
                // If no error occurred, proceed and copy the file.
                copy_file(src_file, dest_file);
                if (chmod(dest_fpath, (stat_src.st_mode) & 0777) == -1) {
                    perror("chmod");
                    return (-num_process);
                }
                if (fclose(dest_file) == EOF) {
                    perror("fopen");
            		exit(-num_process);
                }
            } else { // If sizes are the same, we check hash and permission.
                char *hash_src = hash(f);
                char *hash_dest = hash(src_file);
                if (check_hash(hash_src, hash_dest)) {
                    // If hash is same, then check permission.
                    free(hash_src);
                    free(hash_dest);
                    if (((stat_src.st_mode) & 0777) != ((stat_file.st_mode) & 0777)) {
                        if (chmod(dest_fpath, (stat_src.st_mode) & 0777) == -1) {
                            perror("chmod");
                            return (-num_process);
                        }
                    }
                } else {
                    // If hash is different, then copy the file.
                    FILE *dest_file;
                    if ((dest_file = fopen(dest_fpath,"wb")) == NULL) {
                        if (errno == EACCES) {
                            // If we don't have write permission for dest file.
                            perror("fopen");
                            // Change the permission of destination file if possible.
                            if (chmod(dest_fpath, (stat_src.st_mode) & 0777) == -1) {
                                perror("chmod");
                                return (-num_process);
                            }
                            return (-num_process);
                        } else {
                            perror("fopen");
                    		return (-num_process);
                        }
                	}
                    // If no error occurred, proceed and copy the file.
                    copy_file(src_file, dest_file);
                    if (chmod(dest_fpath, (stat_src.st_mode) & 0777) == -1) {
                        perror("chmod");
                        return (-num_process);
                    }
                    if (fclose(dest_file) == EOF) {
                        perror("fopen");
                		exit(-num_process);
                    }
                }
            }
            if (fclose(src_file) == EOF) {
                perror("fopen");
                exit(-num_process);
            }
            if (fclose(f) == EOF) {
                perror("fclose");
                exit(-num_process);
            }
        }
        free(dest_fpath);
        return num_process;

    } else if (S_ISDIR(stat_src.st_mode)) {
        // If source is a directory.
        // Create a str that is the corresponding path in destination.
        char *dest_fopath = generate_path(dest, basename((char *)src));

        // Declare the pointer here so that I can free it at the end.
        char *src_child;

        // Try to read the directory.
        DIR *dest_fodirp = opendir(dest_fopath);
        if (dest_fodirp == NULL) { // If an error orrurs.
            if (errno == ENOENT) { // If the directory doesn't exist.
                // Make a directory, and properly set its permission.
                if (mkdir(dest_fopath, 0777) == -1) {
                    perror("mkdir");
                    return (-num_process);
                }
                if (chmod(dest_fopath, (stat_src.st_mode) & 0777) == -1) {
                    perror("chmod");
                    return (-num_process);
                }
            } else if (errno == EACCES) { // If we don't have read permission for dest directory.
                perror("opendir");
                // We change its permission if possible.
                if (chmod(dest_fopath, (stat_src.st_mode) & 0777) == -1) {
                    perror("chmod");
                    return (-num_process);
                }
                return (-num_process);
            } else { // If due to other errors.
                perror("opendir");
                return (-num_process);
            }

        } else { // If NO error orrurs.
            // Check the permission.
            struct stat stat_folder;
            lstat(dest_fopath, &stat_folder);
            if (((stat_src.st_mode) & 0777) != ((stat_folder.st_mode) & 0777)) {
                if (chmod(dest_fopath, (stat_src.st_mode) & 0777) == -1) {
                    perror("chmod");
                    return (-num_process);
                }
            }
            if (closedir(dest_fodirp) == -1) {
                perror("closedir");
                exit(-num_process);
            }
        }

        // After copying the first directory, we proceed and check each child
        // of that parent directory.
        DIR *src_dirp = opendir(src);
        if (src_dirp == NULL) {
            perror("opendir");
            return (-num_process);
        }
        struct dirent *dp = readdir(src_dirp);
        while (dp != NULL){
            // Ignore the file named '.' at the beginning.
            if ((*dp).d_name[0] != '.'){
                src_child = generate_path(src, (*dp).d_name);
                struct stat stat_src_child;
                if (lstat(src_child, &stat_src_child) == -1) {
                    perror(src_child);
                    return (-num_process);
                }
                // If the child is a directory.
                if (S_ISDIR(stat_src_child.st_mode)) {
                    // We create a new process and make the recursive call.
                    pid_t pid = fork();
                    if (pid == 0) {
                        exit(copy_ftree(src_child, dest_fopath));
                    } else {
                        num_process++;
                    }
                } else if (S_ISREG(stat_src_child.st_mode)) {
                    // If the child is a file.
                    if (copy_ftree(src_child, dest_fopath) < 0) {
                        neg_flag = 1;
                    }
                } else if (S_ISLNK(stat_src_child.st_mode)) {
                    // Ignore links.
                } else {
                    // shouldn't get here.
                    return (-num_process);
                }
            }
            dp = readdir(src_dirp);
        }
        if (closedir(src_dirp) == -1) {
            perror("closedir");
            exit(-num_process);
        }
        free(dest_fopath);
        free(src_child);
    } else {
        // shouldn't reach here during regular excution.
        return (-num_process);
    }
    // Finally, we have to manipulate the return value.
    int final_re_value = 1;
    // We need to wait for all child processes.
    for (int i = 1; i < num_process; i++){
        int temp;
        char cvalue = 0;
        if(wait(&temp) == -1) { // Waiting...
            perror("wait");
            exit(-num_process);
        } else {
            if(WIFEXITED(temp)) {
                cvalue = WEXITSTATUS(temp);
            }
        }
        // If one of the children exit with negative value, then this indicates
        // that some errors had happened.
        if (cvalue < 0){
            neg_flag = 1;
        }
        // Add up the absolute value of all children's exit value.
        final_re_value += abs(cvalue);
    }
    if (neg_flag) {
        final_re_value = -final_re_value;
    }
    return final_re_value;
}
