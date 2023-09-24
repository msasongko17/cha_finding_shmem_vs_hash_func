# Build
```
g++ consumer.cpp -o consumer
g++ producer.cpp -o producer
```

# Run
```
sudo ./producer
sudo ./consumer 1
```

Running consumer with the option "1" destroys the shared memory. After running the consumer, benchmark results comparing shmem vs hash function will be printed.

It is assumed that the stored data type is integer. Stored item count can be modified with ```NUM``` variable inside ```protocol.h```