# local-recursive-file-sync
A program that is capable of synchronizing data between two folders. 📁↔ 📁  
Multiple processes are used to copy files concurrently and speed up the program.  
```
pid_t pid = fork(); // Handle recursive calls for each subdirectory.
```

## Getting Started

### Prerequisites

* GCC
* Terminal (in Unix) OR PowerShell (in Windows)

### Download source code and compile
The following instructions are presented using Terminal in macOS:
```
# Change to HOME directory
$ cd ~

# Clone this repo and 'cd' into it
$ git clone https://github.com/jellycsc/local-recursive-file-sync.git
$ cd local-recursive-file-sync/

# Let's compile
$ make
gcc -Wall -std=gnu99 -g -c fcopy.c
gcc -Wall -std=gnu99 -g -c ftree.c
gcc -Wall -std=gnu99 -g -c hash_functions.c
gcc -Wall -std=gnu99 -g -o fcopy fcopy.o ftree.o hash_functions.o
```

### Usage
```
Usage: fcopy SRC DEST
```

### Example
```
# Let's start copying all files in src/ to the empty directory dest/
$ ./fcopy src dest
Copy completed successfully
5 processes used

# Compare the results using diff
$ diff -r --exclude=".DS_Store" src dest/src
# Besides the mac auto-generated hidden file, there is no other differences
```
All file contents are successfully transferred to `dest` folder.

## Authors

| Name             | GitHub                                     | Email
| ---------------- | ------------------------------------------ | -------------------------
| Chenjie Ni       | [jellycsc](https://github.com/jellycsc)    | nichenjie2013@gmail.com

## Thoughts and future improvements

* Processes can be replaced with [threads](http://man7.org/linux/man-pages/man7/pthreads.7.html). The later ones are more light-weighted with less overheads. 

## Contributing to this project

1. Fork it ( https://github.com/jellycsc/local-recursive-file-sync/fork )
2. Create your feature branch (`git checkout -b my-new-feature`)
3. Commit your changes (`git commit -m 'Add some feature'`)
4. Push to your feature branch (`git push origin my-new-feature`)
5. Create a new Pull Request

Details are described [here](https://git-scm.com/book/en/v2/GitHub-Contributing-to-a-Project).

## Bug Reporting
Please log bugs under [Issues](https://github.com/jellycsc/local-recursive-file-sync/issues) tab on Github.  
OR you can shoot an email to <nichenjie2013@gmail.com>
