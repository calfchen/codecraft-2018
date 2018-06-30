总结一下之前的代码

- firstFit函数
- 测试map结构

// C++ program to find number of bins required using
// First Fit algorithm.
// #include <bits/stdc++.h>
#include <iostream>
using namespace std;

int firstFit(int weight[], int n, int c);
// Driver program
int main()
{
    int weight[] = {2, 5, 4, 7, 1, 3, 8};
    int c = 10;
    int n = sizeof(weight) / sizeof(weight[0]);
    cout << "Number of bins required in First Fit : "<< firstFit(weight, n, c) << endl;
    return 0;
}

// Returns number of bins required using first fit 
// online algorithm
int firstFit(int weight[], int n, int c)
{
    // Initialize result (Count of bins)
    int res = 0;
 
    // Create an array to store remaining space in bins
    // there can be at most n bins
    int bin_rem[n];
 
    // Place items one by one
    for (int i=0; i<n; i++)
    {
        // Find the first bin that can accommodate
        // weight[i]
        int j;
        for (j=0; j<res; j++)
        {
            if (bin_rem[j] >= weight[i])
            {
                bin_rem[j] = bin_rem[j] - weight[i];
                break;
            }
        }
 
        // If no bin could accommodate weight[i]
        if (j==res)
        {
            bin_rem[res] = c - weight[i];
            res++;
        }
    }
    return res;
}



#include <iostream>
#include <map>
using namespace std;

int main()
{
    map<int,int> f;
    f[1]++;
    f[2]++;
    f[2]++;

    for(map<int,int>::iterator it = f.begin(); it != f.end(); it++){
        cout << it->first << " " << it->second << endl;
    }
    cout << f.size() << " " << f.max_size() << endl;
    return 0;
}