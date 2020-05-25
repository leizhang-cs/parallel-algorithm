using namespace std;

void scan(int* In, int* Out, int* B, int* C, int n) {
        if (n==0) return;
        if (n == 1) {
                Out[0] = In[0];
                return;
        }
        cilk_for (int i = 0; i < n/2; i++)
                B[i] = In[2*i] + In[2*i+1];

        scan(B, C, B+n/2, C+n/2, n/2);
        Out[0] = In[0];

        cilk_for (int i = 1; i < n; i++) {
                if (i%2) Out[i] = C[i/2];
                else Out[i] = C[i/2-1] + In[i];
        }
}

void scan(int* In, int* Out, int n){
        if(n==0) return;
        if(n==1){
                Out[0] = In[0];
                return;
        }
        for(int i=1; i<n; i++){
                Out[i] = Out[i-1] + In[i];
        }
}
