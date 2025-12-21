### *Vittorio Iozzia* ###

# 🧠 **Multi-Client Messaging System in C (POSIX & Windows)**

### **Servizio di Messaggistica Client–Server in C (POSIX & Windows)**

---

# 🇬🇧 **English Version**

---

## 🎯 **Project Overview**

This project implements a **concurrent multi-client electronic message board** based on a **client-server architecture**.
Users can **register, authenticate, send, read, and delete messages** in a shared environment.

Developed entirely in **C**, it uses **TCP/IP sockets** and a **custom text-based protocol**.
Two independent implementations are provided:

- **POSIX version** for Linux/Unix systems
- **Windows version** based on Winsock and Windows threading primitives

Both versions share the **same application logic and protocol**, but differ in the **operating-system-level APIs** used.

---

## ⚙️ **Repository Structure**

```text

├── src/
│   ├── src_POSIX/
│   │   ├── SERVER/
│   │   ├── CLIENT/
│   │   ├── COMMON/
│   │   └── DATA/
│   │
│   └── src_WINDOWS/
│       ├── SERVER/
│       ├── CLIENT/
│       ├── COMMON/
│       └── DATA/
│
├── Makefile
├── README.md
└── LICENSE

```
---

## 🧩 **Architecture Overview**

### Memory & Persistence

- Messages and user data are loaded into **memory (RAM)** at server startup.
- Persistence is implemented using a **write-back strategy**.
- All messages are written to disk **only during a graceful server shutdown**.
- On restart, the server reloads data from disk.

This design reduces disk I/O and simplifies synchronization, at the cost of possible data loss in case of an unexpected crash.

---

### Concurrency Model

| Platform | Thread Model | Synchronization | Shutdown Handling |
|----------|--------------|-----------------|-------------------|
| POSIX    | One thread per client (`pthread`) | `pthread_mutex_t` | `SIGINT` |
| Windows  | One thread per client (`_beginthreadex`) | `CRITICAL_SECTION` | `ConsoleCtrlHandler` |

Shared resources are protected using mutexes to avoid race conditions.

---

### Network Communication

- Reliable **TCP/IP sockets**
- Line-based custom protocol
- Blocking I/O
- Default server port: **8080**

---

## 🗺️ **Main Components**

### **1. Server**

- Accepts multiple concurrent client connections
- Handles user registration and authentication
- Stores messages in an in-memory cache
- Writes data to disk on graceful shutdown
- Protects shared data using mutexes

### **2. Client**

- Command-line, menu-driven interface
- Supports login, message sending, reading, and deletion
- Connects to a local server (default: `127.0.0.1:8080`)

### **3. Persistent Storage**

- `users.txt` — registered users with hashed passwords
- `messages.txt` — stored messages
- Files are shared across executions

---

## 🧮 **Functional Logic**

### Message Posting

Client sends a message → server stores it in memory.

### Reading Messages

Client requests messages → server sends all messages addressed to the user.

### Message Deletion

Users can delete **all messages addressed to them**, regardless of the sender.

### Authentication

Username/password authentication with password hashing.

---

## **Optimization Strategies**

- Write-back persistence minimizes disk access
- Simple text protocol improves robustness
- Mutexes eliminate race conditions
- Clean separation between POSIX and Windows implementations

---

## 🧪 **Build & Execution**

A **single Makefile** is provided to build both versions.

Build POSIX version:

```bash
make posix
```

### **Build WINDOWS version (MinGW/MSYS2):**

```bash
make windows
```
### Run executables:

```bash
./server.exe
./client.exe
```

### Windows Build Notes

The Windows version is designed to be compiled using **MinGW (GCC for Windows)** inside a Unix-like environment such as **MSYS2**.

**Native Visual Studio builds are not supported!**

---

## 🧑‍💻 **Authors & Acknowledgements**

Developed for the **Operating Systems course**, focusing on:

- Concurrency
- Synchronization
- Network programming
- Persistent storage

---

# 🇮🇹 **Versione Italiana**


## 🎯 **Descrizione del Progetto**

Il progetto realizza un **servizio di messaggistica multi-utente concorrente** basato su architettura **client–server**.
Più client possono collegarsi simultaneamente al server per **registrarsi, autenticarsi e scambiare messaggi** in modo concorrente.

L’intero sistema è sviluppato in **linguaggio C** e utilizza **socket TCP/IP** con un **protocollo testuale personalizzato**.
Il progetto è disponibile in **due versioni distinte**, pensate per ambienti diversi:

- Una **versione POSIX**, destinata a sistemi Linux/Unix  
- Una **versione Windows**, basata su Winsock e primitive di sincronizzazione Windows  

Le due implementazioni offrono **lo stesso comportamento funzionale**, ma utilizzano **meccanismi di sistema differenti**, rendendo il progetto portabile e confrontabile tra piattaforme.

---

## ⚙️ **Struttura del Repository**


```text

├── src/
│   ├── src_POSIX/
│   │   ├── SERVER/
│   │   ├── CLIENT/
│   │   ├── COMMON/
│   │   └── DATA/
│   │
│   └── src_WINDOWS/
│       ├── SERVER/
│       ├── CLIENT/
│       ├── COMMON/
│       └── DATA/
│
├── Makefile
├── README.md
└── LICENSE

```

---

## 🧩 **Scelte Architetturali**

### Gestione della Memoria e Persistenza

Durante l’esecuzione del server, **tutti i messaggi vengono mantenuti in memoria** all’interno di una struttura dati condivisa.
La persistenza su disco è gestita tramite una **politica di write-back**:

- I messaggi non vengono scritti immediatamente su file
- L’intero stato viene salvato **solo durante una chiusura controllata del server**
- All’avvio successivo, lo stato viene ripristinato leggendo i file di persistenza

Questa scelta riduce il numero di accessi al disco e semplifica la sincronizzazione tra thread, accettando il rischio di perdita dei messaggi in caso di crash improvviso.

---

### Gestione della Concorrenza

Il server è progettato per gestire **più client contemporaneamente**.

| Piattaforma | Modello di concorrenza | Sincronizzazione | Gestione della terminazione |
|------------|------------------------|------------------|-----------------------------|
| POSIX      | Un thread per client (`pthread`) | `pthread_mutex_t` | `SIGINT` |
| Windows    | Un thread per client (`_beginthreadex`) | `CRITICAL_SECTION` | `ConsoleCtrlHandler` |


Le risorse condivise (file e strutture dati in memoria) sono protette per evitare **race condition**.

---

### Comunicazione di Rete

La comunicazione tra client e server avviene tramite:

- Socket **TCP/IP**
- Protocollo testuale a righe
- I/O bloccante
- Porta di default del server: **8080**

---

## 🗺️ **Componenti del Sistema**

### **Server**

Il server si occupa di:

- Accettare connessioni concorrenti
- Gestire la registrazione e l’autenticazione degli utenti
- Mantenere i messaggi in memoria
- Salvare lo stato su disco in fase di shutdown
- Garantire la mutua esclusione sulle risorse condivise

---

### **Client**

Il client fornisce un’interfaccia testuale a menu che consente di:

- Autenticarsi o registrarsi
- Inviare messaggi ad altri utenti
- Leggere i messaggi ricevuti
- Cancellare i messaggi indirizzati all’utente

---

### **Persistenza dei Dati**

I dati persistenti sono memorizzati in file di testo:

- `users.txt` contiene le credenziali degli utenti (con password hashate)
- `messages.txt` contiene i messaggi salvati dal server

---

## 🧮 **Logica Funzionale**

### Invio dei Messaggi

- Il client invia un messaggio al server, che lo memorizza nella struttura dati in memoria.

### Lettura dei Messaggi

- Quando un utente richiede la lettura, il server invia **tutti i messaggi indirizzati a quell’utente**.

### Cancellazione dei Messaggi

- Un utente può eliminare **tutti i messaggi ricevuti**, indipendentemente dall’utente che li ha inviati.

### Autenticazione

- L’autenticazione è basata su **username e password**, con memorizzazione delle password tramite hashing.

---

## 🧪 **Compilazione ed Esecuzione**

Il progetto utilizza un **Makefile unico** per entrambe le versioni.

### **Compilazione versione POSIX:**

```bash
make posix
```

### **Compilazione versione Windows (MinGW/MSYS2):**

```bash
make windows
```
### Lancio degli eseguibili:

```bash
./server.exe
./client.exe
```
---

**License:** MIT  
**Language:** C  
**Architecture:** Client–Server  
**Protocol:** TCP/IP  



