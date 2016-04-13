echo "Step 2 (for namespace $1)"
id
echo "Network cards now here:"
ip a
sudo netpriv2 "$1" bash ./step3.sh
