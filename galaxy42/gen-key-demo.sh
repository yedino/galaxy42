# This demo create example sets of keys. Signing and veryfying it
# IDP -> IMD -> IDI -> IDC

./tunserver.elf --out-private "IDP" --gen-key "ed25519:x3" "ntru_sign:x1"
./tunserver.elf --out-private "IDM" --gen-key "ed25519:x3" "ntru_sign:x1"
./tunserver.elf --out-private "IDI" --gen-key "ed25519:x3" "ntru_sign:x1"

# IDC is exchange set of keys for connection
./tunserver.elf --out-private "IDC" --gen-key "x25519:x2" "ntru_ees439ep1:x2" "sidh:x1"


./tunserver.elf --sign-with-key ~/.config/antinet/galaxy42/IDM.pub IDP
