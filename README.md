# Asterisk patch for Codec 2

Asterisk already supports iLBC 30 and G.729. If you want to reduce further, you could [add AMR](http://github.com/traud/asterisk-amr). However, to go for even more data reduction, this patch adds [Codec 2…](http://www.rowetel.com/codec2.html)

## Installing the patch

The patch was built on top of Asterisk 13.6.0. If you use a newer version and the patch fails, please, [report](http://help.github.com/articles/creating-an-issue/)!

    cd /usr/src/
    wget downloads.asterisk.org/pub/telephony/asterisk/asterisk-13-current.tar.gz
    tar zxf ./asterisk*
    cd ./asterisk*
    sudo apt-get --assume-yes install build-essential autoconf libssl-dev libncurses-dev libnewt-dev libxml2-dev libsqlite3-dev uuid-dev libjansson-dev libblocksruntime-dev

Install library:

To support transcoding, you’ll need to install the library Codec 2. Since Ubuntu 15.10 and Debian 9.0, the package `libcodec-dev` is available. For earlier releases, for example Ubuntu 14.04 LTS:

    sudo apt-get --assume-yes install dpkg-dev cmake debhelper libspeexdsp-dev
    wget http://archive.ubuntu.com/ubuntu/pool/universe/c/codec2/codec2_0.5-1.dsc
    wget http://archive.ubuntu.com/ubuntu/pool/universe/c/codec2/codec2_0.5-1.debian.tar.xz
    wget http://archive.ubuntu.com/ubuntu/pool/universe/c/codec2/codec2_0.5.orig.tar.xz
    tar xf codec2_*.orig.tar.xz 
    cd codec2-*/
    wget http://archive.ubuntu.com/ubuntu/pool/universe/c/codec2/codec2_0.5-1.debian.tar.xz
    tar xf codec2_*.debian.tar.xz
    dpkg-buildpackage -us -uc -nc
    sudo dpkg --install ../libcodec2-*.deb

Apply all patches:

    wget github.com/traud/asterisk-codec2/archive/master.zip
    unzip -qq master.zip
    rm master.zip
    cp --verbose --recursive ./asterisk-codec2*/* ./
    patch -p0 <./build_codec2.patch
    patch -p0 <./codec2.patch

Run the bootstrap script to re-generate configure:

    ./bootstrap.sh

Configure your patched Asterisk:

    ./configure

Compile and install:

    make --jobs
    sudo make install

## Testing
You can test Codec 2 with [CSipSimple](http://play.google.com/store/apps/details?id=com.csipsimple) plus [Codec Pack](http://play.google.com/store/apps/details?id=com.csipsimple.plugins.codecs.pack1).

## Thanks goes to
* David Rowe, Debian, and Ubuntu for providing the codec and the library.
* Asterisk team: Thanks to their efforts and architecture the module was written in one working day.
* [Antonio Eugenio Burriel](http://github.com/aeburriel/codec2/tree/master/asterisk-11) provided the starting point with his module for Asterisk 11.