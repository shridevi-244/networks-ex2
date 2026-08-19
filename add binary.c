#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* addBinary(char* a, char* b) {
    int i = strlen(a) - 1;
    int j = strlen(b) - 1;
    int carry = 0;
    
    int max_len = (i > j ? i : j) + 2;
    char* result = (char*)malloc((max_len + 1) * sizeof(char));
    
    result[max_len] = '\0'; 
    int k = max_len - 1;    

    while (i >= 0 || j >= 0 || carry) {
        int sum = carry;
        
        if (i >= 0) {
            sum += a[i] - '0'; 
            i--;
        }
        if (j >= 0) {
            sum += b[j] - '0'; 
            j--;
        }
        
        result[k] = (sum % 2) + '0'; 
        k--;
        carry = sum / 2;
    }
    
    if (k >= 0) {
        memmove(result, result + k + 1, max_len - k);
    }
    
    return result;
}
