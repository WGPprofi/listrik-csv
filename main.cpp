// main.cpp - versi dengan penghapusan sisa sebelumnya dan fitur "baru" + skip
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstdlib> 

using namespace std;

const string USER_FILE = "user_list.txt";
const string CSV_FILE = "data.csv";

string normalizeName(const string& name) {
    string n = name;
    n.erase(0, n.find_first_not_of(" \t"));
    n.erase(n.find_last_not_of(" \t") + 1);
    transform(n.begin(), n.end(), n.begin(), ::tolower);
    if (!n.empty()) n[0] = toupper(n[0]);
    return n;
}

vector<string> loadUsers() {
    vector<string> users;
    ifstream file(USER_FILE);
    string name;
    while (getline(file, name)) {
        name = normalizeName(name);
        if (!name.empty()) users.push_back(name);
    }
    return users;
}

void saveUser(const string& user) {
    ofstream file(USER_FILE, ios::app);
    file << user << "\n";
}

void saveUserList(const vector<string>& users) {
    ofstream file(USER_FILE);
    for (const string& u : users) file << u << "\n";
}

void initializeCSV(const vector<string>& users) {
    ifstream inFile(CSV_FILE);
    vector<string> lines;
    string line;
    bool hasHeader = false;
    vector<string> existingNames;

    while (getline(inFile, line)) {
        if (line.empty()) continue;
        if (!hasHeader) {
            hasHeader = true;
            lines.push_back("Nama,Tanggal,Total");
        } else {
            stringstream ss(line);
            string name;
            getline(ss, name, ',');
            existingNames.push_back(normalizeName(name));
            lines.push_back(line);
        }
    }
    inFile.close();

    if (!hasHeader) lines.push_back("Nama,Tanggal,Total");
    for (const string& user : users) {
        if (find(existingNames.begin(), existingNames.end(), normalizeName(user)) == existingNames.end()) {
            lines.push_back(user + ",,");
        }
    }

    ofstream outFile(CSV_FILE);
    for (const string& l : lines) outFile << l << "\n";
    outFile.close();
}

void checkAndInitCSV() {
    vector<string> users = loadUsers();
    ifstream file(CSV_FILE);
    bool reset = false;

    if (!file.good()) reset = true;
    else {
        file.seekg(0, ios::end);
        if (file.tellg() == 0) reset = true;
        else {
            file.seekg(0, ios::beg);
            string header;
            getline(file, header);
            if (header != "Nama,Tanggal,Total") reset = true;
        }
    }
    file.close();
    initializeCSV(users);
}

void listUser () {

    vector<string> users = loadUsers();
    cout << "\nList User\n";
    for (size_t i = 0; i < users.size(); ++i) cout << i+1 << ". " << users[i] << "\n";

}

void tambahUser() {
    string nama;

    listUser();
    cout << "Masukkan nama user baru: ";
    getline(cin, nama);
    nama = normalizeName(nama);

    vector<string> users = loadUsers();
    if (find(users.begin(), users.end(), nama) != users.end()) {
        cout << "⚠️  Nama sudah ada.\n";
        return;
    }
    saveUser(nama);
    initializeCSV(loadUsers());
    cout << "✅ User ditambahkan.\n";
}

void hapusUser() {
    vector<string> users = loadUsers();
    if (users.empty()) {
        cout << "⚠️  Tidak ada user.\n";
        return;
    }

    listUser();
    cout << "Nomor user: ";
    int idx; cin >> idx; cin.ignore();
    if (idx < 1 || idx > users.size()) {
        cout << "❌ Pilihan tidak valid.\n";
        return;
    }

    string toDelete = users[idx-1];
    users.erase(users.begin() + idx - 1);
    saveUserList(users);

    ifstream in(CSV_FILE);
    vector<string> out;
    string line;
    getline(in, line); // header
    out.push_back(line);
    while (getline(in, line)) {
        stringstream ss(line);
        string n; getline(ss, n, ',');
        if (normalizeName(n) != normalizeName(toDelete)) out.push_back(line);
    }
    in.close();

    ofstream o(CSV_FILE);
    for (const auto& l : out) o << l << "\n";
    o.close();

    cout << "✅ User dan data dihapus.\n";
}

bool updateOrAppendRow(const string& user, const string& tanggal, const string& total) {
    ifstream fileIn(CSV_FILE);
    vector<string> lines;
    string line;
    bool updated = false;

    while (getline(fileIn, line)) lines.push_back(line);
    fileIn.close();

    for (size_t i = 1; i < lines.size(); ++i) {
        stringstream ss(lines[i]);
        string nama, tgl, ttl;
        getline(ss, nama, ','); getline(ss, tgl, ','); getline(ss, ttl, ',');

        // Baris bisa ditimpa jika: user cocok && (tgl kosong dan total kosong/0)
        if (normalizeName(nama) == user &&
            (tgl.empty() || ttl.empty() || ttl == "0")) {

            lines[i] = user + "," + tanggal + "," + total;
            updated = true;
            break;
        }
    }

    if (updated) {
        ofstream fileOut(CSV_FILE);
        for (const auto& l : lines) fileOut << l << "\n";
    } else {
        ofstream fileOut(CSV_FILE, ios::app);
        fileOut << user + "," + tanggal + "," + total << "\n";
    }
    return true;
}

// Tambah N entri "baru" untuk user+tanggal: isi slot kosong dulu lalu append
void addBaruSlots(const string& user, const string& tanggal, int count) {
    ifstream fileIn(CSV_FILE);
    vector<string> lines;
    string line;
    while (getline(fileIn, line)) lines.push_back(line);
    fileIn.close();

    int added = 0;

    // isi slot kosong dulu (nama cocok, tgl kosong atau total kosong/0)
    for (size_t i = 1; i < lines.size() && added < count; ++i) {
        stringstream ss(lines[i]);
        string nama, tgl, ttl;
        getline(ss, nama, ',');
        getline(ss, tgl, ',');
        getline(ss, ttl, ',');

        if (normalizeName(nama) == user &&
            (tgl.empty() || ttl.empty() || ttl == "0")) {
            lines[i] = user + "," + tanggal + ",baru";
            added++;
        }
    }

    ofstream fileOut(CSV_FILE);
    for (const string& l : lines) fileOut << l << "\n";
    for (; added < count; ++added) {
        fileOut << user << "," << tanggal << ",baru\n";
    }
}

void inputListrik() {
    vector<string> users = loadUsers();
    if (users.empty()) {
        cout << "⚠️  Tidak ada user.\n";
        return;
    }

    for (size_t i = 0; i < users.size(); ++i) cout << i + 1 << ". " << users[i] << "\n";
    cout << "Pilih user: ";
    int idx; cin >> idx; cin.ignore();
    if (idx < 1 || idx > users.size()) return;
    string nama = users[idx - 1];

    string tanggal, totalStr;
    cout << "Tanggal (YYYY-MM-DD): "; getline(cin, tanggal);
    cout << "Total Bayar (atau ketik \"baru\"): "; getline(cin, totalStr);

    bool is_baru = false;
    int skip_count = 0;
    int total = 0;
    int kelipatan = 0;
    int sisa = 0;

    if (totalStr == "baru") {
        is_baru = true;
        cout << "Berapa kali untuk skip: ";
        string skipStr;
        getline(cin, skipStr);
        try {
            skip_count = stoi(skipStr);
            if (skip_count < 0) skip_count = 0;
        } catch (...) {
            skip_count = 0;
        }
    } else {
        try {
            total = stoi(totalStr);
        } catch (...) {
            cout << "Input tidak valid.\n";
            return;
        }
        kelipatan = total / 20000;
        sisa = total % 20000;
    }

    // kalau bukan "baru", hapus sisa sebelumnya
    int sisa_terakhir = 0;
    if (!is_baru) {
        ifstream inFile(CSV_FILE);
        vector<string> newLines;
        string line;
        while (getline(inFile, line)) {
            stringstream ss(line);
            string n, tgl, ttl;
            getline(ss, n, ','); getline(ss, tgl, ','); getline(ss, ttl, ',');

            if (normalizeName(n) == normalizeName(nama)) {
                try {
                    int val = stoi(ttl);
                    if (val % 20000 != 0) {
                        sisa_terakhir = val;
                        continue; // skip baris ini (hapus sisa sebelumnya)
                    }
                } catch (...) {}
            }
            newLines.push_back(line);
        }
        inFile.close();

        ofstream outFile(CSV_FILE);
        for (const auto& l : newLines) outFile << l << "\n";
        outFile.close();
    }

    if (is_baru) {
        addBaruSlots(nama, tanggal, skip_count);
    } else {
        for (int i = 0; i < kelipatan; ++i)
            updateOrAppendRow(nama, tanggal, "20000");

        int total_sisa = sisa + sisa_terakhir;
        if (total_sisa > 0)
            updateOrAppendRow(nama, tanggal, to_string(total_sisa));
    }

    cout << "✅ Input selesai.\n";
}

void pushToGitHub() {
    int res;

    cout << "\n🌀 Push ke GitHub...\n";

    res = system("git add data.csv");
    if (res != 0) {
        cout << "❌ Gagal git add.\n";
        return;
    }

    res = system("git commit -m \"Update data.csv dari C++\"");
    if (res != 0) {
        cout << "⚠️  Tidak ada perubahan untuk commit.\n";
    }

    res = system("git push origin main");
    if (res != 0) {
        cout << "❌ Gagal push. Coba cek koneksi atau autentikasi Git.\n";
        return;
    }

    cout << "✅ data.csv berhasil di-push ke GitHub.\n";
}

int main() {
    checkAndInitCSV();
    int pilihan;
    do {
        system("cls");
        cout << "\n=== Menu ===\n1. Tambah user\n2. Hapus user\n3. Input listrik\n0. Keluar\nPilihan: ";
        cin >> pilihan; cin.ignore();
        switch (pilihan) {
            case 1: tambahUser(); cin.get(); break;
            case 2: hapusUser(); cin.get(); break;
            case 3: inputListrik(); cin.get(); break;
            case 4: listUser(); cin.get(); break;
            case 0: cout << "Keluar.\n"; break;
            default: cout << "Pilihan tidak valid.\n";
        }
    } while (pilihan != 0);

        pushToGitHub();
        cin.get();
    return 0;
}
