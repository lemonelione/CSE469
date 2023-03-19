#include <iostream>
#include <fstream>
#include <vector>
#include <openssl/md5.h>
#include <openssl/sha.h>
#include <iomanip>
#include <string>
#include <sstream>
#include <cctype>

using namespace std;
struct MbrPartitionEntry {
    uint8_t boot_indicator; // 1 byte
    uint8_t start_chs[3];   // 3 bytes
    uint8_t partition_type; // 1 byte
    uint8_t end_chs[3];     // 3 bytes
    uint32_t start_lba;     // 4 byte
    uint32_t size;          // 4 byte
};

/// @brief memory handler -- avoid memory leak
/// @param arrayList 
/// @param num_lines 
void freeArrayMemory(string** arrayList, int num_lines) {
    for (int i = 0; i < num_lines; i++) {
        delete[] arrayList[i];
    }
    delete[] arrayList;
}

/// @brief convert .cvs to 2d array
/// @param arrayList 
/// @param num_lines 
/// @return 2D array database 
string**  _CVStoArray(string** arrayList, int num_lines) {
    // Declare a vector to store the data from the .csv file
    vector<pair<string, string>> data;

    // Open the .csv file
    ifstream infile("PartitionTypes.csv");

    /**
     * @brief make a pair(hex, Partition type)
     * use while-loop to read each line in .cvs file
     * make a pair with delimiter ','
     */
    string line;
    while (getline(infile, line)) {
        // Split each line into two parts using stringstream and getline()
        stringstream ss(line);
        string token;
        getline(ss, token, ','); // First column (string)
        string first_column = token.length() == 1 ? "0" + token : token; // fill 0 into byte format
        getline(ss, token); // Second column (string)
        string second_column = token;

        data.push_back(make_pair(first_column, second_column));
    }

    // Close the input file stream
    infile.close();

    // Make a 2d arraylist out of the data from the .cvs file
    for (int i = 0; i < num_lines; i++) {
        arrayList[i][0] = data[i].first;
        arrayList[i][1] = data[i].second;
    }
    return arrayList;
}

/// @brief write hash and sha files 
/// @param filename
/// @return .txt 
void write_txtFile(const string& filename) {
    ifstream file(filename, ios::binary);
    if (!file) {
        throw runtime_error("Could not open file");
    }

    // Calculate MD5 hash
    MD5_CTX md5_ctx;
    MD5_Init(&md5_ctx);
    char buf[1024];
    while (file.read(buf, sizeof(buf))) {
        MD5_Update(&md5_ctx, buf, sizeof(buf));
    }
    if (file.gcount() > 0) {
        MD5_Update(&md5_ctx, buf, file.gcount());
    }
    unsigned char md5_hash[MD5_DIGEST_LENGTH];
    MD5_Final(md5_hash, &md5_ctx);

    // Write MD5 hash to file
    ofstream md5_file("MD5-" + filename + ".txt");
    for (int i = 0; i < MD5_DIGEST_LENGTH; i++) {
        md5_file << hex << setw(2) << setfill('0') << static_cast<int>(md5_hash[i]);
    }
    md5_file.close();   // .txt -- md5 hash

    // Calculate SHA-256 hash
    SHA256_CTX sha256_ctx;
    SHA256_Init(&sha256_ctx);
    file.clear();
    file.seekg(0);
    while (file.read(buf, sizeof(buf))) {
        SHA256_Update(&sha256_ctx, buf, sizeof(buf));
    }
    if (file.gcount() > 0) {
        SHA256_Update(&sha256_ctx, buf, file.gcount());
    }
    unsigned char sha256_hash[SHA256_DIGEST_LENGTH];
    SHA256_Final(sha256_hash, &sha256_ctx);

    // Write SHA-256 hash to file
    ofstream sha256_file("SHA-256-" + filename + ".txt");
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        sha256_file << hex << setw(2) << setfill('0') << static_cast<int>(sha256_hash[i]);
    }
    sha256_file.close();    // .txt -- sha256

    file.close();           // .raw 
}

/// @brief read and skip 496 bytes
/// @param image_file 
/// @param partition_table 
void readMBR(ifstream& image_file, vector<MbrPartitionEntry>& partition_table) {
    image_file.seekg(0x1BE);
    partition_table.resize(4);
    for (int i = 0; i < 4; i++) {
        MbrPartitionEntry entry;
        image_file.read(reinterpret_cast<char*>(&entry), 16);
        partition_table[i] = entry;
    }
}

/// @brief read GPT file in binary
/// @param image_file 
void readGPT(string image_file) {
    // Open the binary file
    ifstream binaryFile(image_file, ios::binary);
    if (!binaryFile.is_open()) {
        cerr << "Error: Could not open file " << image_file << endl;
        return;
    }

    // Read the first 400 hex offset in bytes
    uint8_t buffer[0x400];
    binaryFile.read(reinterpret_cast<char*>(buffer), 0x400);

    // GPT goes through the number of partitions
    for (int i = 1; i <= 4; ++i) {
        // Read the Partition Type GUID
        uint8_t guidPartType[16];
        binaryFile.read(reinterpret_cast<char*>(guidPartType), 16);

        // Read and print the Partition information
        cout << "Partition number: " << i << endl;
        cout << endl;
        cout << "Partition Type GUID: ";
        for (int j = 0; j < 16; ++j) {
            cout << setw(2) << setfill('0') << hex << uppercase << static_cast<int>(guidPartType[j]);
        }
        cout << endl;

        binaryFile.read(reinterpret_cast<char*>(buffer), 16);

        uint64_t startLBA, endLBA;
        binaryFile.read(reinterpret_cast<char*>(&startLBA), 8);
        cout << "Starting LBA address in hex: " << "0x" << hex << startLBA << endl;
        binaryFile.read(reinterpret_cast<char*>(&endLBA), 8);
        cout << "Ending LBA address in hex: " << "0x" << hex << nouppercase << endLBA << endl;
        cout << "Starting LBA address in decimal: " << dec << startLBA << endl;
        cout << "Ending LBA address in decimal: " << dec << endLBA << endl;

        binaryFile.read(reinterpret_cast<char*>(buffer), 80);
    }

    binaryFile.close();
}

/// @brief print the partition types and size
/// @brief print the 16 bytes of boot record
/// @param partition_table 
void printMBR(const vector<MbrPartitionEntry>& partition_table, string filename) {
    // local vars
    string id, type; 
    int num_lines;
    
    // Allocate a 2d array to store the data
    num_lines = 98;
    uint8_t sector[512];
    string** ptArrayList = new string*[num_lines];
    for (int i = 0; i < num_lines; i++) {
        ptArrayList[i] = new string[2];
    }
    ptArrayList = _CVStoArray(ptArrayList, num_lines);

    // Loop through each entry in the MBR partition table
    cout << endl; // formatting purpose
    for (int i = 0; i < 4; i++) {
        const MbrPartitionEntry& entry = partition_table[i];

        // Loop through each row in the 2d array database
        for (int j = 0; j < num_lines; j++) {
            // Compare the partition type of the MBR partition table entry
            // with the partition type in the database
            if (entry.partition_type == stoul(ptArrayList[j][0], nullptr, 16)) {
                // if match, return the pair value from the database
                id = ptArrayList[j][0];
                type = ptArrayList[j][1];
                // remove '\n' from the each pair
                if (!type.empty() && isspace(type.back())) { 
                    type.pop_back();
                }
                cout << "(" << id << ") " << type << " , " << dec << entry.start_lba << ", " << entry.size*512 << endl;
                break;
            }
        }
    }

    for (int i = 0; i < 4; i++) {
        const MbrPartitionEntry& entry = partition_table[i];
        // Output the hexadecimal values of the final 16 bytes of the boot record
        cout << "Partition number: " << i +1 << endl;
        cout << "Last 16 bytes of boot record: ";
        // reopen the file in bianry to read it again for each partition
        ifstream new_file(filename, ios::binary);
        new_file.seekg(entry.start_lba + 512 - 16, ios::beg); // skip the correct position
        uint8_t last16[16];
        new_file.read(reinterpret_cast<char*>(last16), sizeof(last16));
        for (int j = 0; j < 16; j++) {
            cout << setfill('0') << setw(2) << hex << static_cast<int>(last16[j]) << " ";
        }
        cout << endl;
    }
}

/// @brief main driver
/// @param argc -- arguments count 
/// @param argv -- arguments vector<string>
/// @return 
int main(int argc, char* argv[]) {
    // throw error message if wrong number of arguments
    // we can make more error handlers, but not neccessary
    if (argc != 5) {
        cerr << "Usage: boot_info -t <type> -f <filename>" << endl;
        return 1;
    }

    // process files and command lines
    string type = argv[2];
    string filename = argv[4];
    ifstream image_file(filename, ios::binary);
    if (!image_file) {
        cerr << "Error opening file " << filename << endl;
        return 1;
    }

    // process the partition tables
    if (type == "mbr") {
        // analyze mbr as binary file and print contents
        vector<MbrPartitionEntry> partition_table;
        readMBR(image_file, partition_table);
        printMBR(partition_table, filename);

        //calculate file size into hash and sha
        write_txtFile(filename);
    } else if (type == "gpt") {
        // analyze mbr as binary file and print contents
        readGPT(filename);

        // calculate file size into hash and sha
        write_txtFile(filename);
    } else {
        cerr << "Invalid partition table type: " << type << endl;
        return 1;
    }
    image_file.close();
    return 0;
}