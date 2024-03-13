
stashdir=$(pwd)

cd sqlcipher

./configure \
    --prefix=$(pwd) \
    --enable-tempstore=yes \
    CFLAGS="-DSQLITE_HAS_CODEC -DSQLITE_THREADSAFE=1 -DSQLITE_ENABLE_PREUPDATE_HOOK" \
    LDFLAGS="-lcrypto" \
#&& make -j4 \
#&& make install \

cd $stashdir

