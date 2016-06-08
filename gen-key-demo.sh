# This demo create example sets of keys. Signing and verifying it
# IDP -> IMD -> IDI -> IDC

# generate
./tunserver.elf --out-private "IDP" --gen-key "ed25519:x3" "ntru_sign:x1"
./tunserver.elf --out-private "IDM" --gen-key "ed25519:x3" "ntru_sign:x1"

# sign
./tunserver.elf --sign-with-key ~/.config/antinet/galaxy42/public/IDM.pub IDP

./tunserver.elf --out-private "IDI" --gen-key "ed25519:x3" "ntru_sign:x1"
./tunserver.elf --sign-with-key ~/.config/antinet/galaxy42/public/IDI.pub IDM

# IDC is exchange set of keys for connection
./tunserver.elf --out-private "IDC" --gen-key "x25519:x2" "ntru_ees439ep1:x2" "sidh:x1"
./tunserver.elf --sign-with-key ~/.config/antinet/galaxy42/public/IDC.pub IDI

# verify all chain

./tunserver.elf --verify-with-key IDM.pub IDP
./tunserver.elf --verify-with-key IDI.pub IDM
./tunserver.elf --verify-with-key IDC.pub IDI


# clean
rm ~/.config/antinet/galaxy42/wallet/IDP.PRV
rm ~/.config/antinet/galaxy42/wallet/IDM.PRV
rm ~/.config/antinet/galaxy42/wallet/IDI.PRV
rm ~/.config/antinet/galaxy42/wallet/IDC.PRV

rm ~/.config/antinet/galaxy42/public/IDP.pub
rm ~/.config/antinet/galaxy42/public/IDM.pub
rm ~/.config/antinet/galaxy42/public/IDI.pub
rm ~/.config/antinet/galaxy42/public/IDC.pub

rm ~/.config/antinet/galaxy42/public/IDM.pub.sig
rm ~/.config/antinet/galaxy42/public/IDI.pub.sig
rm ~/.config/antinet/galaxy42/public/IDC.pub.sig
