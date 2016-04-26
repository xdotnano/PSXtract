psxtract

Tool to decrypt and convert PSOne Classics from PSP/PS3. Written by Hykem.

This tool allows you to decrypt a PSOne Classics EBOOT.PBP on your PC, using the emulated PSP method. It features a modified version of libkirk's source code to support DES encryption/decryption and the AMCTRL functions.

Notes

Using the "-c" option in the command line, psxtract will additionally convert the resulting ISO image to a mountable PSOne CD-ROM binary image (BIN/CUE).

You may supply a KEYS.BIN file to the tool, but this is not necessary. Using the internal files' hashes, psxtract can calculate the key by itself.

Game file manual decryption is also supported (DOCUMENT.DAT).

For more details about the algorithms involved in the extraction process please check the following sources:

PBP unpacking: https://github.com/pspdev/pspsdk/blob/master/tools/unpack-pbp.c

PGD decryption: http://www.emunewz.net/forum/showthread.php?tid=3834 (initial research) https://code.google.com/p/jpcsp/source/browse/trunk/src/jpcsp/crypto/PGD.java (JPCSP) https://github.com/tpunix/kirk_engine/blob/master/npdrm/dnas.c (tpunix)

AMCTRL functions: https://code.google.com/p/jpcsp/source/browse/trunk/src/jpcsp/crypto/AMCTRL.java (JPCSP) https://github.com/tpunix/kirk_engine/blob/master/kirk/amctrl.c (tpunix)

CD-ROM ECC/EDC: https://github.com/DeadlySystem/isofix (Daniel Huguenin)

Working games and compatibility

The following games have been tested with ePSXe and are known to work. All games were bought from the PSN US store unless another store is indicated.

Breath of Fire IV
Crash Bandicoot
Crash Bandicoot 2: Cortex Strikes Back
Crash Bandicoot 3: WARPED
CTR: Crash Team Racing
Final Fantasy VII (German)
Final Fantasy IX
Grandia
Disney's Hercules
Metal Gear Solid (German)
Metal Slug X
Simcity 2000
Spyro the Dragon
Spyro 2: Ripto's Rage
Spyro Year of the Dragon
If a game does not appear on this list, that does not mean it won't work - it means it hasn't been tested yet. All tested games have worked so far. If you experience graphic issues, it will be due to the settings of your emulator. For example, in Final Fantasy IX and Breath of Fire IV, the battle intro animation will not happen or look different from the original game with default ePSXe settings. Make sure to set "Framebuffer effects" to 1 or more in the settings of Pete's graphics plugin. Enabling off-screen drawing is also worth a shot. The games from the PSN store should be full versions. For example, Crash Bandicoot 3 even includes the demo of Spyro the Dragon which is accessible through a cheat code in the main menu, just like the original game.

Credits

Daniel Huguenin (implementation of ECC/EDC CD-ROM patching)

Draan, Proxima and everyone involved in kirk-engine (libkirk source code)

tpunix (C port and research of the PGD and AMCTRL algorithms)

PSPSDK (PBP unpacking sample code)
