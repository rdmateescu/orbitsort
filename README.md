# OrbitSort

A rotating-pair parallel sort.

Pairs orbit the array, they never collide, array stays shared.

- Each phase: `W = floor(n/2)` disjoint pairs `(i, i+1 mod n)`
- Canonicalized: smaller -> lower index, even for wrap pair `(n-1,0) -> (0,n-1)`
- Early exit: 2 zero-swap phases for even n, 3 for odd n (circular proof)
- Every swap reduces inversion count by >=1 -> terminates

### Build
```bash
gcc -O2 -std=c11 -fopenmp orbitsort.c -o orbitsort
g++ -O2 -std=c++17 -fopenmp orbitsort.cpp -o orbitsort_cpp
