# README.md for Parallel File Compression Utility

## Overview
The Parallel File Compression utility is a modern application designed to leverage the capabilities of multi-core processors to enhance the performance of file compression tasks. By employing parallel computing concepts, this utility significantly improves the efficiency of compressing large data files. This project not only demonstrates the practical application of parallel computing but also serves as a benchmark to analyze the performance gains achieved through different parallelization strategies.

## Features
- **Sequential Compression**: Compresses files one at a time, serving as a baseline for performance comparison.
- **Basic Parallel Compression**: Compresses two files in parallel, introducing the concept of program-level parallelism.
- **Advanced Parallel Compression**:
  - **Full Parallelism**: Initiates N parallel tasks simultaneously, one for each of the N files.
  - **Batched Core Parallelism**: Processes files in batches, with each batch containing `NB_CORES` tasks, ensuring all CPU cores are utilized efficiently.
  - **Distributed Core Parallelism**: Distributes the compression tasks evenly across `B_CORES` tasks, allowing multiple file compressions per core.

## Getting Started
1. **Prerequisites**
   - Ensure you have a modern multi-core processor to take full advantage of the utility's capabilities.
   - The utility is designed to work optimally with files of at least 30MB in size to ensure that task runtimes are significant compared to the overhead of creating parallel tasks.

2. **Installation**
   - Clone the repository to your local machine.
   - Follow the setup instructions in the `setup.md` file to configure the environment and dependencies.

3. **Running the Utility**
   - Use the command-line interface to specify the compression task.
   - Choose the parallelization strategy and the number of files to compress.
   - The utility will output the compressed files and a performance report.

## Performance Analysis
- Conduct a thorough performance analysis by comparing the time taken to compress files using sequential execution vs. parallel execution with 2, 3, 4, ..., `NB_CORES`.
- Utilize the provided excel template to input your results and generate graphs that illustrate the performance scale-up achieved with each parallelization strategy.
- Investigate and document the efficiency and scalability of each strategy, providing insights into the optimal use cases for each approach.

## Notes
- `NB_CORES` refers to the number of cores on your CPU.
- It is recommended to work with around 300 files (including duplicates) to have a substantial dataset for analysis.
- Ensure that your environment is correctly set up to support parallel processing to get accurate performance insights.

## Conclusion
The Parallel File Compression utility showcases the significant performance enhancements that can be achieved through thoughtful application of parallel computing concepts. By analyzing the results, users can gain valuable insights into how different parallelization strategies can be effectively employed in real-world scenarios.

## Contributing
Contributions to improve the utility or extend its capabilities are welcome. Please ensure to follow the project's contribution guidelines provided in `CONTRIBUTING.md`.


