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
 ./nocap-tunserver.elf --gen-key --new-key "myself" --key-type "ed25519:x3"
 ./nocap-tunserver.elf --gen-key --new-key "bank" --key-type "ed25519:x1" "ntru_sign:x2"
 ./nocap-tunserver.elf --gen-key --new-key "shadyguy" --key-type "ed25519:x1"
 ./nocap-tunserver.elf --gen-key --new-key-file "myself" --key-type "ed25519:x1"
 ./nocap-tunserver.elf --gen-key --new-key "tester" --key-type "ed25519:x1" "ntru_sign:x1"
					# generating keys - means generate pair (two files) newkey.pub and newkey.PRV
 ./nocap-tunserver.elf --gen-key --new-key-file "newkey" --key-type "ed25519:x1"
# sign 
 ./nocap-tunserver.elf --sign --my-key "myself" --sign-key "tester"
 ./nocap-tunserver.elf --sign --my-key "myself" --sign-key-file "newkey.pub"

 ./nocap-tunserver.elf --sign --my-key-file "myself.PRV" --sign-key-file "newkey.pub"
 ./nocap-tunserver.elf --sign --my-key-file "myself.PRV" --sign-data-file "tunserver.elf"

./nocap-tunserver.elf --sign --my-key "bank" --sign-key "shadyguy"

# verify

./nocap-tunserver.elf --verify --trusted-key "bank" --toverify-key "shadyguy"
./nocap-tunserver.elf --verify --trusted-key-file "myself.pub" --toverify-data-file "tunserver.elf"
# extern signature
./nocap-tunserver.elf --verify --trusted-key-file "myself.pub" --toverify-data-file "tunserver.elf" --signature-file "tunserver.elf.sig"

# clean
echo "Cleaning ..."
echo "removing: ~/.config/antinet/galaxy42/wallet/bank.PR"
rm ~/.config/antinet/galaxy42/wallet/bank.PRV
echo "removing: ~/.config/antinet/galaxy42/wallet/myself.PRV"
rm ~/.config/antinet/galaxy42/wallet/myself.PRV
echo "removing: ~/.config/antinet/galaxy42/wallet/shadyguy.PRV"
rm ~/.config/antinet/galaxy42/wallet/shadyguy.PRV
echo "removing: ~/.config/antinet/galaxy42/wallet/tester.PRV"
rm ~/.config/antinet/galaxy42/wallet/tester.PRV

echo "removing: ~/.config/antinet/galaxy42/public/bank.pub"
rm ~/.config/antinet/galaxy42/public/bank.pub
echo "removing: ~/.config/antinet/galaxy42/public/myself.pub"
rm ~/.config/antinet/galaxy42/public/myself.pub
echo "removing: ~/.config/antinet/galaxy42/public/shadyguy.pub"
rm ~/.config/antinet/galaxy42/public/shadyguy.pub
echo "removing: ~/.config/antinet/galaxy42/public/tester.pub"
rm ~/.config/antinet/galaxy42/public/tester.pub

echo "removing: ~/.config/antinet/galaxy42/public/shadyguy.pub.sig"
rm ~/.config/antinet/galaxy42/public/shadyguy.pub.sig
echo "removing: ~/.config/antinet/galaxy42/public/tester.pub.sig"
rm ~/.config/antinet/galaxy42/public/tester.pub.sig

echo "removing: myself.pub"
rm myself.pub
echo "removing: myself.PRV"
rm myself.PRV
echo "removing: newkey.pub"
rm newkey.pub
echo "removing: newkey.pub.sig"
rm newkey.pub.sig
echo "removing: newkey.PRV"
rm newkey.PRV
echo "removing: tunserver.elf.sig"
rm tunserver.elf.sig
