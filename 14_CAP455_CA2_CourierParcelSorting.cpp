// CAP455 CA2 - Courier Parcel Sorting Console (Persist & Search)

#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <iomanip>
#include <cctype>
#include <cstdlib>
using namespace std;

const int MAX_PARCELS = 50;
const char DATA_FILE[] = "parcels.dat";

// one fixed-size record in the binary file
struct ParcelRecord
{
    char trackingId[7];
    char sender[16];
    char receiver[16];
    char city[21];
    int pinCode;
    double weight;
    char type;
    char zone[6];
};

void copyText(char dest[], string src, int size)
{
    int i = 0;
    while (i < size - 1 && i < (int)src.length())
    {
        dest[i] = src[i];
        i++;
    }
    dest[i] = '\0';
}

class Parcel
{
private:
    string trackingId;
    string sender;
    string receiver;
    string city;
    int pinCode;
    double weight;
    char type;   // N or E
    string zone;

    static int count;

    string findZone();

public:
    Parcel();
    Parcel(string trackingId, string sender, string receiver,
           string city, int pinCode, double weight, char type);
    Parcel(const Parcel &other);
    ~Parcel();

    void setData(string trackingId, string sender, string receiver,
                 string city, int pinCode, double weight, char type);
    void updateDelivery(string city, int pinCode, double weight, char type);

    string getId() { return trackingId; }
    string getZone() { return zone; }

    inline double charge() const;

    ParcelRecord toRecord() const;
    void fromRecord(const ParcelRecord &r);

    static int getCount() { return count; }
    static void setCount(int n) { count = n; }
    static void increaseCount() { count++; }

    friend void printParcel(const Parcel &p);
};

int Parcel::count = 0;

// default constructor
Parcel::Parcel()
{
    trackingId = "";
    sender = "";
    receiver = "";
    city = "";
    pinCode = 0;
    weight = 0;
    type = 'N';
    zone = "";
}

// parameterised constructor
Parcel::Parcel(string trackingId, string sender, string receiver,
               string city, int pinCode, double weight, char type)
{
    setData(trackingId, sender, receiver, city, pinCode, weight, type);
}

// copy constructor
Parcel::Parcel(const Parcel &other)
{
    trackingId = other.trackingId;
    sender = other.sender;
    receiver = other.receiver;
    city = other.city;
    pinCode = other.pinCode;
    weight = other.weight;
    type = other.type;
    zone = other.zone;
}

// destructor - nothing to free, strings clean up themselves
Parcel::~Parcel()
{
}

void Parcel::setData(string trackingId, string sender, string receiver,
                     string city, int pinCode, double weight, char type)
{
    this->trackingId = trackingId;
    this->sender = sender;
    this->receiver = receiver;
    this->city = city;
    this->pinCode = pinCode;
    this->weight = weight;
    this->type = type;
    this->zone = findZone();
}

void Parcel::updateDelivery(string city, int pinCode, double weight, char type)
{
    this->city = city;
    this->pinCode = pinCode;
    this->weight = weight;
    this->type = type;
    this->zone = findZone();
}

string Parcel::findZone()
{
    int firstDigit = pinCode / 100000;

    switch (firstDigit)
    {
    case 1:
    case 2:
        return "NORTH";
    case 3:
    case 4:
        return "WEST";
    case 5:
    case 6:
        return "SOUTH";
    case 7:
    case 8:
        return "EAST";
    case 9:
        return "ARMY";
    }
    return "UNKNOWN";
}

// 40 base + 20 per kg, 50 extra for express
inline double Parcel::charge() const
{
    double amount = 40 + weight * 20;
    if (type == 'E')
        amount = amount + 50;
    return amount;
}

ParcelRecord Parcel::toRecord() const
{
    ParcelRecord r;
    memset(&r, 0, sizeof(r));
    copyText(r.trackingId, trackingId, sizeof(r.trackingId));
    copyText(r.sender, sender, sizeof(r.sender));
    copyText(r.receiver, receiver, sizeof(r.receiver));
    copyText(r.city, city, sizeof(r.city));
    copyText(r.zone, zone, sizeof(r.zone));
    r.pinCode = pinCode;
    r.weight = weight;
    r.type = type;
    return r;
}

void Parcel::fromRecord(const ParcelRecord &r)
{
    trackingId = r.trackingId;
    sender = r.sender;
    receiver = r.receiver;
    city = r.city;
    zone = r.zone;
    pinCode = r.pinCode;
    weight = r.weight;
    type = r.type;
}

void printParcel(const Parcel &p)
{
    string typeName = "Normal";
    if (p.type == 'E')
        typeName = "Express";

    cout << left << setw(8) << p.trackingId
         << setw(16) << p.sender
         << setw(16) << p.receiver
         << setw(21) << p.city
         << setw(8) << p.pinCode
         << setw(7) << fixed << setprecision(2) << p.weight
         << setw(9) << typeName
         << setw(7) << p.zone
         << p.charge() << endl;
}

void printHeader()
{
    cout << left << setw(8) << "ID"
         << setw(16) << "Sender"
         << setw(16) << "Receiver"
         << setw(21) << "City"
         << setw(8) << "PIN"
         << setw(7) << "Kg"
         << setw(9) << "Type"
         << setw(7) << "Zone"
         << "Charge" << endl;
    cout << string(99, '-') << endl;
}

// ---------- file handling ----------

int countRecords()
{
    ifstream file(DATA_FILE, ios::binary);
    if (!file)
        return 0;

    file.seekg(0, ios::end);
    long size = (long)file.tellg();
    return size / sizeof(ParcelRecord);
}

bool readRecord(int index, Parcel &p)
{
    ifstream file(DATA_FILE, ios::binary);
    if (!file)
        return false;

    ParcelRecord r;
    file.seekg(index * sizeof(ParcelRecord), ios::beg);
    file.read((char *)&r, sizeof(r));
    if (!file)
        return false;

    p.fromRecord(r);
    return true;
}

bool appendRecord(const Parcel &p)
{
    ofstream file(DATA_FILE, ios::binary | ios::app);
    if (!file)
        return false;

    ParcelRecord r = p.toRecord();
    file.write((char *)&r, sizeof(r));
    return (bool)file;
}

bool writeRecord(int index, const Parcel &p)
{
    fstream file(DATA_FILE, ios::binary | ios::in | ios::out);
    if (!file)
        return false;

    ParcelRecord r = p.toRecord();
    file.seekp(index * sizeof(ParcelRecord), ios::beg);
    file.write((char *)&r, sizeof(r));
    return (bool)file;
}

// returns record number of the parcel, or -1 if not found
int findPosition(string id)
{
    ifstream file(DATA_FILE, ios::binary);
    if (!file)
        return -1;

    ParcelRecord r;
    for (int i = 0; i < Parcel::getCount(); i++)
    {
        file.seekg(i * sizeof(ParcelRecord), ios::beg);
        file.read((char *)&r, sizeof(r));
        if (!file)
            return -1;
        if (id == r.trackingId)
            return i;
    }
    return -1;
}

// reads every record into the array, returns how many were read
int loadAll(Parcel list[])
{
    int n = 0;
    Parcel *p = list;
    while (n < Parcel::getCount() && n < MAX_PARCELS)
    {
        if (!readRecord(n, *(p + n)))
            break;
        n++;
    }
    return n;
}

// ---------- input ----------

string readLine(string msg)
{
    string s;
    cout << msg;
    if (!getline(cin, s))
        exit(0);
    return s;
}

string trim(string s)
{
    while (s.length() > 0 && s[0] == ' ')
        s.erase(0, 1);
    while (s.length() > 0 && s[s.length() - 1] == ' ')
        s.erase(s.length() - 1, 1);
    return s;
}

int readInt(string msg)
{
    while (true)
    {
        string s = trim(readLine(msg));
        int len = (int)s.length();
        bool ok = true;

        if (len == 0 || len > 9)
            ok = false;
        for (int i = 0; i < len; i++)
        {
            if (!isdigit(s[i]))
                ok = false;
        }

        if (ok)
            return atoi(s.c_str());
        cout << "  Please enter a whole number.\n";
    }
}

double readDouble(string msg)
{
    while (true)
    {
        string s = trim(readLine(msg));
        int len = (int)s.length();
        int digits = 0, dots = 0;
        bool ok = true;

        if (len > 9)
            ok = false;
        for (int i = 0; i < len; i++)
        {
            if (isdigit(s[i]))
                digits++;
            else if (s[i] == '.')
                dots++;
            else
                ok = false;
        }

        if (ok && digits > 0 && dots < 2)
            return atof(s.c_str());
        cout << "  Please enter a number.\n";
    }
}

string toUpper(string s)
{
    int len = (int)s.length();
    for (int i = 0; i < len; i++)
        s[i] = toupper(s[i]);
    return s;
}

// ---------- validation ----------

bool isValidId(string id)
{
    if (id.length() != 6)
        return false;
    if (id[0] != 'C' || id[1] != 'P')
        return false;
    for (int i = 2; i < 6; i++)
    {
        if (!isdigit(id[i]))
            return false;
    }
    return true;
}

bool isValidName(string name, int maxLength)
{
    int len = (int)name.length();
    if (len == 0 || len > maxLength)
        return false;
    for (int i = 0; i < len; i++)
    {
        if (!isalpha(name[i]) && name[i] != ' ')
            return false;
    }
    return true;
}

bool isValidPin(int pin)
{
    return pin >= 110000 && pin <= 999999;
}

string askName(string msg, int maxLength)
{
    string s = trim(readLine(msg));
    while (!isValidName(s, maxLength))
    {
        cout << "  Only letters and spaces, max " << maxLength << " characters.\n";
        s = trim(readLine(msg));
    }
    return s;
}

int askPin()
{
    int pin = readInt("PIN code: ");
    while (!isValidPin(pin))
    {
        cout << "  PIN must be 6 digits and start with 1 to 9.\n";
        pin = readInt("PIN code: ");
    }
    return pin;
}

double askWeight()
{
    double weight = readDouble("Weight in kg (max 50): ");
    while (weight <= 0 || weight > 50)
    {
        cout << "  Weight must be more than 0 and up to 50 kg.\n";
        weight = readDouble("Weight in kg (max 50): ");
    }
    return weight;
}

char askType()
{
    while (true)
    {
        string t = toUpper(trim(readLine("Type (N = Normal, E = Express): ")));
        if (t == "N" || t == "E")
            return t[0];
        cout << "  Enter N or E only.\n";
    }
}

// ---------- operations ----------

void addParcel()
{
    if (Parcel::getCount() >= MAX_PARCELS)
    {
        cout << "Storage is full. Cannot add more parcels.\n";
        return;
    }

    string id;
    while (true)
    {
        id = toUpper(trim(readLine("Tracking ID (e.g. CP1001): ")));
        if (!isValidId(id))
            cout << "  Invalid ID. Use CP followed by 4 digits.\n";
        else if (findPosition(id) != -1)
            cout << "  This ID already exists. Enter a different one.\n";
        else
            break;
    }

    string sender = askName("Sender name: ", 15);
    string receiver = askName("Receiver name: ", 15);
    string city = askName("Destination city: ", 20);
    int pin = askPin();
    double weight = askWeight();
    char type = askType();

    Parcel p(id, sender, receiver, city, pin, weight, type);
    if (!appendRecord(p))
    {
        cout << "Could not save the parcel to the file.\n";
        return;
    }
    Parcel::increaseCount();

    cout << "\nParcel " << id << " saved. Send it to the " << p.getZone() << " bin.\n";
}

void showAll()
{
    if (Parcel::getCount() == 0)
    {
        cout << "No parcels saved yet.\n";
        return;
    }

    printHeader();
    Parcel p;
    for (int i = 0; i < Parcel::getCount(); i++)
    {
        if (readRecord(i, p))
            printParcel(p);
    }
    cout << "\nTotal parcels: " << Parcel::getCount() << endl;
}

void searchParcel()
{
    string id = toUpper(trim(readLine("Enter tracking ID to search: ")));
    int pos = findPosition(id);

    if (pos == -1)
    {
        cout << "No parcel found with ID " << id << endl;
        return;
    }

    Parcel p;
    readRecord(pos, p);
    cout << "\nParcel found at record number " << pos + 1 << ":\n";
    printHeader();
    printParcel(p);
}

void showZoneWise()
{
    if (Parcel::getCount() == 0)
    {
        cout << "No parcels saved yet.\n";
        return;
    }

    Parcel list[MAX_PARCELS];
    int n = loadAll(list);
    string zones[5] = {"NORTH", "WEST", "SOUTH", "EAST", "ARMY"};

    for (int z = 0; z < 5; z++)
    {
        int found = 0;
        cout << "\n===== " << zones[z] << " BIN =====\n";

        for (int i = 0; i < n; i++)
        {
            if (list[i].getZone() == zones[z])
            {
                if (found == 0)
                    printHeader();
                printParcel(list[i]);
                found++;
            }
        }

        if (found == 0)
            cout << "(empty)\n";
        else
            cout << "Parcels in this bin: " << found << endl;
    }
}

void updateParcel()
{
    string id = toUpper(trim(readLine("Enter tracking ID to update: ")));
    int pos = findPosition(id);

    if (pos == -1)
    {
        cout << "No parcel found with ID " << id << endl;
        return;
    }

    Parcel p;
    readRecord(pos, p);
    Parcel before(p);   // copy constructor keeps the old details

    cout << "\nCurrent details:\n";
    printHeader();
    printParcel(before);

    cout << "\nEnter the new delivery details.\n";
    string city = askName("Destination city: ", 20);
    int pin = askPin();
    double weight = askWeight();
    char type = askType();

    p.updateDelivery(city, pin, weight, type);
    if (!writeRecord(pos, p))
    {
        cout << "Could not update the file.\n";
        return;
    }

    cout << "\nParcel " << id << " updated.\n";
    printHeader();
    cout << "Before:\n";
    printParcel(before);
    cout << "After:\n";
    printParcel(p);
}

int main()
{
    Parcel::setCount(countRecords());

    cout << "=====================================\n";
    cout << "   COURIER PARCEL SORTING CONSOLE\n";
    cout << "=====================================\n";
    cout << Parcel::getCount() << " parcel(s) loaded from " << DATA_FILE << endl;

    int choice;
    do
    {
        cout << "\n1. Add parcel\n";
        cout << "2. Show all parcels\n";
        cout << "3. Search parcel by ID\n";
        cout << "4. Show parcels zone-wise\n";
        cout << "5. Update parcel\n";
        cout << "6. Exit\n";

        choice = readInt("Enter choice: ");
        cout << endl;

        switch (choice)
        {
        case 1:
            addParcel();
            break;
        case 2:
            showAll();
            break;
        case 3:
            searchParcel();
            break;
        case 4:
            showZoneWise();
            break;
        case 5:
            updateParcel();
            break;
        case 6:
            cout << "Goodbye!\n";
            break;
        default:
            cout << "Wrong choice, enter 1 to 6.\n";
        }
    } while (choice != 6);

    return 0;
}
