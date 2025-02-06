# 🚀 Blockchain-Based Supply Chain Tracking System

## **Overview**
This project implements a **supply chain tracking system using blockchain technology**. The system ensures **data integrity, transparency, and security** by leveraging **SHA-256 hashing, digital signatures (RSA), proof-of-work mining, and persistent storage (SQLite)**.

Additionally, a **Flask-based API (Blockchain Explorer)** allows users to interact with the blockchain through a web interface.

---

## **📌 Features**

### **✅ Blockchain Features**
✔ **SHA-256 cryptographic hashing** for block integrity  
✔ **RSA Digital Signatures** to validate transactions  
✔ **Proof-of-Work (PoW) Mining** for block generation  
✔ **Persistent Blockchain Storage** (SQLite database)  
✔ **Smart Contracts** for supply chain validation  
✔ **Multithreaded Mining** for faster PoW  
✔ **Command-Line Interface (CLI) for interaction**  

### **✅ Blockchain Explorer (Flask API)**
✔ **Fetch all blocks** (`/blocks`)  
✔ **Fetch a specific block** (`/blocks/<index>`)  
✔ **Validate Blockchain Integrity** (`/validate`)  

---

## **📜 Installation and Usage**

### **1️⃣ Install Required Dependencies**
Ensure you have **GCC**, **OpenSSL**, **SQLite**, and **Python Flask** installed

#### **For Linux (Ubuntu/Debian):**j
```bash
sudo apt update
sudo apt install gcc libssl-dev sqlite3 libsqlite3-dev python3-pip
pip install flask
```

#### **For macOS (Homebrew users):**

brew install openssl sqlite3
pip3 install flask


### **2️⃣ Compile the Blockchain System (C Program)**
```bash
gcc blockchain_supply_chain.c -o blockchain_supply_chain -lssl -lcrypto -pthread -lsqlite3
./blockchain_supply_chain
```

### **3️⃣ Run the Blockchain Explorer (Flask API)**
```bash
python blockchain_explorer.py
```
### **4️⃣ Access API in Web Browser**
View All Blocks → http://127.0.0.1:5000/blocks
View Specific Block → http://127.0.0.1:5000/blocks/1
Validate Blockchain → http://127.0.0.1:5000/validate

## **📌 API Endpoints**
Endpoint	Method	Description
/blocks	GET	Fetch all blocks
/blocks/<index>	GET	Fetch a specific block by index
/validate	GET	Validate blockchain integrity

## **📜 Project Structure**
📂 blockchaindev_assignment2
│── blockchain_supply_chain.c  # Main Blockchain System (C)
│── blockchain_explorer.py     # Flask API (Python)
│── blockchain.db              # SQLite Database for Blockchain
│── README.md                  # Project Documentation
