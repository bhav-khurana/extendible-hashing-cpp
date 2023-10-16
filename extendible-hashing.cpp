#include <bits/stdc++.h>
using namespace std;

// function to get the last k bits of a number n
string getLastKBits(int n, int k) {

    string bits;
    for (int i = 0; i < k; i++) {
        if (n & (1 << i))
            bits.push_back('1');
        else
            bits.push_back('0');
    }

    reverse(bits.begin(), bits.end());
    return bits;
}

// function to convert a binary string to its corresponding decimal value
int getDecimal(string binary) {

    int n = binary.size();
    reverse(binary.begin(), binary.end());

    int value = 0;
    for (int i = 0; i < n; i++) {
        if (binary[i] == '1') value += (1 << i);
    }
    return value;
}


class Bucket {

    private:
        int capacity;
        int localDepth;

    public:
        vector<int> data;
        int getLocalDepth();
        void increaseLocalDepth();
        void decreaseLocalDepth();
        bool isFull();
        bool isEmpty();

        // initializes a bucket with given capacity and local depth
        Bucket(int cap, int depth) {
            capacity = cap;
            localDepth = depth;
        }
};

class Directory {

    private:
        int globalDepth;
        int bucketCapacity;
        
    public:
        vector<Bucket*> pointers;
        int getGlobalDepth();
        void expand();
        void setPointers(string lastK, Bucket* &newBucket);
        void split(int bucketNo);
        void insert(int value);
        int search(int value);
        void shrink();
        void merge(int bucketNo);
        void deleteValue(int value);
        void show();

        // Initializes a directory with global depth = 0 and bucket of given capacity
        Directory(int bucketCap) {
            bucketCapacity = bucketCap;
            globalDepth = 0;
            pointers.resize(1);
            pointers[0] = new Bucket(bucketCap, 0);
        }
};


// Bucket Functions

// function to get the current local depth of the bucket
int Bucket::getLocalDepth() {
    return localDepth;
}

// function to increment the current local depth of the bucket
void Bucket::increaseLocalDepth() {
    localDepth = localDepth + 1;
}

// function to decrement the current local depth of the bucket
void Bucket::decreaseLocalDepth() {
    localDepth = localDepth - 1;
}

// function to check if the bucket is full
bool Bucket::isFull() {
    return data.size() == capacity;
}

// function to check if the bucket is empty
bool Bucket::isEmpty() {
    return data.size() == 0;
}



// Directory Functions

// function to get the current global depth of the directory
int Directory::getGlobalDepth() {
    return globalDepth;
}

// function to expand the directory to double its entries
void Directory::expand() {

    int currDepth = globalDepth;
    int noOfEntries = (1 << currDepth);

    for (int i = 0; i < noOfEntries; i++)
        pointers.push_back(pointers[i]);
        
    globalDepth = currDepth + 1;
}

// function to set the pointers to newBucket after splitting
void Directory::setPointers(string lastK, Bucket* &newBucket) {
    int k = lastK.size();
    int noOfEntries = (1 << globalDepth);
    for (int i = 0; i < noOfEntries; i++) {
        if (getLastKBits(i, k) == lastK) {
            pointers[i] = newBucket;
        }
    }
}

// function to split a bucket
void Directory::split(int bucketNo) {

    int noOfEntries = (1 << globalDepth);

    Bucket* currBucket = pointers[bucketNo];
    vector<int> values = currBucket->data;
    currBucket->data = {};
    int currLocalDepth = currBucket->getLocalDepth();
    string lastK = getLastKBits(bucketNo, currLocalDepth);
    lastK = "1" + lastK;
    Bucket* newBucket = new Bucket(bucketCapacity, currLocalDepth + 1);

    currBucket->increaseLocalDepth();

    setPointers(lastK, newBucket);

    for (auto val : values) {
        string lastK = getLastKBits(val, globalDepth);
        int bucketNo = getDecimal(lastK);
        (pointers[bucketNo]->data).push_back(val);
    }
}

// function to insert a value in the directory
void Directory::insert(int value) {

    string lastK = getLastKBits(value, globalDepth);
    int bucketNo = getDecimal(lastK);

    Bucket* bucket = pointers[bucketNo];

    if (!bucket->isFull())
        (bucket->data).push_back(value);

    else {
        if (bucket->getLocalDepth() < globalDepth) {
            split(bucketNo);
            insert(value);
        }
        else {
            expand();
            insert(value);
        }
    }
    
}

// function to search the bucket number of a particular value
// returns -1 if the value is not present in the directory
int Directory::search(int value) {
    string lastK = getLastKBits(value, globalDepth);
    int bucketNo = getDecimal(lastK);
    for (auto &val : pointers[bucketNo]->data) {
        if (val == value)
            return bucketNo;
    }
    return -1;
}

// function to shrink the directory to half its size
void Directory::shrink() {

    int noOfEntries = (1 << globalDepth);
    for (int i = noOfEntries-1; i >= noOfEntries/2; i--)
        pointers.pop_back();

    globalDepth = globalDepth - 1;
}

// function to merge to buckets after deletion
void Directory::merge(int bucketNo) {

    vector<int> counterParts;
    int noOfEntries = (1 << globalDepth);
    int currLocalDepth = pointers[bucketNo]->getLocalDepth();
    for (int i = 0; i < noOfEntries; i++) {
        if (pointers[i] == pointers[bucketNo]) {
            int counterPart = (i ^ (1 << (currLocalDepth-1)));
            counterParts.push_back(counterPart);
        }
    }

    bool sameBucket = true;
    for (int i = 0; i < counterParts.size(); i++) {
        if (pointers[counterParts[i]] != pointers[counterParts[0]]) {
            sameBucket = false;
            break;
        }
    }

    // merging the buckets if all the counterparts point to the same bucket
    // and the sum of their sizes is less than the bucket capacity
    if (sameBucket) {
        Bucket* bucket1 = pointers[bucketNo];
        Bucket* bucket2 = pointers[counterParts[0]];

        if (bucket1->data.size() + bucket2->data.size() <= bucketCapacity) {
            vector<int> values1 = bucket1->data;
            vector<int> values2 = bucket2->data;
            bucket1->data.clear(), bucket2->data.clear();
            for (auto counterPart : counterParts)
                pointers[counterPart] = bucket1;
            for (auto val : values1)
                bucket1->data.push_back(val);
            for (auto val : values2)
                bucket1->data.push_back(val);

            pointers[bucketNo]->decreaseLocalDepth();

            bool toShrink = true;
            for (int i = 0; i < noOfEntries; i++) {
                if (pointers[i]->getLocalDepth() == globalDepth) {
                    toShrink = false;
                    break;
                }
            }
            if (toShrink)
                shrink();
            if(globalDepth > 0)
                merge(bucketNo);
        }
    }
}

// function to delete a value from the directory
void Directory::deleteValue(int value) {

    int bucketNo = search(value);
    if (bucketNo == -1) {
        cout << "Value not present in the directory!\n\n";
        return;
    }

    // erasing the value from the bucket
    auto it = find(pointers[bucketNo]->data.begin(), pointers[bucketNo]->data.end(), value);
    pointers[bucketNo]->data.erase(it);

    merge(bucketNo);
}

// function to show the contents of the directory
void Directory::show() {

    cout << "\n\n";
    cout << "Global Depth = " << globalDepth << endl << endl;

    map<Bucket*, vector<string>> mp;
    int noOfEntries = (1 << globalDepth);
    for (int i = 0; i < noOfEntries; i++) {
        string lastK = getLastKBits(i, globalDepth);
        Bucket* bucket = pointers[i];
        mp[bucket].push_back(lastK);
    }
    int maxSpace = 0;
    int maxSpace2 = 0;
    for (auto &val : mp) {
        int n = val.second.size();
        int g = globalDepth;
        int space = g*n + 2*n + 5;
        int space2 = 0;
        if ((val.first)->data.empty()) {
            maxSpace2 = max(maxSpace2, 10);
        }
        else {
            for (auto &no : (val.first)->data) {
                string tmp = to_string(no);
                space2 += tmp.size();
                space2 += 2;
            }
        }
        maxSpace = max(maxSpace, space);
        maxSpace2 = max(maxSpace2, space2);
    }
    for (auto &val : mp) {

        for (int i = 0; i < maxSpace + 12 && i < 60; i++) cout << "--";
        cout << endl;

        int n = val.second.size();
        int g = globalDepth;
        int space1 = g*n + 2*n + 2;
        int space2 = 0;
        if ((val.first)->data.empty()) {
            space2 = 10;
        }
        else {
            for (auto &no : (val.first)->data) {
                string tmp = to_string(no);
                space2 += tmp.size();
                space2 += 2;
            }
        }

        for (int i = 0; i < val.second.size(); i++) {
            cout << val.second[i] << " ,"[i != val.second.size() - 1] << " ";
        }

        int noOfSpaces = maxSpace - space1;
        for (int i = 0; i < noOfSpaces && i < 50; i++) cout << " ";

        cout << "-->    ";
        
        int noOfSpaces2 = maxSpace2 - space2;
        for (int i = 0; i < noOfSpaces2 && i < 50; i++) cout << " ";

        if ((val.first)->isEmpty()) {
            cout << "No values";
        }
        else {
            for (int i = 0; i < (val.first)->data.size(); i++) {
                cout << (val.first)->data[i] << " ,"[i != (val.first)->data.size() - 1] << " ";
            }
        }
        cout << " (Local Depth = " << (val.first)->getLocalDepth() << ")" << endl;
    }
    for (int i = 0; i < maxSpace + 12 && i < 60; i++) cout << "--";
    cout << endl;

    cout << "\n\n";

}

int main() {
    int bucketCapacity;
    cout << "Enter bucket capacity: ";
    cin >> bucketCapacity;

    Directory directory(bucketCapacity);

    while (1) {

        int option;
        cout << "Choose the operation you want to perform:" << endl
            << "1. Show directory" << endl
            << "2. Insert a value" << endl
            << "3. Delete a value" << endl
            << "4. Quit" << endl;
        cin >> option;
        switch (option) {

            case 1:
                directory.show();
                break;

            case 2:
                int value;
                cout << "Enter the value you want to insert: ";
                cin >> value;
                directory.insert(value);
                break;
            
            case 3:
                cout << "Enter the value you want to delete: ";
                cin >> value;
                directory.deleteValue(value);
                break;

            case 4:
                goto end;
                break;
            
            default:
                cout << "Invalid Input\n" << endl;
        }

    }
    end: {}
    return 0;
}