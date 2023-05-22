# top_k_parallel
 Parallel TopK algorithm using multiple threads
# File access and data parsing 
The efficiency of the implementation of a parallel TopK algorithm depends heavily on how to 
store and order a vast amount of data. First off, we should have read the data from the files. My 
team and I have decided to read the files not using either parallelism or concurrency, as it seems 
that the hard drive speed is a bottleneck, and we actually lose some performance due to the 
overhead: 
Average of the ten tests on the case 5 where the K is random: 
 With parallelism: 6.86s (the clock() function is used) 
 Without parallelism: 2.68s (the clock() function is used) 
Reading a file was performed as follows: the program gets a line from a file, observes the first 
ten characters, then converts them into a long integer using the strtol() function and increases the 
value in a correlating cell in a counter array. The counter array length was set to 9321 + 1, as 
there are 9321 hours between the minimum and maximum entry dates: ceil((1679046032 – 
1645491600)/3600) = 9321. 
# Getting top K values 
We have decided to use an array representation of heap trees to order the values. The 
heapTraverse() function checks if any of a node’s children are greater than the node and, if so, 
swaps the node with the larger child, then calls itself recursively. That approach assures us that 
the node is the largest in the subtree after performing the heapTraverse() function on it. The 
buildHeap() method is rather straightforward: it accepts the start and end points of the counter 
array (this portion of the array is to be heapified) and performs the heapTraverse() function to all 
the internal nodes, building a heap. The time complexities of the functions are O(logN) for the 
heapTraverse() and O(N) for the buildHeap(). 
As we lose the initial position – and, eventually, the log date – of a log number after heapifying, 
we have created an Info structure with the information about the time and number of the logs. An 
Info is considered to be greater than another one if its number of logs is greater (if the log 
number is the same for both Info structures, the entry date is compared). 
After the threads create the heaps in the counter array using the buildHeap() function, we have 
simply decided to iterate through the roots of the heaps (first elements in the heapified array 
fragments), getting the maximum value. Then we print the number of logs and date using ctime() 
function, remove the root node from the heap by setting the log number of it to -1 and reheapify 
the corresponding heap tree. 
# Evaluation and time complexity analysis 
The time complexity of the approach described above is O(N + F + KlogN). 
O(F) – reading the input files, O(N) – building a heap, O(KlogN) – getting the top K values, 
where F – number of input logs, N – number of distinct hours. 
