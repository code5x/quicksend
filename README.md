## 🚀 QuickSend - Fast Local File Transfer Tool

QuickSend is a command-line based file transfer application. It is designed for sharing files on a local network without internet. The program works using a sender and a receiver model. One device runs the receiver, and the other sends data to it.
It supports files, folders, and text messages. It is written fully in C and uses standard system libraries. The program follows an FTP-like design and is built using TCP socket programming. The focus of this project is speed, simplicity, and portability.

---


## ✨ Features 

- Send a single file from sender to receiver
- Send multiple files in one session
- Send an entire folder as a ZIP file
- Send all files inside a folder or using file extension filter
- Send plain text messages
- Fast automatic receiver IP detection using multi-threading
- Manual IP input support if auto detection fails
- Real-time progress display with percentage, speed and ETA
- Safe file overwrite handling (auto rename)
- Transfer history logging for both sender and receiver
- Works on:
	- Linux
	- Termux (Android)

---


## 📋 Requirements 

Required tools and libraries:
- gcc / clang (C compiler)
- make 
- zip

Install on Debian / Ubuntu:
```sh
sudo apt install gcc make zip
```

Install on Termux (Android):
```sh
pkg install clang make zip
```

---


## 🛠️ Build / Install 

Clone or download the repository:
```sh
git clone https://github.com/code5x/quicksend.git
```

Move into the project directory:
```sh
cd quicksend
```

From the project root directory:
```sh
make
```

Install the programs to system bin directory:
```sh
make install
```

---


## 📘 Usage

### Step 1: Start Receiver

On the device that will receive files:
```sh
recv
```
The receiver will:
- Display its local IP address
- Wait for incoming connection

### Step 2: Send Data

On the sender device:

1. Send a single file:
```sh
send -f /path/to/file
```

2. Send multiple files:
```sh
send -f file1 file2 file3
```

3. Send a folder as ZIP, with auto detect receiver IP:
```sh
send -i auto -z /path/to/folder
```

4. Send files from a folder using filter:
```sh
send -d "/path/to/folder/*.txt"
```
This sends only .txt files from the folder.

5. Send a text message:
```sh
send -m "Hello from sender"
```

6. Interactive menu mode:
```sh
send
```
This mode allows you to choose actions from a menu.

7. Show help message:
```sh
send -h
```
It will print the help message, like:
```text
Usage:
 send [options]

Options:
 -i, --ip <ip> | auto     Receiver IP address
 -f, --file <file...>     Send file(s)
 -z, --zip <folder>       Send folder as zip
 -d, --directory <path>   Send all files in folder
                          You can use filter like /path/*.ext
 -m, --message <text>     Send text message
 -h, --help               Show this help

Examples:
 send -i 192.168.1.5 -f /path/to/document.pdf
 send -f /path/to/video.mp4
 send -i auto -f /path/to/file1.ext /path/to/file2.ext
 send -z /path/to/folder
 send -i auto -d "/path/to/folder/*.ext"
 send -i 192.168.1.5 -m "Hello"
 send
```

---


## 🖼️ Images (Screenshots)

![Demo Screenshot 1](https://github.com/code5x/quicksend/blob/main/images/screenshot_001.jpg)
![Demo Screenshot 2](https://github.com/code5x/quicksend/blob/main/images/screenshot_002.jpg)

---


## 📝 Notes

- Sender and receiver must be connected to the same local wifi network
- Internet connection is not needed
- Port 8080 must be allowed by firewall
- ZIP feature requires zip command to be installed
- Only IPv4 is supported
- Designed for LAN usage, not for public internet
