/*
 * ================================================================
 *   Hospital Patient Scheduling System  v2.0
 *   Single-file C++ console application
 *
 *   Data files created / used:
 *     patients.txt      – patient records
 *     doctors.txt       – doctor records
 *     appointments.txt  – appointment / scheduling records
 *     departments.txt   – department info & bed counts
 *     discharge.txt     – discharge summaries
 *     logs.txt          – timestamped activity log
 * ================================================================
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <queue>
#include <string>
#include <algorithm>
#include <iomanip>
#include <ctime>
#include <limits>
#include <map>

using namespace std;

// ════════════════════════════════════════════════════════
//  FILE NAME CONSTANTS
// ════════════════════════════════════════════════════════
const string FILE_PATIENTS     = "patients.txt";
const string FILE_DOCTORS      = "doctors.txt";
const string FILE_APPOINTMENTS = "appointments.txt";
const string FILE_DEPARTMENTS  = "departments.txt";
const string FILE_DISCHARGE    = "discharge.txt";
const string FILE_LOGS         = "logs.txt";

// ════════════════════════════════════════════════════════
//  DATA STRUCTURES
// ════════════════════════════════════════════════════════

struct Patient {
    int    id;
    string name;
    int    age;
    string gender;
    string phone;
    string bloodGroup;
    int    severity;        // 1 (low) - 10 (critical)
    string department;
    string assignedDoctor;
    string admitDate;
    string status;          // Active | Discharged | Under Observation
    string notes;

    bool operator<(const Patient& o) const { return severity < o.severity; }
};

struct Doctor {
    int    id;
    string name;
    string specialization;
    string department;
    string qualification;
    string phone;
    int    experience;      // years
    string availability;    // Morning | Evening | Night | On-Call
    int    patientCount;
};

struct Appointment {
    int    id;
    int    patientId;
    string patientName;
    int    doctorId;
    string doctorName;
    string department;
    string date;
    string time;
    string purpose;
    string status;          // Scheduled | Completed | Cancelled
};

struct Department {
    string name;
    int    totalBeds;
    int    occupiedBeds;
    string headDoctor;
    string floor;
    string phone;
};

// ════════════════════════════════════════════════════════
//  STATIC DEPARTMENT & DOCTOR CATALOG
// ════════════════════════════════════════════════════════

struct DeptInfo { string name, floor, phone; int totalBeds; };

const vector<DeptInfo> DEPT_CATALOG = {
    {"Cardiology",        "3rd Floor", "ext-301", 30},
    {"Neurology",         "4th Floor", "ext-401", 25},
    {"Orthopedics",       "2nd Floor", "ext-201", 35},
    {"Pediatrics",        "1st Floor", "ext-101", 40},
    {"Oncology",          "5th Floor", "ext-501", 20},
    {"Emergency",         "Ground",    "ext-001", 50},
    {"General Medicine",  "1st Floor", "ext-102", 45},
    {"Gynecology",        "2nd Floor", "ext-202", 30},
    {"Dermatology",       "3rd Floor", "ext-302", 15},
    {"Psychiatry",        "4th Floor", "ext-402", 20},
    {"Ophthalmology",     "3rd Floor", "ext-303", 10},
    {"ENT",               "2nd Floor", "ext-203", 15},
    {"Urology",           "4th Floor", "ext-403", 20},
    {"Pulmonology",       "5th Floor", "ext-502", 25},
    {"ICU",               "Ground",    "ext-002", 15}
};

// Doctors per department  tuple<name, qualification, experience_years>
const map<string, vector<tuple<string,string,int>>> DOCTOR_CATALOG = {
    {"Cardiology",       {{"Dr. Arjun Sharma","MD, DM Cardiology",18},{"Dr. Priya Nair","MBBS, MD, FACC",12},{"Dr. Suresh Mehta","MD, DNB Cardiology",22}}},
    {"Neurology",        {{"Dr. Kavita Rao","MD, DM Neurology",15},{"Dr. Rajan Gupta","MBBS, MD, DNB",10},{"Dr. Ananya Singh","MD, PhD Neuroscience",8}}},
    {"Orthopedics",      {{"Dr. Vikram Patel","MS Ortho, FICS",20},{"Dr. Neha Joshi","MBBS, MS Ortho",9},{"Dr. Harish Verma","MS, MCh Ortho",14}}},
    {"Pediatrics",       {{"Dr. Sunita Agarwal","MD Pediatrics, DCH",16},{"Dr. Rohan Malhotra","MBBS, MD Peds",7},{"Dr. Meena Kapoor","MD, DNB Pediatrics",11}}},
    {"Oncology",         {{"Dr. Rajiv Bose","MD, DM Oncology",25},{"Dr. Shalini Desai","MBBS, MD, DNB Onco",13},{"Dr. Anil Kumar","MD, PhD Oncology",19}}},
    {"Emergency",        {{"Dr. Deepak Tiwari","MD Emergency Med",11},{"Dr. Pooja Sharma","MBBS, MD, FCEM",6},{"Dr. Manish Khanna","MD, Critical Care",14}}},
    {"General Medicine", {{"Dr. Ramesh Iyer","MD General Medicine",20},{"Dr. Anjali Mehta","MBBS, MD",8},{"Dr. Sanjay Pillai","MD, DNB Medicine",15}}},
    {"Gynecology",       {{"Dr. Lakshmi Reddy","MS Gynecology, FRCOG",17},{"Dr. Reena Jain","MBBS, MS Gynae",10},{"Dr. Veena Nambiar","MD, MS OBG",13}}},
    {"Dermatology",      {{"Dr. Aisha Khan","MD Dermatology",12},{"Dr. Tushar Shah","MBBS, MD Derm",7},{"Dr. Nisha Bakshi","MD, DVD",9}}},
    {"Psychiatry",       {{"Dr. Prashant Bhat","MD Psychiatry, MRCPsych",14},{"Dr. Swati Deshpande","MBBS, MD Psych",8},{"Dr. Kartik Nair","MD, DPM",11}}},
    {"Ophthalmology",    {{"Dr. Samir Kulkarni","MS Ophthalmology",16},{"Dr. Ritu Joshi","MBBS, MS Ophthal",9},{"Dr. Mohan Das","DO, MS Eye",12}}},
    {"ENT",              {{"Dr. Arun Saxena","MS ENT, FICS",18},{"Dr. Geeta Rao","MBBS, MS ENT",7},{"Dr. Vijay Patil","DLO, MS ENT",10}}},
    {"Urology",          {{"Dr. Sunil Chandra","MCh Urology",21},{"Dr. Anita Reddy","MS, MCh Uro",11},{"Dr. Kedar Joshi","MBBS, MS, MCh",8}}},
    {"Pulmonology",      {{"Dr. Girish Naik","MD, DM Pulmonology",15},{"Dr. Smita Ghosh","MBBS, MD Pulm",9},{"Dr. Prakash Hegde","MD, DNB Pulm",12}}},
    {"ICU",              {{"Dr. Rahul Mishra","MD, Critical Care EDIC",16},{"Dr. Divya Menon","MBBS, MD Intensivist",10},{"Dr. Siddharth Roy","MD, FJPICM",13}}}
};

const vector<string> BLOOD_GROUPS  = {"A+","A-","B+","B-","AB+","AB-","O+","O-"};
const vector<string> GENDER_LIST   = {"Male","Female","Other"};
const vector<string> SEVERITY_DESC = {
    "","1-Minimal","2-Very Low","3-Low","4-Mild","5-Moderate",
    "6-Significant","7-High","8-Serious","9-Critical","10-Life-Threatening"
};
const vector<string> STATUS_LIST   = {"Active","Under Observation","Discharged"};
const vector<string> AVAIL_LIST    = {"Morning","Evening","Night","On-Call"};
const vector<string> APPT_TIMES    = {
    "08:00","09:00","10:00","11:00","12:00",
    "14:00","15:00","16:00","17:00","18:00","19:00","20:00"
};

// ════════════════════════════════════════════════════════
//  UTILITY
// ════════════════════════════════════════════════════════
string currentTimestamp() {
    time_t now = time(nullptr); char buf[20];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", localtime(&now));
    return string(buf);
}
string currentDate() {
    time_t now = time(nullptr); char buf[11];
    strftime(buf, sizeof(buf), "%Y-%m-%d", localtime(&now));
    return string(buf);
}
void clearInputBuffer() { cin.clear(); cin.ignore(numeric_limits<streamsize>::max(), '\n'); }
void pause() { cout << "\n  Press Enter to continue..."; clearInputBuffer(); }
void printLine(char c='-', int w=72) { cout << string(w,c) << "\n"; }
void printHeader(const string& title) { cout<<"\n"; printLine('='); cout<<"  "<<title<<"\n"; printLine('='); }

// ════════════════════════════════════════════════════════
//  LOGS
// ════════════════════════════════════════════════════════
void appendLog(const string& action, const string& detail) {
    ofstream f(FILE_LOGS, ios::app);
    f << "[" << currentTimestamp() << "] [" << action << "] " << detail << "\n";
}

// ════════════════════════════════════════════════════════
//  SELECTION MENUS
// ════════════════════════════════════════════════════════
string selectFromList(const string& title, const vector<string>& options) {
    cout << "\n  -- " << title << " --\n";
    for (size_t i=0; i<options.size(); ++i)
        cout << "  [" << setw(2) << (i+1) << "] " << options[i] << "\n";
    cout << "  [0]  Cancel / Back\n  Choice: ";
    int ch; cin >> ch; clearInputBuffer();
    if (ch<1 || ch>(int)options.size()) return "";
    return options[ch-1];
}

string selectDepartment() {
    cout << "\n  -- Select Department --\n";
    for (size_t i=0; i<DEPT_CATALOG.size(); ++i)
        cout << "  [" << setw(2) << (i+1) << "] " << left << setw(20)
             << DEPT_CATALOG[i].name << "  (" << DEPT_CATALOG[i].floor << ")\n";
    cout << "  [0]  Cancel\n  Choice: ";
    int ch; cin >> ch; clearInputBuffer();
    if (ch<1 || ch>(int)DEPT_CATALOG.size()) return "";
    return DEPT_CATALOG[ch-1].name;
}

string selectDoctor(const string& dept) {
    auto it = DOCTOR_CATALOG.find(dept);
    if (it==DOCTOR_CATALOG.end()) { cout<<"  No doctors listed for this department.\n"; return ""; }
    const auto& docs = it->second;
    cout << "\n  -- Select Doctor (" << dept << ") --\n";
    for (size_t i=0; i<docs.size(); ++i)
        cout << "  [" << (i+1) << "] " << left << setw(28)
             << get<0>(docs[i]) << "  " << get<1>(docs[i])
             << "  (" << get<2>(docs[i]) << " yrs exp)\n";
    cout << "  [0]  Cancel\n  Choice: ";
    int ch; cin >> ch; clearInputBuffer();
    if (ch<1 || ch>(int)docs.size()) return "";
    return get<0>(docs[ch-1]);
}

// ════════════════════════════════════════════════════════
//  PATIENT FILE I/O
//  id|name|age|gender|phone|bloodGroup|severity|dept|doctor|admitDate|status|notes
// ════════════════════════════════════════════════════════
string serializePatient(const Patient& p) {
    return to_string(p.id)+"|"+p.name+"|"+to_string(p.age)+"|"+
           p.gender+"|"+p.phone+"|"+p.bloodGroup+"|"+
           to_string(p.severity)+"|"+p.department+"|"+
           p.assignedDoctor+"|"+p.admitDate+"|"+p.status+"|"+p.notes;
}
bool deserializePatient(const string& line, Patient& p) {
    istringstream ss(line); string t; vector<string> v;
    while (getline(ss,t,'|')) v.push_back(t);
    if (v.size()<12) return false;
    try {
        p.id=stoi(v[0]); p.name=v[1]; p.age=stoi(v[2]); p.gender=v[3];
        p.phone=v[4]; p.bloodGroup=v[5]; p.severity=stoi(v[6]);
        p.department=v[7]; p.assignedDoctor=v[8]; p.admitDate=v[9];
        p.status=v[10]; p.notes=v[11];
    } catch(...) { return false; }
    return true;
}
vector<Patient> loadPatients() {
    vector<Patient> list; ifstream f(FILE_PATIENTS); if(!f) return list;
    string line;
    while (getline(f,line)) { if(line.empty()) continue; Patient p; if(deserializePatient(line,p)) list.push_back(p); }
    return list;
}
void savePatients(const vector<Patient>& list) {
    ofstream f(FILE_PATIENTS, ios::trunc);
    for (auto& p:list) f << serializePatient(p) << "\n";
}

// ════════════════════════════════════════════════════════
//  DOCTOR FILE I/O
//  id|name|spec|dept|qual|phone|exp|avail|patientCount
// ════════════════════════════════════════════════════════
string serializeDoctor(const Doctor& d) {
    return to_string(d.id)+"|"+d.name+"|"+d.specialization+"|"+d.department+"|"+
           d.qualification+"|"+d.phone+"|"+to_string(d.experience)+"|"+
           d.availability+"|"+to_string(d.patientCount);
}
bool deserializeDoctor(const string& line, Doctor& d) {
    istringstream ss(line); string t; vector<string> v;
    while (getline(ss,t,'|')) v.push_back(t);
    if (v.size()<9) return false;
    try {
        d.id=stoi(v[0]); d.name=v[1]; d.specialization=v[2]; d.department=v[3];
        d.qualification=v[4]; d.phone=v[5]; d.experience=stoi(v[6]);
        d.availability=v[7]; d.patientCount=stoi(v[8]);
    } catch(...) { return false; }
    return true;
}
vector<Doctor> loadDoctors() {
    vector<Doctor> list; ifstream f(FILE_DOCTORS); if(!f) return list;
    string line;
    while (getline(f,line)) { if(line.empty()) continue; Doctor d; if(deserializeDoctor(line,d)) list.push_back(d); }
    return list;
}
void saveDoctors(const vector<Doctor>& list) {
    ofstream f(FILE_DOCTORS, ios::trunc);
    for (auto& d:list) f << serializeDoctor(d) << "\n";
}

// ════════════════════════════════════════════════════════
//  APPOINTMENT FILE I/O
//  id|patientId|patientName|doctorId|doctorName|dept|date|time|purpose|status
// ════════════════════════════════════════════════════════
string serializeAppt(const Appointment& a) {
    return to_string(a.id)+"|"+to_string(a.patientId)+"|"+a.patientName+"|"+
           to_string(a.doctorId)+"|"+a.doctorName+"|"+a.department+"|"+
           a.date+"|"+a.time+"|"+a.purpose+"|"+a.status;
}
bool deserializeAppt(const string& line, Appointment& a) {
    istringstream ss(line); string t; vector<string> v;
    while (getline(ss,t,'|')) v.push_back(t);
    if (v.size()<10) return false;
    try {
        a.id=stoi(v[0]); a.patientId=stoi(v[1]); a.patientName=v[2];
        a.doctorId=stoi(v[3]); a.doctorName=v[4]; a.department=v[5];
        a.date=v[6]; a.time=v[7]; a.purpose=v[8]; a.status=v[9];
    } catch(...) { return false; }
    return true;
}
vector<Appointment> loadAppointments() {
    vector<Appointment> list; ifstream f(FILE_APPOINTMENTS); if(!f) return list;
    string line;
    while (getline(f,line)) { if(line.empty()) continue; Appointment a; if(deserializeAppt(line,a)) list.push_back(a); }
    return list;
}
void saveAppointments(const vector<Appointment>& list) {
    ofstream f(FILE_APPOINTMENTS, ios::trunc);
    for (auto& a:list) f << serializeAppt(a) << "\n";
}

// ════════════════════════════════════════════════════════
//  DEPARTMENT FILE I/O
//  name|totalBeds|occupiedBeds|headDoctor|floor|phone
// ════════════════════════════════════════════════════════
string serializeDept(const Department& d) {
    return d.name+"|"+to_string(d.totalBeds)+"|"+to_string(d.occupiedBeds)+"|"+
           d.headDoctor+"|"+d.floor+"|"+d.phone;
}
bool deserializeDept(const string& line, Department& d) {
    istringstream ss(line); string t; vector<string> v;
    while (getline(ss,t,'|')) v.push_back(t);
    if (v.size()<6) return false;
    try {
        d.name=v[0]; d.totalBeds=stoi(v[1]); d.occupiedBeds=stoi(v[2]);
        d.headDoctor=v[3]; d.floor=v[4]; d.phone=v[5];
    } catch(...) { return false; }
    return true;
}
vector<Department> loadDepartments() {
    vector<Department> list; ifstream f(FILE_DEPARTMENTS); if(!f) return list;
    string line;
    while (getline(f,line)) { if(line.empty()) continue; Department d; if(deserializeDept(line,d)) list.push_back(d); }
    return list;
}
void saveDepartments(const vector<Department>& list) {
    ofstream f(FILE_DEPARTMENTS, ios::trunc);
    for (auto& d:list) f << serializeDept(d) << "\n";
}
void initDepartments() {
    if (!loadDepartments().empty()) return;
    vector<Department> list;
    for (auto& di:DEPT_CATALOG) {
        Department d; d.name=di.name; d.totalBeds=di.totalBeds; d.occupiedBeds=0;
        d.floor=di.floor; d.phone=di.phone;
        auto it=DOCTOR_CATALOG.find(di.name);
        d.headDoctor=(it!=DOCTOR_CATALOG.end())?get<0>(it->second[0]):"TBD";
        list.push_back(d);
    }
    saveDepartments(list);
}
void adjustBeds(const string& deptName, int delta) {
    auto list=loadDepartments();
    for (auto& d:list) if(d.name==deptName) d.occupiedBeds=max(0,min(d.totalBeds,d.occupiedBeds+delta));
    saveDepartments(list);
}

// ════════════════════════════════════════════════════════
//  DISCHARGE FILE  (append-only)
//  patientId|name|dept|doctor|admitDate|dischargeDate|summary
// ════════════════════════════════════════════════════════
void appendDischarge(const Patient& p, const string& summary) {
    ofstream f(FILE_DISCHARGE, ios::app);
    f << p.id<<"|"<<p.name<<"|"<<p.department<<"|"<<p.assignedDoctor<<"|"
      <<p.admitDate<<"|"<<currentDate()<<"|"<<summary<<"\n";
}

// ════════════════════════════════════════════════════════
//  ID generators
// ════════════════════════════════════════════════════════
int nextPatientId(const vector<Patient>& v)      { int m=0; for(auto& x:v) m=max(m,x.id); return m+1; }
int nextDoctorId (const vector<Doctor>&  v)      { int m=0; for(auto& x:v) m=max(m,x.id); return m+1; }
int nextApptId   (const vector<Appointment>& v)  { int m=0; for(auto& x:v) m=max(m,x.id); return m+1; }

// ════════════════════════════════════════════════════════
//  DISPLAY HELPERS
// ════════════════════════════════════════════════════════
void printPatientHeader() {
    printLine();
    cout << left
         << setw(5) <<" ID" << setw(22)<<" Name" << setw(5)<<" Age" << setw(8)<<" Gender"
         << setw(6)<<" Sev" << setw(18)<<" Department" << setw(24)<<" Doctor" << " Status\n";
    printLine();
}
void printPatientRow(const Patient& p) {
    cout << left
         << setw(5) <<(" "+to_string(p.id)) << setw(22)<<(" "+p.name) << setw(5)<<(" "+to_string(p.age))
         << setw(8)<<(" "+p.gender) << setw(6)<<(" "+to_string(p.severity))
         << setw(18)<<(" "+p.department) << setw(24)<<(" "+p.assignedDoctor)
         << " "<<p.status<<"\n";
}
void printPatientFull(const Patient& p) {
    printLine();
    cout << "  Patient ID     : " << p.id         << "\n"
         << "  Name           : " << p.name        << "\n"
         << "  Age / Gender   : " << p.age << " / " << p.gender << "\n"
         << "  Phone          : " << p.phone        << "\n"
         << "  Blood Group    : " << p.bloodGroup   << "\n"
         << "  Severity       : " << p.severity << "  " << SEVERITY_DESC[p.severity] << "\n"
         << "  Department     : " << p.department   << "\n"
         << "  Doctor         : " << p.assignedDoctor << "\n"
         << "  Admit Date     : " << p.admitDate    << "\n"
         << "  Status         : " << p.status       << "\n"
         << "  Notes          : " << p.notes        << "\n";
    printLine();
}

// ════════════════════════════════════════════════════════
//  MODULE 1 - PATIENT MANAGEMENT
// ════════════════════════════════════════════════════════

void registerPatient() {
    printHeader("Register New Patient");
    auto list=loadPatients();
    Patient p; p.id=nextPatientId(list); p.severity=1;

    clearInputBuffer();
    cout << "  Full Name      : "; getline(cin,p.name);
    cout << "  Age            : "; cin>>p.age; clearInputBuffer();

    p.gender=selectFromList("Select Gender",GENDER_LIST);
    if (p.gender.empty()) p.gender="Other";

    cout << "  Phone Number   : "; getline(cin,p.phone);

    p.bloodGroup=selectFromList("Select Blood Group",BLOOD_GROUPS);
    if (p.bloodGroup.empty()) p.bloodGroup="Unknown";

    cout << "\n  -- Select Severity --\n";
    for (int i=1;i<=10;++i) cout<<"  ["<<setw(2)<<i<<"] "<<SEVERITY_DESC[i]<<"\n";
    cout << "  Choice (1-10): "; cin>>p.severity; clearInputBuffer();
    p.severity=max(1,min(10,p.severity));

    p.department=selectDepartment();
    if (p.department.empty()) { cout<<"  Registration cancelled.\n"; pause(); return; }

    p.assignedDoctor=selectDoctor(p.department);
    if (p.assignedDoctor.empty()) { cout<<"  Registration cancelled.\n"; pause(); return; }

    p.status="Active"; p.admitDate=currentDate();
    cout << "  Clinical Notes : "; getline(cin,p.notes);

    list.push_back(p);
    savePatients(list);
    adjustBeds(p.department,+1);
    appendLog("ADMITTED","ID:"+to_string(p.id)+" | "+p.name+" | Dept:"+p.department+" | Dr:"+p.assignedDoctor+" | Severity:"+to_string(p.severity));
    cout<<"\n  Patient registered! Assigned ID: "<<p.id<<"\n"; pause();
}

void updatePatient() {
    printHeader("Update Patient Record");
    auto list=loadPatients();
    int id; cout<<"  Enter Patient ID: "; cin>>id;
    auto it=find_if(list.begin(),list.end(),[&](const Patient& p){ return p.id==id&&p.status!="Discharged"; });
    if (it==list.end()) { cout<<"  Active patient not found.\n"; pause(); return; }

    printPatientFull(*it);
    cout<<"  What to update?\n"
        <<"  [1] Severity\n  [2] Department & Doctor\n  [3] Status\n"
        <<"  [4] Clinical Notes\n  [5] Phone\n  [6] All\n  Choice: ";
    int ch; cin>>ch; clearInputBuffer();

    string oldDept=it->department;
    if (ch==1||ch==6) {
        cout<<"\n  -- Select New Severity --\n";
        for(int i=1;i<=10;++i) cout<<"  ["<<setw(2)<<i<<"] "<<SEVERITY_DESC[i]<<"\n";
        cout<<"  Choice: "; cin>>it->severity; clearInputBuffer();
        it->severity=max(1,min(10,it->severity));
    }
    if (ch==2||ch==6) {
        string nd=selectDepartment();
        if (!nd.empty()) {
            string ndr=selectDoctor(nd);
            if (!ndr.empty()) { adjustBeds(oldDept,-1); it->department=nd; it->assignedDoctor=ndr; adjustBeds(nd,+1); }
        }
    }
    if (ch==3||ch==6) {
        vector<string> st={"Active","Under Observation"};
        string ns=selectFromList("Select Status",st);
        if (!ns.empty()) it->status=ns;
    }
    if (ch==4||ch==6) { cout<<"  New Notes: "; getline(cin,it->notes); }
    if (ch==5||ch==6) { cout<<"  New Phone: "; getline(cin,it->phone); }

    savePatients(list);
    appendLog("UPDATED","ID:"+to_string(it->id)+" | "+it->name+" | Dept:"+it->department+" | Sev:"+to_string(it->severity));
    cout<<"\n  Record updated.\n"; pause();
}

void dischargePatient() {
    printHeader("Discharge Patient");
    auto list=loadPatients();
    int id; cout<<"  Enter Patient ID: "; cin>>id;
    auto it=find_if(list.begin(),list.end(),[&](const Patient& p){ return p.id==id&&p.status!="Discharged"; });
    if (it==list.end()) { cout<<"  Active patient not found.\n"; pause(); return; }

    printPatientFull(*it);
    cout<<"  Discharge summary / reason: "; clearInputBuffer(); string summary; getline(cin,summary);
    cout<<"  Confirm discharge (y/n): "; char c; cin>>c;
    if (tolower(c)!='y') { cout<<"  Cancelled.\n"; pause(); return; }

    it->status="Discharged";
    savePatients(list);
    adjustBeds(it->department,-1);
    appendDischarge(*it,summary);
    appendLog("DISCHARGED","ID:"+to_string(it->id)+" | "+it->name+" | "+it->department);
    cout<<"  Patient discharged. Summary saved to "<<FILE_DISCHARGE<<"\n"; pause();
}

void searchPatients() {
    printHeader("Search Patients");
    auto list=loadPatients();
    cout<<"  [1] By ID\n  [2] By Name\n  [3] By Department\n  [4] By Doctor\n  [5] By Status\n  Choice: ";
    int ch; cin>>ch; clearInputBuffer();
    vector<Patient> results;

    if (ch==1) {
        int id; cout<<"  ID: "; cin>>id;
        for(auto& p:list) if(p.id==id) results.push_back(p);
    } else if (ch==2) {
        string q; cout<<"  Name: "; getline(cin,q);
        string ql=q; transform(ql.begin(),ql.end(),ql.begin(),::tolower);
        for(auto& p:list) { string nl=p.name; transform(nl.begin(),nl.end(),nl.begin(),::tolower); if(nl.find(ql)!=string::npos) results.push_back(p); }
    } else if (ch==3) {
        string dept=selectDepartment();
        for(auto& p:list) if(p.department==dept) results.push_back(p);
    } else if (ch==4) {
        string q; cout<<"  Doctor name: "; getline(cin,q);
        string ql=q; transform(ql.begin(),ql.end(),ql.begin(),::tolower);
        for(auto& p:list) { string dl=p.assignedDoctor; transform(dl.begin(),dl.end(),dl.begin(),::tolower); if(dl.find(ql)!=string::npos) results.push_back(p); }
    } else if (ch==5) {
        string st=selectFromList("Status",STATUS_LIST);
        for(auto& p:list) if(p.status==st) results.push_back(p);
    }

    if (results.empty()) { cout<<"\n  No patients found.\n"; }
    else { cout<<"\n  Found "<<results.size()<<" result(s):\n";
           if(results.size()==1) printPatientFull(results[0]);
           else { printPatientHeader(); for(auto& p:results) printPatientRow(p); } }
    pause();
}

void displayPriorityQueue() {
    printHeader("Priority Treatment Queue  (Highest Severity First)");
    auto list=loadPatients();
    priority_queue<Patient> pq;
    for(auto& p:list) if(p.status!="Discharged") pq.push(p);
    if (pq.empty()) { cout<<"  No active patients.\n"; pause(); return; }
    printPatientHeader();
    int rank=1;
    while(!pq.empty()) { cout<<"  #"<<rank++<<" "; printPatientRow(pq.top()); pq.pop(); }
    pause();
}

void displayAllPatients() {
    printHeader("All Patient Records");
    auto list=loadPatients();
    if (list.empty()) { cout<<"  No records.\n"; pause(); return; }
    printPatientHeader();
    for(auto& p:list) printPatientRow(p);
    int active=0,disc=0;
    for(auto& p:list) { if(p.status=="Discharged") disc++; else active++; }
    cout<<"\n  Total: "<<list.size()<<"  |  Active: "<<active<<"  |  Discharged: "<<disc<<"\n";
    pause();
}

// ════════════════════════════════════════════════════════
//  MODULE 2 - DOCTOR MANAGEMENT
// ════════════════════════════════════════════════════════

void addDoctor() {
    printHeader("Add Doctor Record");
    auto list=loadDoctors(); Doctor d; d.id=nextDoctorId(list); d.patientCount=0;
    clearInputBuffer();
    cout<<"  Full Name      : "; getline(cin,d.name);
    d.department=selectDepartment();
    if (d.department.empty()) { cout<<"  Cancelled.\n"; pause(); return; }
    d.specialization=d.department;
    cout<<"  Specialization : "; getline(cin,d.specialization);
    cout<<"  Qualification  : "; getline(cin,d.qualification);
    cout<<"  Phone          : "; getline(cin,d.phone);
    cout<<"  Experience(yrs): "; cin>>d.experience; clearInputBuffer();
    d.availability=selectFromList("Select Availability",AVAIL_LIST);
    if (d.availability.empty()) d.availability="Morning";
    list.push_back(d); saveDoctors(list);
    appendLog("DOCTOR_ADDED","ID:"+to_string(d.id)+" | "+d.name+" | "+d.department);
    cout<<"\n  Doctor added! ID: "<<d.id<<"\n"; pause();
}

void viewDoctors() {
    printHeader("Doctor Records");
    auto list=loadDoctors();
    if (list.empty()) { cout<<"  No doctor records on file.\n  (Doctors are available via catalog during patient registration)\n"; pause(); return; }
    printLine();
    cout<<left<<setw(5)<<" ID"<<setw(28)<<" Name"<<setw(18)<<" Department"<<setw(12)<<" Exp(yrs)"<<setw(12)<<" Avail"<<" Patients\n";
    printLine();
    for(auto& d:list)
        cout<<left<<setw(5)<<(" "+to_string(d.id))<<setw(28)<<(" "+d.name)<<setw(18)<<(" "+d.department)
            <<setw(12)<<(" "+to_string(d.experience))<<setw(12)<<(" "+d.availability)<<" "<<d.patientCount<<"\n";
    pause();
}

void viewDoctorsByDept() {
    printHeader("Doctors by Department");
    string dept=selectDepartment();
    if (dept.empty()) { pause(); return; }
    cout<<"\n  Registered doctors (doctors.txt) in "<<dept<<":\n";
    bool found=false;
    for(auto& d:loadDoctors()) if(d.department==dept) {
        cout<<"  [FILE] "<<d.name<<" | "<<d.qualification<<" | "<<d.availability<<"\n"; found=true;
    }
    if (!found) cout<<"  (none on file)\n";
    cout<<"\n  Catalog doctors for "<<dept<<":\n";
    auto it=DOCTOR_CATALOG.find(dept);
    if (it!=DOCTOR_CATALOG.end())
        for(auto& tup:it->second)
            cout<<"  [CATALOG] "<<get<0>(tup)<<" | "<<get<1>(tup)<<" | "<<get<2>(tup)<<" yrs\n";
    else cout<<"  (none)\n";
    pause();
}

// ════════════════════════════════════════════════════════
//  MODULE 3 - APPOINTMENT MANAGEMENT
// ════════════════════════════════════════════════════════

void scheduleAppointment() {
    printHeader("Schedule Appointment");
    auto patients=loadPatients(); auto appts=loadAppointments();
    int pid; cout<<"  Enter Patient ID: "; cin>>pid;
    auto pit=find_if(patients.begin(),patients.end(),[&](const Patient& p){ return p.id==pid&&p.status!="Discharged"; });
    if (pit==patients.end()) { cout<<"  Active patient not found.\n"; pause(); return; }

    Appointment a; a.id=nextApptId(appts);
    a.patientId=pit->id; a.patientName=pit->name;
    a.department=selectDepartment();
    if (a.department.empty()) { cout<<"  Cancelled.\n"; pause(); return; }
    a.doctorName=selectDoctor(a.department);
    if (a.doctorName.empty()) { cout<<"  Cancelled.\n"; pause(); return; }
    a.doctorId=0;
    cout<<"\n  Appointment date (YYYY-MM-DD): "; clearInputBuffer(); getline(cin,a.date);
    a.time=selectFromList("Select Time Slot",APPT_TIMES);
    if (a.time.empty()) a.time="09:00";
    cout<<"  Purpose        : "; getline(cin,a.purpose);
    a.status="Scheduled";
    appts.push_back(a); saveAppointments(appts);
    appendLog("APPOINTMENT","ApptID:"+to_string(a.id)+" | Patient:"+a.patientName+" | Dr:"+a.doctorName+" | "+a.date+" "+a.time);
    cout<<"\n  Appointment scheduled! ID: "<<a.id<<"\n"; pause();
}

void viewAppointments() {
    printHeader("Appointments");
    cout<<"  [1] All\n  [2] By Patient ID\n  [3] By Doctor\n  [4] Scheduled only\n  Choice: ";
    int ch; cin>>ch; clearInputBuffer();
    auto appts=loadAppointments(); vector<Appointment> results;
    if (ch==1) results=appts;
    else if (ch==2) { int pid; cout<<"  Patient ID: "; cin>>pid; for(auto& a:appts) if(a.patientId==pid) results.push_back(a); }
    else if (ch==3) {
        string q; cout<<"  Doctor name: "; getline(cin,q);
        string ql=q; transform(ql.begin(),ql.end(),ql.begin(),::tolower);
        for(auto& a:appts) { string dl=a.doctorName; transform(dl.begin(),dl.end(),dl.begin(),::tolower); if(dl.find(ql)!=string::npos) results.push_back(a); }
    } else if (ch==4) { for(auto& a:appts) if(a.status=="Scheduled") results.push_back(a); }

    if (results.empty()) { cout<<"  No appointments found.\n"; pause(); return; }
    printLine();
    cout<<left<<setw(5)<<" ID"<<setw(22)<<" Patient"<<setw(24)<<" Doctor"
        <<setw(16)<<" Department"<<setw(12)<<" Date"<<setw(8)<<" Time"<<" Status\n";
    printLine();
    for(auto& a:results)
        cout<<left<<setw(5)<<(" "+to_string(a.id))<<setw(22)<<(" "+a.patientName)
            <<setw(24)<<(" "+a.doctorName)<<setw(16)<<(" "+a.department)
            <<setw(12)<<(" "+a.date)<<setw(8)<<(" "+a.time)<<" "<<a.status<<"\n";
    pause();
}

void updateAppointmentStatus() {
    printHeader("Update Appointment Status");
    auto appts=loadAppointments();
    int id; cout<<"  Appointment ID: "; cin>>id;
    auto it=find_if(appts.begin(),appts.end(),[&](const Appointment& a){ return a.id==id; });
    if (it==appts.end()) { cout<<"  Not found.\n"; pause(); return; }
    vector<string> statuses={"Scheduled","Completed","Cancelled"};
    string ns=selectFromList("New Status",statuses);
    if (!ns.empty()) it->status=ns;
    saveAppointments(appts);
    appendLog("APPT_UPDATE","ApptID:"+to_string(id)+" -> "+it->status);
    cout<<"  Status updated.\n"; pause();
}

// ════════════════════════════════════════════════════════
//  MODULE 4 - REPORTS & LOGS
// ════════════════════════════════════════════════════════

void viewDepartmentStatus() {
    printHeader("Department Status & Bed Availability  (departments.txt)");
    auto depts=loadDepartments();
    if (depts.empty()) { cout<<"  No department data.\n"; pause(); return; }
    printLine();
    cout<<left<<setw(20)<<" Department"<<setw(10)<<" Total"<<setw(10)<<" Occupied"
        <<setw(10)<<" Free"<<setw(24)<<" Head Doctor"<<" Floor\n";
    printLine();
    for(auto& d:depts) {
        int free=d.totalBeds-d.occupiedBeds;
        cout<<left<<setw(20)<<(" "+d.name)<<setw(10)<<(" "+to_string(d.totalBeds))
            <<setw(10)<<(" "+to_string(d.occupiedBeds))<<setw(10)<<(" "+to_string(free))
            <<setw(24)<<(" "+d.headDoctor)<<" "<<d.floor<<"\n";
    }
    pause();
}

void viewLogs() {
    printHeader("Activity Log  ("+FILE_LOGS+")");
    cout<<"  [1] All\n  [2] Filter by keyword (ADMITTED/DISCHARGED/UPDATED/APPOINTMENT)\n  Choice: ";
    int ch; cin>>ch; clearInputBuffer();
    string filter="";
    if (ch==2) { cout<<"  Keyword: "; getline(cin,filter); transform(filter.begin(),filter.end(),filter.begin(),::toupper); }
    ifstream f(FILE_LOGS); if(!f) { cout<<"  Log file not found.\n"; pause(); return; }
    string line; int cnt=0;
    while(getline(f,line)) {
        if (!filter.empty()&&line.find(filter)==string::npos) continue;
        cout<<"  "<<line<<"\n"; ++cnt;
    }
    if(cnt==0) cout<<"  No matching entries.\n";
    else cout<<"\n  Entries shown: "<<cnt<<"\n";
    pause();
}

void viewDischargeRecords() {
    printHeader("Discharge Records  ("+FILE_DISCHARGE+")");
    ifstream f(FILE_DISCHARGE); if(!f) { cout<<"  No discharge records yet.\n"; pause(); return; }
    printLine();
    cout<<left<<setw(5)<<" PID"<<setw(22)<<" Name"<<setw(16)<<" Department"
        <<setw(12)<<" Admitted"<<setw(12)<<" Discharged"<<" Summary\n";
    printLine();
    string line;
    while(getline(f,line)) {
        if(line.empty()) continue;
        istringstream ss(line); string t; vector<string> v;
        while(getline(ss,t,'|')) v.push_back(t);
        if(v.size()<7) continue;
        cout<<left<<setw(5)<<(" "+v[0])<<setw(22)<<(" "+v[1])<<setw(16)<<(" "+v[2])
            <<setw(12)<<(" "+v[4])<<setw(12)<<(" "+v[5])<<" "<<v[6]<<"\n";
    }
    pause();
}

void generateSummaryReport() {
    printHeader("Hospital Summary Report");
    auto patients=loadPatients(); auto appts=loadAppointments(); auto depts=loadDepartments();
    int active=0,disc=0,obs=0;
    map<string,int> deptCount;
    for(auto& p:patients) {
        if(p.status=="Active") active++;
        else if(p.status=="Discharged") disc++;
        else obs++;
        if(p.status!="Discharged") deptCount[p.department]++;
    }
    int totalBeds=0,occupiedBeds=0;
    for(auto& d:depts) { totalBeds+=d.totalBeds; occupiedBeds+=d.occupiedBeds; }
    int schAppts=0; for(auto& a:appts) if(a.status=="Scheduled") schAppts++;

    cout<<"\n  +-----------------------------------+\n"
        <<"  |      HOSPITAL STATISTICS          |\n"
        <<"  +-----------------------------------+\n"
        <<"  | Total Patients Registered : "<<setw(5)<<patients.size()<<" |\n"
        <<"  | Currently Active          : "<<setw(5)<<active          <<" |\n"
        <<"  | Under Observation         : "<<setw(5)<<obs             <<" |\n"
        <<"  | Discharged                : "<<setw(5)<<disc            <<" |\n"
        <<"  | Total Beds                : "<<setw(5)<<totalBeds       <<" |\n"
        <<"  | Occupied Beds             : "<<setw(5)<<occupiedBeds    <<" |\n"
        <<"  | Free Beds                 : "<<setw(5)<<(totalBeds-occupiedBeds)<<" |\n"
        <<"  | Scheduled Appointments    : "<<setw(5)<<schAppts        <<" |\n"
        <<"  +-----------------------------------+\n";

    if (!deptCount.empty()) {
        cout<<"\n  Department-wise Active Patients:\n"; printLine();
        for(auto& kv:deptCount) cout<<"  "<<left<<setw(22)<<kv.first<<" : "<<kv.second<<"\n";
    }
    pause();
}

// ════════════════════════════════════════════════════════
//  SUB-MENUS
// ════════════════════════════════════════════════════════
void patientMenu() {
    int ch;
    do {
        cout<<"\n"; printLine('=');
        cout<<"  PATIENT MANAGEMENT\n"; printLine('=');
        cout<<"  [1] Register New Patient\n  [2] Update Patient Record\n"
            <<"  [3] Discharge Patient\n  [4] Search Patients\n"
            <<"  [5] Priority Treatment Queue\n  [6] View All Patients\n"
            <<"  [0] Back\n"; printLine('-');
        cout<<"  Choice: "; cin>>ch;
        switch(ch) {
            case 1: registerPatient();      break;
            case 2: updatePatient();        break;
            case 3: dischargePatient();     break;
            case 4: searchPatients();       break;
            case 5: displayPriorityQueue(); break;
            case 6: displayAllPatients();   break;
        }
    } while(ch!=0);
}

void doctorMenu() {
    int ch;
    do {
        cout<<"\n"; printLine('=');
        cout<<"  DOCTOR MANAGEMENT\n"; printLine('=');
        cout<<"  [1] Add Doctor Record\n  [2] View All Doctors\n"
            <<"  [3] View Doctors by Department\n  [0] Back\n"; printLine('-');
        cout<<"  Choice: "; cin>>ch;
        switch(ch) {
            case 1: addDoctor();         break;
            case 2: viewDoctors();       break;
            case 3: viewDoctorsByDept(); break;
        }
    } while(ch!=0);
}

void appointmentMenu() {
    int ch;
    do {
        cout<<"\n"; printLine('=');
        cout<<"  APPOINTMENT MANAGEMENT\n"; printLine('=');
        cout<<"  [1] Schedule Appointment\n  [2] View Appointments\n"
            <<"  [3] Update Appointment Status\n  [0] Back\n"; printLine('-');
        cout<<"  Choice: "; cin>>ch;
        switch(ch) {
            case 1: scheduleAppointment();     break;
            case 2: viewAppointments();        break;
            case 3: updateAppointmentStatus(); break;
        }
    } while(ch!=0);
}

void reportsMenu() {
    int ch;
    do {
        cout<<"\n"; printLine('=');
        cout<<"  REPORTS & LOGS\n"; printLine('=');
        cout<<"  [1] Activity Logs          -> "<<FILE_LOGS<<"\n"
            <<"  [2] Discharge Records      -> "<<FILE_DISCHARGE<<"\n"
            <<"  [3] Department Status      -> "<<FILE_DEPARTMENTS<<"\n"
            <<"  [4] Hospital Summary Report\n  [0] Back\n"; printLine('-');
        cout<<"  Choice: "; cin>>ch;
        switch(ch) {
            case 1: viewLogs();             break;
            case 2: viewDischargeRecords(); break;
            case 3: viewDepartmentStatus(); break;
            case 4: generateSummaryReport();break;
        }
    } while(ch!=0);
}

// ════════════════════════════════════════════════════════
//  MAIN MENU
// ════════════════════════════════════════════════════════
void showMainMenu() {
    cout<<"\n"; printLine('=',72);
    cout<<"          HOSPITAL PATIENT SCHEDULING SYSTEM  v2.0\n";
    printLine('=',72);
    cout<<"  Data files:\n"
        <<"    patients.txt       doctors.txt        appointments.txt\n"
        <<"    departments.txt    discharge.txt       logs.txt\n";
    printLine('-',72);
    cout<<"  [1]  Patient Management\n"
        <<"  [2]  Doctor Management\n"
        <<"  [3]  Appointment Scheduling\n"
        <<"  [4]  Reports & Logs\n"
        <<"  [0]  Exit\n";
    printLine('-',72);
    cout<<"  Choice: ";
}

int main() {
    initDepartments();  // seed departments.txt on first run
    int choice;
    do {
        showMainMenu();
        if (!(cin>>choice)) { clearInputBuffer(); choice=-1; }
        switch(choice) {
            case 1: patientMenu();     break;
            case 2: doctorMenu();      break;
            case 3: appointmentMenu(); break;
            case 4: reportsMenu();     break;
            case 0: cout<<"\n  Thank you. Goodbye!\n\n"; break;
            default: cout<<"  Invalid option.\n";
        }
    } while(choice!=0);
    return 0;
}
// g++ hospital_system.cpp -o hospital_run.exe
// .\hospital_run.exe