#ifndef AVRPROGRAMMER_H
#define AVRPROGRAMMER_H

#include <QtSerialPort>


// Given a serial object that is suspected to have an AVR109-compatible booloader
// attached to it (ie, caterina), use it to load a new user program.
//
// Note that these are just commands; you still need to send them in the
// correct sequence.
//
// Based on AVRProgrammer from AVR911 - AVR Open-source Programmer
// https://code.google.com/p/avrosp/source/browse/trunk/SourceCode/AVRInSystemProg.hpp
class AvrProgrammer
{
public:
    AvrProgrammer();

    bool connect(QSerialPortInfo info);
    void disconnect();

    bool isConnected();

    void enterProgrammingMode();

//    bool chipErase();

//    bool readOSCCAL( long pos, long * value );
//    bool readSignature( long * sig0, long * sig1, long * sig2 );
//    bool checkSignature( long sig0, long sig1, long sig2 );

//    bool writeFlashByte( long address, long value );
//    bool writeEEPROMByte( long address, long value );

//    bool writeFlash( HEXFile * data );
//    bool readFlash( HEXFile * data );

//    bool writeEEPROM( HEXFile * data );
//    bool readEEPROM( HEXFile * data );

//    bool writeLockBits( long bits );
//    bool readLockBits( long * bits );

//    bool writeFuseBits( long bits );
//    bool readFuseBits( long * bits );
//    bool writeExtendedFuseBits( long bits );
//    bool readExtendedFuseBits( long * bits );

//    bool programmerSoftwareVersion( long * major, long * minor );
//    bool programmerHardwareVersion( long * major, long * minor );

    /// High-level interface functions

    /// Read the contents of the flash
    /// @param data QByteArray containing the data read from the flash
    /// @param startAddress Word-aligned address to begin reading from, in bytes
    /// @param lengthBytes Length of data to read, in bytes
    bool readFlash(QByteArray& data, int startAddress, int lengthBytes);

    /// Write the contents of the flash
    /// Note that if length is not a multiple of the page size, some pre-existing
    /// data at the end may be erased.
    /// @param data QByteArray containing the data to write to the flash
    /// @param startAddress Page-aligned address to begin writing to, in bytes
    bool writeFlash(QByteArray& data, int startAddress);

private:
    QSerialPort serial;

    /// Convenience serial functions

    // Send a command to a connected bootloader
    bool sendCommand(QByteArray command);

    bool readResponse(QByteArray& response, int expectedLength);

    // Check that a known response was received
    // Reads back all available serial data, and returns true if it is equal in
    // length and content to response.
    bool checkResponse(QByteArray response);

    /// These functions are direct implementations of the bootloader interface

    // Set the current read/write address for flash/EEPROM reading/writing
    // Note: The address needs to be divisble by 2, because it is interpreted
    // as a word address by the bootloader.
    bool setAddress(int address);

    // Read the SPM (flash) page size. This is the size of (8-bit) blocks that
    // are erased/written at once
    // TODO: This is actually the buffer size, not the flash size?
    bool getFlashPageSize(int& pageSizeBytes);

    // Check that we are talking to the correct bootloader
    bool checkSoftwareIdentifier();

    // Check that we are talking to the correct device
    bool checkDeviceSignature();

    // Reset the processor by forcing its countdown timer to expire.
    bool reset();

};

#endif // AVRPROGRAMMER_H
