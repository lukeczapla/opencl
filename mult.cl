
__kernel void matrixMultiply(__global int *a, __global int *b, __global int *c, 
				      const int M, const int N, const int K) {

    int colIndex = get_global_id(0);
    int rowIndex = get_global_id(1);
    int index = (N*rowIndex) + colIndex;

    int sum = 0;
    for(int k = 0; k < K; k++) {
        sum += a[K*rowIndex + k] * b[N*k + colIndex];
    }
    c[index] = sum;
}

