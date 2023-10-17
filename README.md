# Build
```
g++ consumer.cpp -O3 -o consumer
g++ producer.cpp -O3 -o producer
```

# Run
Fire up a new terminal and then issue 

```
sudo su
ulimit -s unlimited
```

in order to circumvent stack size limitations.

And then run with sudo privilege:
```
./producer
./consumer 1
```

Running consumer with the option "1" destroys the shared memory. After running the consumer, benchmark results comparing shmem vs hash function will be printed.

It is assumed that the stored data type is integer. Stored item count can be modified with ```NUM``` variable inside ```protocol.h```

# Room For Improvement

- Store only addresses for the starting address of cache lines, since all addresses within a single line will have the same CHA.
- I can store CHAs with a single bytes. I can even do store them with bits since on this system max value a CHA can get is 28, which indicates that I should be fine with just 5 bits. But this is an aggressive optimization.
