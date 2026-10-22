#include <iostream>
#include <vector>
using namespace std;


vector<int> crc(vector<int> data, const vector<int>& generator) {

    int n = data.size();
    int k = generator.size();

    // Perform XOR division
    for (int i = 0; i <= n - k; i++) {

        if (data[i] == 1) {

            for (int j = 0; j < k; j++) {
                data[i + j] = data[i + j] ^ generator[j];
            }
        }
    }

    vector<int> remainder;

    for (int i = n - (k - 1); i < n; i++) {
        remainder.push_back(data[i]);
    }

    return remainder;
}


void printBits(const vector<int>& bits) {

    for (int bit : bits) {
        cout << bit;
    }

    cout << endl;
}


bool isZero(const vector<int>& remainder) {

    for (int bit : remainder) {
        if (bit != 0)
            return false;
    }

    return true;
}


int main() {


    int dataSize;
    cout << "Enter number of data bits: ";
    cin >> dataSize;

    vector<int> data(dataSize);

    cout << "Enter data bits: ";

    for (int i = 0; i < dataSize; i++) {
        cin >> data[i];
    }


    int generatorSize;
    cout << "Enter number of generator bits: ";
    cin >> generatorSize;

    vector<int> generator(generatorSize);

    cout << "Enter generator bits: ";

    for (int i = 0; i < generatorSize; i++) {
        cin >> generator[i];
    }


    cout << "\n========== SENDER SIDE ==========\n";

    cout << "Original Data: ";
    printBits(data);


    cout << "Generator: ";
    printBits(generator);


    // Append k-1 zeros
    vector<int> transmittedData = data;

    for (int i = 0; i < generatorSize - 1; i++) {
        transmittedData.push_back(0);
    }


    cout << "Data after appending zeros: ";
    printBits(transmittedData);


    // Calculate CRC
    vector<int> remainder = crc(transmittedData, generator);


    cout << "CRC Remainder: ";
    printBits(remainder);


    // Create final transmitted codeword
    vector<int> codeword = data;

    for (int bit : remainder) {
        codeword.push_back(bit);
    }


    cout << "Final Transmitted Codeword: ";
    printBits(codeword);


    cout << "\n========== RECEIVER SIDE ==========\n";

    int receivedSize;

    cout << "Enter number of received bits: ";
    cin >> receivedSize;

    vector<int> receivedData(receivedSize);

    cout << "Enter received data bits: ";

    for (int i = 0; i < receivedSize; i++) {
        cin >> receivedData[i];
    }


    cout << "Received Codeword: ";
    printBits(receivedData);

    vector<int> receiverRemainder = crc(receivedData, generator);


    cout << "Receiver Remainder: ";
    printBits(receiverRemainder);


    if (isZero(receiverRemainder)) {

        cout << "\nNo error detected.\n";
        cout << "Data ACCEPTED.\n";

    } else {

        cout << "\nError detected!\n";
        cout << "Data REJECTED.\n";
    }


    return 0;
}