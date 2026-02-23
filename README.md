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
│   │   └── COMMON/
│   │
│   ├── src_WINDOWS/
│   │   ├── SERVER/
│   │   ├── CLIENT/
│   │   └── COMMON/
│   │
│   └── DATA/
│
├── Makefile
├── README.md
└── LICENSE

```
---

## 🧩 **Architecture Overview**

### Memory & Persistence

- Messages and user data are loaded into **memory (RAM)** at server startup for ultra-fast, non-blocking access.
- User data follows a **Write-Through** policy, protected by a dedicated mutex.
- Message persistence is managed by an asynchronous **Flush Thread** running in the background. It periodically writes to disk only if data has changed (**Dirty Bit optimization**).
- Writes utilize an **Atomic Update Pattern** (write to a `.tmp` file → force hardware write with `fsync` → replace original with `rename`) to absolutely prevent data corruption during unexpected system crashes or power losses.
- A final guaranteed save occurs during a graceful server shutdown.

---

### Concurrency Model

The server handles multiple clients simultaneously using a thread-per-client model, fortified with a connection limit (`MAX_CLIENTS`) to prevent **Denial of Service (DoS)** attacks.

| Platform | Thread Model | Synchronization | Shutdown Handling |
|----------|--------------|-----------------|-------------------|
| POSIX    | One thread per client (`pthread`) | `pthread_mutex_t` | `SIGINT` + `shutdown()` |
| Windows  | One thread per client (`_beginthreadex`) | `CRITICAL_SECTION` | `ConsoleCtrlHandler` |

Shared resources are protected using granular mutexes (`cache_mutex` and `file_mutex`) to avoid race conditions.

---

### Network Communication

- Reliable **TCP/IP sockets** with a custom line-based protocol.
- **Robust Error Handling**: Uses `MSG_NOSIGNAL` to prevent fatal `SIGPIPE` crashes when clients disconnect abruptly.
- **Socket Timeouts**: Implements `SO_RCVTIMEO` and `SO_SNDTIMEO` to prevent server deadlocks from inactive or unresponsive clients.
- **Protocol Synchronization**: Safe buffer parsing with automatic discarding of excess bytes to prevent command desynchronization in case of buffer overflows.
- Default server port: **8080**.

---

## 🗺️ **Main Components**

### **1. Server**

- Accepts multiple concurrent client connections securely.
- Handles user registration and authentication with atomic TOCTOU (Time-of-Check to Time-of-Use) prevention.
- Stores messages in an in-memory cache managed by a background Flush Thread.
- Protects shared data using multiple mutexes.

### **2. Client**

- Command-line, menu-driven interface with strict, sanitized input validation (`strtol`, dynamic `sscanf`).
- Supports login, message sending, reading, and deletion.
- Connects to a local server (default: `127.0.0.1:8080`).

### **3. Persistent Storage (DATA/)**

- `users.txt` — registered users with hashed passwords.
- `messages.txt` — stored messages managed atomically.
- The `DATA/` directory is shared across both POSIX and Windows executions.

---

## 🧮 **Functional Logic**

### Message Posting

Client sends a message → server stores it in memory instantly.

### Reading Messages

Client requests messages → server securely fetches and sends all messages addressed to the user using a counted header protocol.

### Message Deletion

Users can delete **all messages addressed to them**, implemented via an efficient O(N) in-place array compaction.

### Authentication

Username/password authentication with password hashing (djb2).

---

## 🚀 **Optimization Strategies**

- **Atomic Persistence & Dirty Bit**: Minimizes disk I/O and guarantees database integrity.
- **Thread-Safety**: Complete reentrancy using `strtok_r` and granular locking.
- **Network Resilience**: Timeouts and signal masking (`MSG_NOSIGNAL`) prevent server hangs and crashes.
- Clean separation between POSIX and Windows implementations while sharing the same `DATA/` directory.

---

## 🧪 **Build & Execution**

A **single Makefile** is provided to build both versions.

### **Build POSIX version:**

```bash
make posix
```

### **Build WINDOWS version (MinGW/MSYS2):**

```bash
make windows
```

### **Run executables:**

**For POSIX:**
```bash
./server_posix
./client_posix
```

**For WINDOWS:**
```bash
./server_win.exe
./client_win.exe
```

### Windows Build Notes

The Windows version is designed to be compiled using **MinGW (GCC for Windows)** inside a Unix-like environment such as **MSYS2**.

**⚠️ Native Visual Studio builds are not supported!**


---

## 🧑‍💻 **Authors & Acknowledgements**

**Author**: *Vittorio Iozzia*

Developed for the **Operating Systems course**, focusing on:

- Concurrency and Race Condition prevention
- Synchronization and Atomic Operations
- Advanced Network programming (Timeouts, Broken Pipes)
- Robust Persistent storage

---

# 🇮🇹 **Versione Italiana**


## 🎯 **Descrizione del Progetto**

Il progetto realizza un **servizio di messaggistica multi-utente concorrente** basato su architettura **client–server**.
Più client possono collegarsi simultaneamente al server per **registrarsi, autenticarsi e scambiare messaggi** in modo concorrente e sicuro.

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
│   │   └── COMMON/
│   │
│   ├── src_WINDOWS/
│   │   ├── SERVER/
│   │   ├── CLIENT/
│   │   └── COMMON/
│   │
│   └── DATA/
│
├── Makefile
├── README.md
└── LICENSE

```

---

## 🧩 **Scelte Architetturali**

### Gestione della Memoria e Persistenza

Durante l’esecuzione del server, **tutti i messaggi vengono mantenuti in RAM** all’interno di una struttura dati condivisa per azzerare la latenza. 
La persistenza su disco è implementata in modo robusto tramite una **Strategia Ibrida**:

- I dati utente seguono una politica di *Write-Through*.
- La persistenza dei messaggi è delegata a un **Flush Thread asincrono** in background, ottimizzato da un **Dirty Bit** (il salvataggio scatta solo in caso di reali modifiche in memoria).
- Ogni scrittura su disco è blindata dal pattern di **Aggiornamento Atomico** (scrittura su file `.tmp` → `fsync` → `rename`) per impedire la corruzione dei dati in caso di blackout o crash.
- Un ultimo salvataggio di sicurezza è forzato durante il Graceful Shutdown.

---

### Gestione della Concorrenza

Il server è progettato per gestire **più client contemporaneamente** ed è protetto contro attacchi DoS tramite un limite massimo di connessioni (`MAX_CLIENTS`).

| Piattaforma | Modello di concorrenza | Sincronizzazione | Gestione della terminazione |
|------------|------------------------|------------------|-----------------------------|
| POSIX      | Un thread per client (`pthread`) | `pthread_mutex_t` | `SIGINT` + `shutdown()` |
| Windows    | Un thread per client (`_beginthreadex`) | `CRITICAL_SECTION` | `ConsoleCtrlHandler` |


Le risorse condivise (file e strutture dati in memoria) sono protette da mutex granulari (`cache_mutex` e `file_mutex`) per evitare **race condition**.

---

### Comunicazione di Rete

La comunicazione tra client e server avviene in modo resiliente tramite:

- Socket **TCP/IP** con protocollo testuale a righe delimitato.
- **Prevenzione Crash**: L'uso del flag `MSG_NOSIGNAL` converte le disconnessioni brutali (Broken Pipe) in errori gestibili, prevenendo la terminazione anomala per `SIGPIPE`.
- **Prevenzione Deadlock**: Impostazione di timeout a livello kernel (`SO_RCVTIMEO`, `SO_SNDTIMEO`) per disconnettere i client inattivi.
- **Sincronizzazione Stream**: Funzioni di ricezione potenziate che troncano le stringhe in overflow e scartano i byte in eccesso dal socket per mantenere la sincronia del protocollo.
- Porta di default del server: **8080**

---

## 🗺️ **Componenti del Sistema**

### **Server**

Il server si occupa di:

- Accettare connessioni concorrenti in modo sicuro.
- Gestire registrazione e autenticazione bloccando tentativi TOCTOU.
- Mantenere i messaggi in RAM (gestiti dal Flush Thread).
- Garantire uno spegnimento controllato sbloccando le `accept()` pendenti tramite `shutdown()`.

---

### **Client**

Il client fornisce un’interfaccia testuale a menu estremamente rigorosa:

- Validazione sicura degli input tramite `strtol` e formati dinamici.
- Autenticazione, invio, lettura e cancellazione dei messaggi.

---

### **Persistenza dei Dati (DATA/)**

I dati persistenti sono memorizzati in file di testo in formato standardizzato all'interno della cartella condivisa `DATA/`:

- `users.txt` contiene le credenziali (con password hashate).
- `messages.txt` contiene i messaggi salvati in modo atomico.

---

## 🧮 **Logica Funzionale**

### Invio dei Messaggi

- Il client invia un messaggio al server, che lo memorizza nella cache in RAM istantaneamente.

### Lettura dei Messaggi

- Il server trasmette al client tutti i messaggi destinatigli, preceduti da un header di conteggio per sincronizzare il socket.

### Cancellazione dei Messaggi

- Effettuata tramite una compattazione in-place (O(N)) dell'array dinamico, sicura e senza riallocazioni.

### Autenticazione

- Basata su username e password con algoritmo di hashing djb2 e protezione contro attacchi Brute Force (disconnessione dopo 3 fallimenti).

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

### **Lancio degli eseguibili:**

**Per POSIX:**
```bash
./server_posix
./client_posix
```

**Per WINDOWS:**
```bash
./server_win.exe
./client_win.exe
```

### Note sulla compilazione di Windows

La versione Windows è progettata per essere compilata utilizzando **MinGW (GCC per Windows)** all’interno di un ambiente **Unix-like** come **MSYS2**.

**⚠️ La compilazione nativa tramite Visual Studio non è supportata!**

---

**License:** MIT  
**Language:** C  
**Architecture:** Client–Server  
**Protocol:** TCP/IP