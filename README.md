# Hash-Table
# Hash Table Scalability Analysis

This repository contains a concurrent hash table implementation and a comprehensive scalability analysis. The project demonstrates how the hash table performs under varying loads and concurrency levels, using POSIX threads and readers-writer locks to ensure efficient and safe concurrent operations.

## Features

- **Concurrent Hash Table**: Implements a hash table with separate chaining for collision resolution.
- **POSIX Threads**: Utilizes POSIX threads (`pthreads`) for concurrent operations.
- **Readers-Writer Locks**: Ensures safe concurrent access using readers-writer locks.
- **Scalability Testing**: Includes a test suite to measure performance metrics such as throughput and latency under different scenarios.

## Scalability Analysis

The scalability analysis evaluates the performance of the hash table with different numbers of threads and operations. Key metrics such as throughput (operations per second) and latency (time per operation) are measured and analyzed.

## Getting Started

To run the server, client, and scalability tests, follow these steps:

1. **Clone the repository**:
   ```sh
   git clone https://github.com/yourusername/hashtable-scalability-analysis.git
   cd hashtable-scalability-analysis
2. **Compile the code**:
   ```sh
   make
3. **Run the server**
   ```sh
   ./server 10
4. **Run the Client**
   ```sh
   ./client
5. **Run the scalability test**
     ```sh
   ./scalability_test <number_of_threads> <number_of_operations_per_thread>

    
