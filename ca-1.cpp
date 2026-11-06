// CAP455 CA1 - Courier Parcel Sorting Console

#include <iostream>
#include <string>
#include <iomanip>
#include <cctype>
#include <cstdlib>
using namespace std;

const int MAX_PARCELS = 50;

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
    void setData(string trackingId, string sender, string receiver,
                 string city, int pinCode, double weight, char type);

    string getId() { return trackingId; }
    string getZone() { return zone; }

    inline double charge() const;

    static int getCount() { return count; }
    static void increaseCount() { count++; }

    friend void printParcel(const Parcel &p);
};

int Parcel::count = 0;

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
        int len = s.length();
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
        int len = s.length();
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
    int len = s.length();
    for (int i = 0; i < len; i++)
        s[i] = toupper(s[i]);
    return s;
}

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
    int len = name.length();
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

Parcel *findParcel(Parcel list[], string id)
{
    Parcel *p = list;
    for (int i = 0; i < Parcel::getCount(); i++)
    {
        if ((p + i)->getId() == id)
            return p + i;
    }
    return NULL;
}

void addParcel(Parcel list[])
{
    if (Parcel::getCount() == MAX_PARCELS)
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
        else if (findParcel(list, id) != NULL)
            cout << "  This ID already exists. Enter a different one.\n";
        else
            break;
    }

    string sender = trim(readLine("Sender name: "));
    while (!isValidName(sender, 15))
    {
        cout << "  Only letters and spaces, max 15 characters.\n";
        sender = trim(readLine("Sender name: "));
    }

    string receiver = trim(readLine("Receiver name: "));
    while (!isValidName(receiver, 15))
    {
        cout << "  Only letters and spaces, max 15 characters.\n";
        receiver = trim(readLine("Receiver name: "));
    }

    string city = trim(readLine("Destination city: "));
    while (!isValidName(city, 20))
    {
        cout << "  Only letters and spaces, max 20 characters.\n";
        city = trim(readLine("Destination city: "));
    }

    int pin = readInt("PIN code: ");
    while (!isValidPin(pin))
    {
        cout << "  PIN must be 6 digits and start with 1 to 9.\n";
        pin = readInt("PIN code: ");
    }

    double weight = readDouble("Weight in kg (max 50): ");
    while (weight <= 0 || weight > 50)
    {
        cout << "  Weight must be more than 0 and up to 50 kg.\n";
        weight = readDouble("Weight in kg (max 50): ");
    }

    char type;
    while (true)
    {
        string t = toUpper(trim(readLine("Type (N = Normal, E = Express): ")));
        if (t == "N" || t == "E")
        {
            type = t[0];
            break;
        }
        cout << "  Enter N or E only.\n";
    }

    Parcel *p = &list[Parcel::getCount()];
    p->setData(id, sender, receiver, city, pin, weight, type);
    Parcel::increaseCount();

    cout << "\nParcel " << id << " added. Send it to the " << p->getZone() << " bin.\n";
}

void showAll(Parcel list[])
{
    if (Parcel::getCount() == 0)
    {
        cout << "No parcels added yet.\n";
        return;
    }

    printHeader();
    Parcel *p = list;
    for (int i = 0; i < Parcel::getCount(); i++)
    {
        printParcel(*(p + i));
    }
    cout << "\nTotal parcels: " << Parcel::getCount() << endl;
}

void searchParcel(Parcel list[])
{
    string id = toUpper(trim(readLine("Enter tracking ID to search: ")));
    Parcel *p = findParcel(list, id);

    if (p == NULL)
    {
        cout << "No parcel found with ID " << id << endl;
        return;
    }

    cout << "\nParcel found:\n";
    printHeader();
    printParcel(*p);
}

void showZoneWise(Parcel list[])
{
    if (Parcel::getCount() == 0)
    {
        cout << "No parcels added yet.\n";
        return;
    }

    string zones[5] = {"NORTH", "WEST", "SOUTH", "EAST", "ARMY"};

    for (int z = 0; z < 5; z++)
    {
        int found = 0;
        cout << "\n===== " << zones[z] << " BIN =====\n";

        for (int i = 0; i < Parcel::getCount(); i++)
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

int main()
{
    Parcel parcels[MAX_PARCELS];
    int choice;

    cout << "=====================================\n";
    cout << "   COURIER PARCEL SORTING CONSOLE\n";
    cout << "=====================================\n";

    do
    {
        cout << "\n1. Add parcel\n";
        cout << "2. Show all parcels\n";
        cout << "3. Search parcel by ID\n";
        cout << "4. Show parcels zone-wise\n";
        cout << "5. Exit\n";

        choice = readInt("Enter choice: ");
        cout << endl;

        switch (choice)
        {
        case 1:
            addParcel(parcels);
            break;
        case 2:
            showAll(parcels);
            break;
        case 3:
            searchParcel(parcels);
            break;
        case 4:
            showZoneWise(parcels);
            break;
        case 5:
            cout << "Goodbye!\n";
            break;
        default:
            cout << "Wrong choice, enter 1 to 5.\n";
        }
    } while (choice != 5);

    return 0;
}
