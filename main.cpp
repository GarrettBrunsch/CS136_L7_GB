// Garrett, Brunsch
// Lab #7 Hashing S25
// CS_136_14728SP25P
// Due 4/6/25 with extension due 4/13

#include <iostream>
#include <string>
#include <limits>
#include <stdexcept>
#include <fstream>
#include <sstream>
#include <iomanip>
using namespace std;

const string INPUT_FILENAME = "in_students.txt";
const string UNPROCESSED_FILENAME = "out_unprocessed.txt";

enum MenuOption
{
    CREATE_TABLES = 1,
    SEARCH_RECORD,
    ARCHIVE_RECORD,
    PRINT_ACTIVE,
    PRINT_ARCHIVED,
    PRINT_UNPROCESSED,
    EXIT // 7 
};

class StudentRecord
{
public:
    int studentID = 0; // 9 digit ID with first two digits guaranteed to be 88 or 99
    int unitsTaken = 0;
    string firstName = "n/a";
    string lastName = "n/a";
    bool isArchived = false;

    string toString() const;

    StudentRecord();
    StudentRecord(int id, int units, string first, string last);
};

class HashTable
{
private:
    const int ID_MASK = 10000000;
    const int FIRST_MASK = 10000;
    const int SECOND_MASK = 10000;

    const int SMALL_TABLE_SIZE = 10;
    const int MEDIUM_TABLE_SIZE = 100;
    const int LARGE_TABLE_SIZE = 1000;

    int hashTableSize = 0;
    int overflowTableSize = 0;
    int digitsToExtract = 0;

    bool* hashTableUsed = nullptr;
    bool* overflowTableUsed = nullptr;

    StudentRecord* hashTable;
    StudentRecord* overflowTable;

    int reverseNumber(int input);
    int boundaryFolding(int originalID);
    int countDigits(int number);
    int numberToDigitArray(int number, char digits[]);
    int extractMiddleDigits(int number, int digitsToExtract);

    void swapPointers(StudentRecord*& a, StudentRecord*& b);
    void swapBoolPointers(bool*& a, bool*& b);

    void clearData();

public:
    int obtainValidHashSize();
    int calcDigitsToExtract(int hashTableSize);
    int calcHashIndex(int originalNumber, int digitsToExtract, int hashTableSize);
    int getNumSquared(int number) const;

    void createTables(int hashSize);
    bool insertRecord(const StudentRecord& record);
    StudentRecord* findRecord(int studentID, bool& inHashTable, int& position);
    bool archiveRecord(int studentID);

    void printHeader(const string& title) const;
    void printActiveRecords() const;
    void printArchivedRecords() const;

    HashTable();
    HashTable(const HashTable& other);
    HashTable& operator=(const HashTable& other);
    ~HashTable();
};

class StudentDatabase
{
private:
    HashTable hashTable;
    bool tablesCreated = false;

public:
    int calcUnprocessedArraySize();

    void createTables();
    void ensureTablesCreated();

    void processRecords();
    void handleUnprocessedRecords(StudentRecord* records, int count);
    void searchRecord();
    void archiveRecord();

    void printActiveRecords();
    void printArchivedRecords();
    void printUnprocessedRecords();

    StudentDatabase();
};

void clearInputError(string& error);
void swap(int& a, int& b);
int displayMenu();

int main()
{
    int exitCode = 0;

    try
    {
        StudentDatabase database;
        int choice;
        do
        {
            choice = displayMenu();
            switch (choice)
            {
            case CREATE_TABLES:
                database.createTables();
                break;
            case SEARCH_RECORD:
                database.searchRecord();
                break;
            case ARCHIVE_RECORD:
                database.archiveRecord();
                break;
            case PRINT_ACTIVE:
                database.printActiveRecords();
                break;
            case PRINT_ARCHIVED:
                database.printArchivedRecords();
                break;
            case PRINT_UNPROCESSED:
                database.printUnprocessedRecords();
                break;
            case EXIT:
                cout << "Exiting program...\n";
                break;
            default:
                string error = "Invalid option. Please enter an option between 1 and 7 \n";
                clearInputError(error);
            }
        } while (choice != EXIT);
    }
    catch (const exception& e)
    {
        cout << "Error: " << e.what() << "\n";
        exitCode = 1;
    }
    return exitCode;
}

void clearInputError(string& error)
{
    cin.clear();
    cin.ignore(numeric_limits<streamsize>::max(), '\n');
    cout << error;
}

void swap(int& a, int& b)
{
    int temp = a;
    a = b;
    b = temp;
}

int displayMenu()
{
    int choice = -1;

    cout << "\n\n===== MAIN MENU =====\n"
        "1. Create hash tables and load records\n"
        "2. Search for a record by ID\n"
        "3. Archive a record\n"
        "4. Print all active records\n"
        "5. Print all archived records\n"
        "6. Print unprocessed records\n"
        "7. Quit\n"
        "Choice: ";

    cin >> choice;
    if (cin.fail())
    {
        choice = -1;
    }
    return choice;
}

StudentRecord::StudentRecord()
{
    int studentID = 0;
    int unitsTaken = 0;
    string firstName = "n/a";
    string lastName = "n/a";
    bool isArchived = false;
}

StudentRecord::StudentRecord(int id, int units, string first, string last)
{
    studentID = id;
    unitsTaken = units;
    firstName = first;
    lastName = last;
    isArchived = false;
}

string StudentRecord::toString() const
{
    stringstream ss;
    ss << left << setw(12) << studentID
        << left << setw(15) << firstName
        << left << setw(15) << lastName
        << right << setw(8) << unitsTaken
        << "   " << (isArchived ? "Archived" : "Active");
    return ss.str();
}

HashTable::HashTable()
{
    hashTable = nullptr;
    overflowTable = nullptr;
    hashTableUsed = nullptr;
    overflowTableUsed = nullptr;
    hashTableSize = 0;
    overflowTableSize = 0;
    digitsToExtract = 0;
}

HashTable::~HashTable()
{
    clearData();
}

void HashTable::clearData()
{
    delete[] hashTable;
    delete[] overflowTable;
    delete[] hashTableUsed;
    delete[] overflowTableUsed;

    hashTable = nullptr;
    overflowTable = nullptr;
    hashTableUsed = nullptr;
    overflowTableUsed = nullptr;

    hashTableSize = 0;
    overflowTableSize = 0;
    digitsToExtract = 0;
}

HashTable::HashTable(const HashTable& other)
{
    hashTable = nullptr;
    overflowTable = nullptr;
    hashTableUsed = nullptr;
    overflowTableUsed = nullptr;

    try
    {
        if (other.hashTableSize > 0)
        {
            hashTable = new StudentRecord[other.hashTableSize];
            hashTableUsed = new bool[other.hashTableSize];

            for (int i = 0; i < other.hashTableSize; i++)
            {
                hashTable[i] = other.hashTable[i];
                hashTableUsed[i] = other.hashTableUsed[i];
            }
            hashTableSize = other.hashTableSize;
        }

        if (other.overflowTableSize > 0)
        {
            overflowTable = new StudentRecord[other.overflowTableSize];
            overflowTableUsed = new bool[other.overflowTableSize];

            for (int i = 0; i < other.overflowTableSize; i++)
            {
                overflowTable[i] = other.overflowTable[i];
                overflowTableUsed[i] = other.overflowTableUsed[i];
            }
            overflowTableSize = other.overflowTableSize;
        }

        digitsToExtract = other.digitsToExtract;
    }
    catch (const bad_alloc& e)
    {
        clearData();
        throw runtime_error("Memory allocation failed during copy: " + string(e.what()));
    }
    catch (...)
    {
        clearData();
        throw;
    }
}

HashTable& HashTable::operator=(const HashTable& other)
{
    if (this != &other)
    {
        HashTable temp(other);

        swap(hashTableSize, temp.hashTableSize);
        swap(overflowTableSize, temp.overflowTableSize);
        swap(digitsToExtract, temp.digitsToExtract);
        swapPointers(hashTable, temp.hashTable);
        swapPointers(overflowTable, temp.overflowTable);
        swapBoolPointers(hashTableUsed, temp.hashTableUsed);
        swapBoolPointers(overflowTableUsed, temp.overflowTableUsed);
    }
    return *this;
}

void HashTable::swapPointers(StudentRecord*& a, StudentRecord*& b)
{
    StudentRecord* temp = a;
    a = b;
    b = temp;
}

void HashTable::swapBoolPointers(bool*& a, bool*& b)
{
    bool* temp = a;
    a = b;
    b = temp;
}

int HashTable::reverseNumber(int input)
{
    int reversedNum = 0;

    while (input > 0)
    {
        reversedNum = reversedNum * 10 + input % 10;
        input = input / 10;
    }
    return reversedNum;
}

int HashTable::calcDigitsToExtract(int hashTableSize)
{
    int digitsToExtract = (hashTableSize == SMALL_TABLE_SIZE) ? 1 :
        (hashTableSize == MEDIUM_TABLE_SIZE) ? 2 : 3;

    return digitsToExtract;
}

int HashTable::calcHashIndex(int originalID, int digitsToExtract, int hashTableSize)
{
    int sumOfReversed = boundaryFolding(originalID);
    int sumSquared = getNumSquared(sumOfReversed);
    int middleDigits = extractMiddleDigits(sumSquared, digitsToExtract);

    int hashIndex = middleDigits % hashTableSize;

    return hashIndex;
}

int HashTable::boundaryFolding(int originalID)
{
    int extractedID = (originalID % ID_MASK);
    int firstGroup = extractedID / FIRST_MASK;
    int secondGroup = extractedID % SECOND_MASK;

    int reversedFirst = reverseNumber(firstGroup);
    int reversedSecond = reverseNumber(secondGroup);
    int sumOfReversed = reversedFirst + reversedSecond;

    return sumOfReversed;
}

int HashTable::countDigits(int number)
{
    int digitCount = 0;
    if (number == 0)
    {
        digitCount = 1;
    }
    else
    {
        while (number > 0)
        {
            number = number / 10;
            digitCount++;
        }
    }
    return digitCount;
}

int HashTable::numberToDigitArray(int number, char digits[])
{
    int digitCount = countDigits(number);

    for (int i = digitCount - 1; i >= 0; i--)
    {
        digits[i] = (number % 10) + '0';
        number /= 10;
    }

    return digitCount;
}

int HashTable::extractMiddleDigits(int number, int digitsToExtract)
{
    int result = 0;
    char digits[20]; // 10 should be fine for 9 digits + null terminating, but 20 used for safety

    int length = numberToDigitArray(number, digits);
    int start = (length - digitsToExtract) / 2;

    for (int i = 0; i < digitsToExtract; i++)
    {
        if (start + i < length)
        {
            result = result * 10 + (digits[start + i] - '0');
        }
    }

    return result;
}

int HashTable::obtainValidHashSize()
{
    int size;
    bool validInput = false;

    while (!validInput)
    {
        cout << "Enter hash table size (" << SMALL_TABLE_SIZE << ", "
            << MEDIUM_TABLE_SIZE << ", or " << LARGE_TABLE_SIZE << "): ";
        cin >> size;

        if (cin.fail() || (size != SMALL_TABLE_SIZE && size != MEDIUM_TABLE_SIZE && size != LARGE_TABLE_SIZE))
        {
            string errorMessage = "Invalid input. Please enter " + to_string(SMALL_TABLE_SIZE) + ", "
                + to_string(MEDIUM_TABLE_SIZE) + ", or " + to_string(LARGE_TABLE_SIZE) + " for the size\n";

            clearInputError(errorMessage);
        }
        else
        {
            validInput = true;
        }
    }
    return size;
}

int HashTable::getNumSquared(int number) const
{
    return (number * number);
}

void HashTable::createTables(int hashSize)
{
    clearData();

    hashTableSize = hashSize;
    overflowTableSize = hashTableSize * 0.2; // Project specifies 20% of 10,100,100 so int works

    digitsToExtract = calcDigitsToExtract(hashTableSize);

    try
    {
        hashTable = new StudentRecord[hashTableSize];
        overflowTable = new StudentRecord[overflowTableSize];
        hashTableUsed = new bool[hashTableSize]();
        overflowTableUsed = new bool[overflowTableSize]();

        cout << "Created hash table with " << hashTableSize << " slots and overflow table with "
            << overflowTableSize << " slots\n";
    }
    catch (const bad_alloc& e)
    {
        clearData();
        throw runtime_error("Memory allocation failed: " + string(e.what()));
    }
}

bool HashTable::insertRecord(const StudentRecord& record)
{
    bool inserted = false;

    if (hashTableSize > 0)
    {
        int hashIndex = calcHashIndex(record.studentID, digitsToExtract, hashTableSize);

        if (!hashTableUsed[hashIndex])
        {
            hashTable[hashIndex] = record;
            hashTableUsed[hashIndex] = true;
            inserted = true;
        }
        else
        {
            // If collision try to insert into overflow table w/ linear probing
            int i = 0;

            while (!inserted && i < overflowTableSize)
            {
                if (!overflowTableUsed[i])
                {
                    overflowTable[i] = record;
                    overflowTableUsed[i] = true;
                    inserted = true;
                }
                i++;
            }
        }
    }

    return inserted;
}

StudentRecord* HashTable::findRecord(int studentID, bool& inHashTable, int& position)
{
    StudentRecord* foundRecord = nullptr;

    if (hashTableSize > 0)
    {
        int hashIndex = calcHashIndex(studentID, digitsToExtract, hashTableSize);

        if (hashTableUsed[hashIndex] && hashTable[hashIndex].studentID == studentID)
        {
            inHashTable = true;
            position = hashIndex;
            foundRecord = &hashTable[hashIndex];
        }
        else
        {
            int i = 0;
            while (foundRecord == nullptr && i < overflowTableSize)
            {
                if (overflowTableUsed[i] && overflowTable[i].studentID == studentID)
                {
                    inHashTable = false;
                    position = i;
                    foundRecord = &overflowTable[i];
                }
                i++;
            }
        }
    }

    return foundRecord;
}

bool HashTable::archiveRecord(int studentID)
{
    bool archived = false;
    bool inHashTable;
    int position;

    StudentRecord* record = findRecord(studentID, inHashTable, position);

    if (record != nullptr)
    {
        record->isArchived = true;
        archived = true;
    }
    return archived;
}

void HashTable::printHeader(const string& title) const
{
    cout << "\n===== " << title << " =====\n"
        << left << setw(12) << "Student ID"
        << left << setw(15) << "First Name"
        << left << setw(15) << "Last Name"
        << right << setw(8) << "Units"
        << "   Status\n";
    cout << string(60, '-') << "\n";
}

void HashTable::printActiveRecords() const
{
    printHeader("ACTIVE RECORDS");

    bool found = false;

    for (int i = 0; i < hashTableSize; i++)
    {
        if (hashTableUsed[i] && !hashTable[i].isArchived)
        {
            cout << hashTable[i].toString() << " (Hash Table [" << i << "])\n";
            found = true;
        }
    }

    for (int i = 0; i < overflowTableSize; i++)
    {
        if (overflowTableUsed[i] && !overflowTable[i].isArchived)
        {
            cout << overflowTable[i].toString() << " (Overflow Table [" << i << "])\n";
            found = true;
        }
    }

    if (!found)
    {
        cout << "No active records found\n";
    }
}

void HashTable::printArchivedRecords() const
{
    printHeader("ARCHIVED RECORDS");

    bool found = false;

    for (int i = 0; i < hashTableSize; i++)
    {
        if (hashTableUsed[i] && hashTable[i].isArchived)
        {
            cout << hashTable[i].toString() << " (Hash Table [" << i << "])\n";
            found = true;
        }
    }

    for (int i = 0; i < overflowTableSize; i++)
    {
        if (overflowTableUsed[i] && overflowTable[i].isArchived)
        {
            cout << overflowTable[i].toString() << " (Overflow Table [" << i << "])\n";
            found = true;
        }
    }

    if (!found)
    {
        cout << "No archived records found.\n";
    }
}

StudentDatabase::StudentDatabase()
{
    tablesCreated = false;
}

void StudentDatabase::createTables()
{
    bool shouldCreate = true;
    if (tablesCreated)
    {
        cout << "Tables already created. Do you want to recreate them? (Y/N): ";
        char confirm;
        cin >> confirm;
        shouldCreate = (confirm == 'Y' || confirm == 'y');
    }

    if (shouldCreate)
    {
        int hashSize = hashTable.obtainValidHashSize();
        hashTable.createTables(hashSize);
        processRecords();
        tablesCreated = true;
    }
}

int StudentDatabase::calcUnprocessedArraySize()
{
    int result = 1;
    ifstream countFile(INPUT_FILENAME);

    if (countFile.is_open())
    {
        int totalRecords = 0;
        int id = 0;
        int units = 0;
        string firstName = "n/a";
        string lastName = "n/a";

        while (countFile >> id >> firstName >> lastName >> units)
        {
            totalRecords++;
        }
        countFile.close();

        if (totalRecords > 1)
        {
            result = totalRecords;
        }
    }

    return result;
}

void StudentDatabase::processRecords()
{
    cout << "Loading records from " << INPUT_FILENAME << "...\n";
    ifstream inputFile(INPUT_FILENAME);
    bool fileExists = inputFile.is_open();

    int recordsProcessed = 0;
    int unprocessedCount = 0;
    int unprocessedArraySize = calcUnprocessedArraySize();
    StudentRecord* unprocessedRecords = nullptr;

    if (fileExists)
    {
        int id, units;
        string firstName, lastName;

        try
        {
            unprocessedRecords = new StudentRecord[unprocessedArraySize];

            while (inputFile >> id >> firstName >> lastName >> units)
            {
                StudentRecord record(id, units, firstName, lastName);
                if (hashTable.insertRecord(record))
                {
                    recordsProcessed++;
                }
                else
                {
                    if (unprocessedCount < unprocessedArraySize) //TODO? add a safety check with upper bounds for edge case input? Not required for project so not added
                    {
                        unprocessedRecords[unprocessedCount] = record;
                        unprocessedCount++;
                    }
                }
            }
            inputFile.close();

            if (recordsProcessed == 0)
            {
                cout << "The input file was empty\n";
            }

            if (unprocessedCount > 0)
            {
                cout << "Both hash and overflow tables are full. " << unprocessedCount << " records could not be processed\n";
                handleUnprocessedRecords(unprocessedRecords, unprocessedCount);
            }

            cout << "Processed " << recordsProcessed << " records from the file.\n";
            delete[] unprocessedRecords;
        }
        catch (const exception& e)
        {
            inputFile.close();
            delete[] unprocessedRecords;
            cout << "Error while processing file: " << e.what() << "\n";
        }
    }
    else
    {
        cout << "Error: Could not open file " << INPUT_FILENAME << "\n";
    }
}

void StudentDatabase::handleUnprocessedRecords(StudentRecord* records, int count)
{
    ofstream outputFile(UNPROCESSED_FILENAME);
    bool fileOpened = outputFile.is_open();

    if (fileOpened)
    {
        for (int i = 0; i < count; i++)
        {
            outputFile << records[i].studentID << " "
                << records[i].firstName << " "
                << records[i].lastName << " "
                << records[i].unitsTaken << "\n";
        }
        outputFile.close();
        cout << "Wrote " << count << " unprocessed records to " << UNPROCESSED_FILENAME << "\n";
    }
    else
    {
        cout << "Error: Could not create unprocessed records file\n";
    }
}

void StudentDatabase::ensureTablesCreated()
{
    if (!tablesCreated)
    {
        cout << "Tables must first be created. Now creating tables...\n";
        createTables();
    }
}

void StudentDatabase::searchRecord()
{
    ensureTablesCreated();
    int searchID;
    cout << "Enter student ID to search for: ";
    cin >> searchID;

    bool inHashTable;
    int position;
    StudentRecord* record = hashTable.findRecord(searchID, inHashTable, position);

    if (record)
    {
        cout << "Record found in " << (inHashTable ? "hash" : "overflow") << " table at position " << position << ":\n";
        hashTable.printHeader("SEARCH RESULT");
        cout << record->toString() << "\n";
    }
    else
    {
        cout << "Record with ID " << searchID << " not found\n";
    }
}

void StudentDatabase::archiveRecord()
{
    ensureTablesCreated();

    int archiveID;
    cout << "Enter student ID to archive: ";
    cin >> archiveID;

    if (hashTable.archiveRecord(archiveID))
    {
        cout << "Record with ID " << archiveID << " has been archived\n";
    }
    else
    {
        cout << "Record with ID " << archiveID << " not found\n";
    }
}

void StudentDatabase::printActiveRecords()
{
    ensureTablesCreated();
    hashTable.printActiveRecords();
}

void StudentDatabase::printArchivedRecords()
{
    ensureTablesCreated();
    hashTable.printArchivedRecords();
}

void StudentDatabase::printUnprocessedRecords()
{
    ifstream unprocessedFile(UNPROCESSED_FILENAME);
    bool fileExists = unprocessedFile.is_open();

    if (fileExists)
    {
        cout << "\n===== UNPROCESSED RECORDS =====\n";
        string line;
        while (getline(unprocessedFile, line))
        {
            cout << line << "\n";
        }
        unprocessedFile.close();
    }
    else
    {
        cout << "No unprocessed records file found\n";
    }
}

/* 

TEST CASE: Missing Input File

  ===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 1
Enter hash table size (10, 100, or 1000): 10
Created hash table with 10 slots and overflow table with 2 slots
Loading records from missing_input_example.txt...
Error: Could not open file missing_input_example.txt

TEST CASE: Small overflow due to collision

===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 1
Enter hash table size (10, 100, or 1000): 100
Created hash table with 100 slots and overflow table with 20 slots
Loading records from in_students.txt...
Both hash and overflow tables are full. 1 records could not be processed
Wrote 1 unprocessed records to out_unprocessed.txt
Processed 39 records from the file.


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 4

===== ACTIVE RECORDS =====
Student ID  First Name     Last Name         Units   Status
------------------------------------------------------------
881234567   John           Smith                30   Active (Hash Table [0])
889012345   Andrew         Martin               42   Active (Hash Table [2])
886789012   Christopher    Taylor               48   Active (Hash Table [10])
887788990   Brandon        Scott                21   Active (Hash Table [15])
880012345   Tyler          Phillips             27   Active (Hash Table [17])
883344556   Steven         Lewis                39   Active (Hash Table [18])
888901234   Matthew        White                36   Active (Hash Table [27])
882233445   Brian          Robinson             18   Active (Hash Table [35])
881122334   Kevin          Garcia               21   Active (Hash Table [38])
885566778   Joshua         Allen                42   Active (Hash Table [43])
886677889   Justin         King                 30   Active (Hash Table [48])
883456789   Michael        Brown                18   Active (Hash Table [55])
885678901   James          Miller               21   Active (Hash Table [70])
880234567   Samuel         Perez                18   Active (Hash Table [74])
880345678   Nathan         Cook                 30   Active (Hash Table [80])
882345678   David          Johnson              36   Active (Hash Table [84])
880123456   Kyle           Carter               45   Active (Hash Table [86])
884455667   Jonathan       Walker               36   Active (Hash Table [87])
884567890   Robert         Jones                33   Active (Hash Table [92])
991234567   Maria          Rodriguez            24   Active (Overflow Table [0])
992345678   Sarah          Williams             45   Active (Overflow Table [1])
993456789   Jennifer       Davis                27   Active (Overflow Table [2])
994567890   Emily          Wilson               42   Active (Overflow Table [3])
995678901   Jessica        Moore                39   Active (Overflow Table [4])
996789012   Amanda         Anderson             15   Active (Overflow Table [5])
887890123   Daniel         Thomas               30   Active (Overflow Table [6])
997890123   Olivia         Jackson              27   Active (Overflow Table [7])
998901234   Sophia         Harris               24   Active (Overflow Table [8])
999012345   Emma           Thompson             33   Active (Overflow Table [9])
991122334   Natalie        Martinez             45   Active (Overflow Table [10])
992233445   Ava            Clark                30   Active (Overflow Table [11])
993344556   Mia            Lee                  27   Active (Overflow Table [12])
994455667   Isabella       Hall                 24   Active (Overflow Table [13])
995566778   Chloe          Young                15   Active (Overflow Table [14])
996677889   Grace          Wright               48   Active (Overflow Table [15])
997788990   Lily           Turner               33   Active (Overflow Table [16])
990012345   Abigail        Nelson               36   Active (Overflow Table [17])
990123456   Hannah         Mitchell             24   Active (Overflow Table [18])
990234567   Victoria       Roberts              39   Active (Overflow Table [19])


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 6

===== UNPROCESSED RECORDS =====
990345678 Zoe Morgan 42


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 7
Exiting program...

TEST CASE: All menu options tested for valid+invalid combos (existant/non-existant results for search options)

===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 1
Enter hash table size (10, 100, or 1000): 10
Created hash table with 10 slots and overflow table with 2 slots
Loading records from in_students.txt...
Both hash and overflow tables are full. 30 records could not be processed
Wrote 30 unprocessed records to out_unprocessed.txt
Processed 10 records from the file.


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 2
Enter student ID to search for: 123
Record with ID 123 not found


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 2
Enter student ID to search for: 991234567
Record found in overflow table at position 0:

===== SEARCH RESULT =====
Student ID  First Name     Last Name         Units   Status
------------------------------------------------------------
991234567   Maria          Rodriguez            24   Active


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 3
Enter student ID to archive: 123
Record with ID 123 not found


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 3
Enter student ID to archive: 9912345678
Record with ID 2147483647 not found


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: Invalid option. Please enter an option between 1 and 7


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 4

===== ACTIVE RECORDS =====
Student ID  First Name     Last Name         Units   Status
------------------------------------------------------------
881234567   John           Smith                30   Active (Hash Table [0])
883344556   Steven         Lewis                39   Active (Hash Table [1])
884567890   Robert         Jones                33   Active (Hash Table [2])
881122334   Kevin          Garcia               21   Active (Hash Table [3])
885566778   Joshua         Allen                42   Active (Hash Table [4])
883456789   Michael        Brown                18   Active (Hash Table [5])
880234567   Samuel         Perez                18   Active (Hash Table [7])
882345678   David          Johnson              36   Active (Hash Table [8])
991234567   Maria          Rodriguez            24   Active (Overflow Table [0])
992345678   Sarah          Williams             45   Active (Overflow Table [1])


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 5

===== ARCHIVED RECORDS =====
Student ID  First Name     Last Name         Units   Status
------------------------------------------------------------
No archived records found.


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 3
Enter student ID to archive: 991234567
Record with ID 991234567 has been archived


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 5

===== ARCHIVED RECORDS =====
Student ID  First Name     Last Name         Units   Status
------------------------------------------------------------
991234567   Maria          Rodriguez            24   Archived (Overflow Table [0])


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 6

===== UNPROCESSED RECORDS =====
993456789 Jennifer Davis 27
994567890 Emily Wilson 42
885678901 James Miller 21
995678901 Jessica Moore 39
886789012 Christopher Taylor 48
996789012 Amanda Anderson 15
887890123 Daniel Thomas 30
997890123 Olivia Jackson 27
888901234 Matthew White 36
998901234 Sophia Harris 24
889012345 Andrew Martin 42
999012345 Emma Thompson 33
991122334 Natalie Martinez 45
882233445 Brian Robinson 18
992233445 Ava Clark 30
993344556 Mia Lee 27
884455667 Jonathan Walker 36
994455667 Isabella Hall 24
995566778 Chloe Young 15
886677889 Justin King 30
996677889 Grace Wright 48
887788990 Brandon Scott 21
997788990 Lily Turner 33
880012345 Tyler Phillips 27
990012345 Abigail Nelson 36
880123456 Kyle Carter 45
990123456 Hannah Mitchell 24
990234567 Victoria Roberts 39
880345678 Nathan Cook 30
990345678 Zoe Morgan 42


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: g
Invalid option. Please enter an option between 1 and 7


===== MAIN MENU =====
1. Create hash tables and load records
2. Search for a record by ID
3. Archive a record
4. Print all active records
5. Print all archived records
6. Print unprocessed records
7. Quit
Choice: 7
Exiting program...

*/
