#include <bits/stdc++.h>
using namespace std;

// Performs bitwise XOR between two binary strings (a and b)
string findXor(string a, string b) {
    int n = b.length();
    string result = "";
    
    // Compare each bit (skip first bit as per CRC standard)
    for (int i = 1; i < n; i++) {
        if (a[i] == b[i])
            result += "0";
        else
            result += "1";
    }
    return result;
}

// Performs Modulo-2 division (CRC division algorithm)
string mod2div(string dividend, string divisor) {
    int n = dividend.length();
    int pick = divisor.length();
    string tmp = dividend.substr(0, pick); // Initial window

    while (pick < n) {
        if (tmp[0] == '1')
            // XOR with divisor and bring down next bit
            tmp = findXor(divisor, tmp) + dividend[pick];
        else
            // XOR with zeros and bring down next bit
            tmp = findXor(string(pick, '0'), tmp) + dividend[pick];
        pick++;
    }

    // Final XOR step
    if (tmp[0] == '1')
        tmp = findXor(divisor, tmp);
    else
        tmp = findXor(string(pick, '0'), tmp);

    return tmp;
}

// Appends CRC remainder to the original data
string encodeData(string data, string key) {
    int n = key.length();
    string paddedData = data + string(n - 1, '0'); // Append n-1 zeros
    string remainder = mod2div(paddedData, key);
    return data + remainder; // Return data + CRC
}

// Checks if received data has errors (remainder = 0)
int receiver(string code, string key) {
    string remainder = mod2div(code, key);
    return (remainder.find('1') == string::npos) ? 1 : 0;
}

int main() {
    string data = "100100";
    string key = "1101";
    
    cout << "Sender Side" << endl;
    cout << "Data: " << data << endl;
    cout << "Key: " << key << endl;
    string code = encodeData(data, key);
    cout << "Encoded Data: " << code << endl << endl;

    cout << "Receiver Side" << endl;
    if (receiver(code, key))
        cout << "Data is correct (No errors detected)" << endl;
    else
        cout << "Data is incorrect (Error detected)" << endl;

    return 0;
}