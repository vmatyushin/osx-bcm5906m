This is a Mac OS X driver for the NetLink BCM5906M Fast Ethernet PCI Express Adapter.
Based on FreeBSD's bge(4) source code.

It is also available at http://code.google.com/p/osx-bcm5906m/ until the end of 2016.
Tested in Mac OS X 10.6 and 10.7.
Flow control and wake-on-lan is not implemented.

Manual installation:
    Copy the "BCM5906MEthernet.kext" file to /System/Library/Extensions/
        sudo cp -R BCM5906MEthernet.kext /System/Library/Extensions/
    Change the file permissions using the following commands:
        sudo chown -R root:wheel /System/Library/Extensions/BCM5906MEthernet.kext
        sudo chmod -R 755 /System/Library/Extensions/BCM5906MEthernet.kext
        sudo touch /System/Library/Extensions

Automatic installation:
    Use the Kext Utility (http://cvad-mac.narod2.ru/Kext_Utility)

License: see the file "Licence.txt"
Broadcom is not responsible for this driver.
