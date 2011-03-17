This is Mac OS X driver for the NetLink BCM5906M Fast Ethernet PCI Express Adapter.
FreeBSD's bge(4) source code was used as an example.

It can be found here: http://code.google.com/p/osx-bcm5906m/

Tested in Mac OS X 10.6.

Flow control and wake-on-lan is not implemented.
Power control is not tested as the hibernate mode doesn't work in Mac OS X on my laptop.

Manual installation:
    Copy the "BCM5906MEthernet.kext" file to /System/Library/Extensions/
        sudo cp -R BCM5906MEthernet.kext /System/Library/Extensions/
    Either change the file permissions with the following commands:
        sudo chown -R root:wheel /System/Library/Extensions/BCM5906MEthernet.kext
        sudo chmod -R 755 /System/Library/Extensions/BCM5906MEthernet.kext
        sudo touch /System/Library/Extensions
    Or just run the Kext Utility.app (you can easily find it with the search engine).

Automatic installation:
    Use Kext Helper.app (http://cheetha.net/)

License: see the file "Licence.txt"

Broadcom is not responsible for this driver.
