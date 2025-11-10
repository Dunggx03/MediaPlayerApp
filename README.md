# Media Player App

This is a **Command Line Interface (CLI)** application written in **C++** for Linux systems, designed as a lightweight media browser and player.
The application allows users to browse directories, view and manage media files, create playlists, view and edit metadata, and play music using **SDL2** and **TagLib** libraries.
The interface is implemented using **Ncurses** for an interactive terminal experience.

## Requirements

* Linux Operating System (tested on Ubuntu 24.04+)
* g++ Compiler (C++17 standard)
* TagLib Library
* SDL2 and SDL2_mixer Library
* Ncursesw Library
* Make (build system)
* pkg-config

To install dependencies on Ubuntu:

```bash
sudo apt install g++ make pkg-config libncursesw5-dev libsdl2-dev libsdl2-mixer-dev libtag1-dev
```

## Installation

1. Clone the repository:

   ```bash
   git clone https://github.com/Dunggx03/MediaPlayerApp.git
   ```
**you must pull the wanted version from remote to your local**

2. Navigate to the project directory:

   ```bash
   cd MediaPlayer
   ```

3. Build the source code using Makefile:

   ```bash
   make
   ```

4. (Optional) Install the application globally:

   ```bash
   sudo make install
   ```

   After installation, the binary will be available system-wide as:

   ```bash
   mediaplayer
   ```

   The following user directories will be created automatically:

   ```
   ~/Music/MediaPlayer/test_media
   ~/Music/MediaPlayer/playlist ( you must create a file playlist.json first)
   ```

## Usage

### Running the Application

Run the program directly from the terminal:

```bash
make run
```

or if installed globally:

```bash
mediaplayer
```

The application can be launched from any directory or external USB device (ensure the device is mounted before running).

### Using with USB
You need to use this command to eject and change the USB:
```bash
sudo -v
```
 
When mounted, the MediaPlayer automatically scans and displays media files available in the USB directory.


### Available Features

1. **View List of Media Files**

   * Displays a list of all available audio and video files in the current directory and subdirectories with pagination (25 files per page).

2. **Playlists**

   * Manage playlists:

     * View existing playlists
     * View contents of a playlist
     * Create, update, and delete playlists

3. **Metadata of a File**

   * Retrieve and edit metadata of media files:

     * View metadata such as title, album, artist, duration, genre, publisher, year (for audio files)
     * View file name, size, duration, bitrate, and codec (for video files)
     * Edit metadata fields directly in the terminal interface

4. **Play Music**

   * Uses SDL2 to play audio:

     * Play, pause, and resume
     * Next and previous track control
     * Automatically plays the next song after one finishes
     * Displays the current time and total duration of the song

5. **Change Volume**

   * Adjust system or playback volume through UI controls.

## Testing

All unit test files are located in the `src/tests` directory.
To compile and run them all:

```bash
make test
```

Each test is built into `bin/*.out` and executed sequentially.

