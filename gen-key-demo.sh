# This demo create example sets of keys. Signing and verifying it
# IDP -> IMD -> IDI -> IDC

# # generate
# ./tunserver.elf --out-private "IDP" --gen-key "ed25519:x3" "ntru_sign:x1"
# ./tunserver.elf --out-private "IDM" --gen-key "ed25519:x3" "ntru_sign:x1"
# 
# # sign
# ./tunserver.elf --sign-with-key ~/.config/antinet/galaxy42/public/IDM.pub IDP
# 
# ./tunserver.elf --out-private "IDI" --gen-key "ed25519:x3" "ntru_sign:x1"
# ./tunserver.elf --sign-with-key ~/.config/antinet/galaxy42/public/IDI.pub IDM
# 
# # IDC is exchange set of keys for connection
# ./tunserver.elf --out-private "IDC" --gen-key "x25519:x2" "ntru_ees439ep1:x2" "sidh:x1"
# ./tunserver.elf --sign-with-key ~/.config/antinet/galaxy42/public/IDC.pub IDI
# 
# # verify all chain
# 
# ./tunserver.elf --verify-with-key IDM.pub IDP
# ./tunserver.elf --verify-with-key IDI.pub IDM
# ./tunserver.elf --verify-with-key IDC.pub IDI
# 
# 
# # clean
# rm ~/.config/antinet/galaxy42/wallet/IDP.PRV
# rm ~/.config/antinet/galaxy42/wallet/IDM.PRV
# rm ~/.config/antinet/galaxy42/wallet/IDI.PRV
# rm ~/.config/antinet/galaxy42/wallet/IDC.PRV
# 
# rm ~/.config/antinet/galaxy42/public/IDP.pub
# rm ~/.config/antinet/galaxy42/public/IDM.pub
# rm ~/.config/antinet/galaxy42/public/IDI.pub
# rm ~/.config/antinet/galaxy42/public/IDC.pub
# 
# rm ~/.config/antinet/galaxy42/public/IDM.pub.sig
# rm ~/.config/antinet/galaxy42/public/IDI.pub.sig
# rm ~/.config/antinet/galaxy42/public/IDC.pub.sig

# new api

# generate
# ./nocap-tunserver.elf --gen-key --new-key "myself" --key-type "ed25519:x3"
# ./nocap-tunserver.elf --gen-key --new-key "bank" --key-type "ed25519:x1" "ntru_sign:x2"
# ./nocap-tunserver.elf --gen-key --new-key "shadyguy" --key-type "ed25519:x1"
# ./nocap-tunserver.elf --gen-key --new-key-file "myself" --key-type "ed25519:x3"
# ./nocap-tunserver.elf --gen-key --new-key "tester" --key-type "ed25519:x1" "ntru_sign:x1"
					# generating keys - means generate pair (two files) newkey.pub and newkey.PRV
# ./nocap-tunserver.elf --gen-key --new-key-file "newkey" --key-type "ed25519:x1"
# sign 
# ./nocap-tunserver.elf --sign --my-key "myself" --sign-key "tester"
# ./nocap-tunserver.elf --sign --my-key "myself" --sign-key-file "newkey.pub"

# ./nocap-tunserver.elf --sign --my-key-file "myself.PRV" --sign-key-file "newkey.pub"
# ./nocap-tunserver.elf --sign --my-key "tester" --sign-data-file "tunserver.elf"

./nocap-tunserver.elf --sign --my-key "bank" --sign-key "shadyguy"

# verify

./nocap-tunserver.elf --verify --trusted-key "bank" --toverify-key "shadyguy"
./nocap-tunserver.elf --verify --trusted-key-file "myself.pub" --toverify-data-file "tunserver.elf" --signature-file "tunserver.elf.sig"

