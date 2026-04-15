The Hospital Patient Scheduling System is a C++ based console application designed to streamline daily hospital operations efficiently.
It allows seamless patient registration with complete medical details including name, age, blood group, severity level, and clinical notes.
The system features a severity-based priority queue that automatically treats the most critical patients first, potentially saving lives in emergency situations.
Patients can be assigned to any of 15+ departments including Cardiology, Neurology, Emergency, ICU, and Orthopedics.
Each department has pre-configured doctors (45+ total), and the system intelligently assigns available specialists based on patient needs.
Appointment scheduling is fully supported with date, time slot selection, purpose tracking, and status management (Scheduled/Completed/Cancelled).
Bed management is automated across all departments, tracking total beds, occupied beds, and free beds in real-time.
Every action is logged with timestamps, creating a complete audit trail for accountability and legal compliance.
Discharge summaries are permanently saved with admit date, discharge date, and medical notes for future reference.
All data persists across sessions using six text files (patients, doctors, appointments, departments, discharge, logs), ensuring no information is ever lost.
