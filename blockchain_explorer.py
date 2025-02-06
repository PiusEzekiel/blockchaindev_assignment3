from flask import Flask, jsonify, request, render_template
import os
import struct
import time
import hashlib

app = Flask(__name__, template_folder="templates")

BLOCKCHAIN_FILE = "blockchain.dat"

# Blockchain Constants
MAX_TRANSACTIONS = 5
MAX_HASH_SIZE = 64
DIFFICULTY = 4  

# Block Structure Format for Binary Storage
BLOCK_FORMAT = "i d 64s 64s i i"
TRANSACTION_FORMAT = "i 256s 256s"

def bytes_to_string(byte_data):
    return byte_data.decode("utf-8").rstrip("\x00")

def read_blockchain():
    blockchain = []
    if not os.path.exists(BLOCKCHAIN_FILE):
        return blockchain
    
    try:
        with open(BLOCKCHAIN_FILE, "rb") as file:
            while True:
                block_data = file.read(struct.calcsize(BLOCK_FORMAT))
                if not block_data:
                    break

                # Unpack block data correctly
                try:
                    index, timestamp, prev_hash, block_hash, nonce, transaction_count = struct.unpack(BLOCK_FORMAT, block_data)
                    prev_hash = prev_hash.decode("utf-8", errors="ignore").strip("\x00")
                    block_hash = block_hash.decode("utf-8", errors="ignore").strip("\x00")

                    # Ensure timestamp is converted correctly
                    timestamp_str = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(timestamp))

                    transactions = []
                    for _ in range(transaction_count):
                        tx_data = file.read(struct.calcsize(TRANSACTION_FORMAT))
                        if not tx_data:
                            break
                        item_id, description, signature = struct.unpack(TRANSACTION_FORMAT, tx_data)
                        transactions.append({
                            "item_id": item_id,
                            "description": description.decode("utf-8", errors="ignore").strip("\x00"),
                            "signature": signature.decode("utf-8", errors="ignore").strip("\x00")
                        })

                    blockchain.append({
                        "index": index,
                        "timestamp": timestamp_str,
                        "prev_hash": prev_hash,
                        "hash": block_hash,
                        "nonce": nonce,
                        "transactions": transactions
                    })
                except Exception as e:
                    print(f"⚠️ Error decoding block: {e}")
                    break  # Stop processing corrupted blockchain files

    except Exception as e:
        print(f"⚠️ Error reading blockchain: {e}")
    
    return blockchain

def save_blockchain(blockchain):
    """ Save the blockchain to a binary file """
    try:
        with open(BLOCKCHAIN_FILE, "wb") as file:
            for block in blockchain:
                # Ensure timestamp is stored as float, not a string
                timestamp = time.mktime(time.strptime(block["timestamp"], '%Y-%m-%d %H:%M:%S')) if isinstance(block["timestamp"], str) else block["timestamp"]

                packed_block = struct.pack(
                    BLOCK_FORMAT, 
                    block["index"], 
                    timestamp,  # ✅ Corrected timestamp format
                    block["prev_hash"].encode(),
                    block["hash"].encode(),
                    block["nonce"],
                    len(block["transactions"])
                )
                file.write(packed_block)

                for tx in block["transactions"]:
                    packed_tx = struct.pack(
                        TRANSACTION_FORMAT, 
                        tx["item_id"], 
                        tx["description"].encode(), 
                        tx["signature"].encode()
                    )
                    file.write(packed_tx)

        print("✅ Blockchain successfully saved!")

    except Exception as e:
        print(f"❌ Error saving blockchain: {e}")

@app.route("/")
def home():
    return render_template("index.html")

@app.route("/blocks", methods=["GET"])
def get_blocks():
    blockchain = read_blockchain()

    for block in blockchain:
        if "nonce" not in block:
            block["nonce"] = 0  # Default nonce if missing
        
        if "hash" not in block:
            block["hash"] = "UNKNOWN"

        # Ensure transactions are included in each block
        if "transactions" not in block:
            block["transactions"] = []

    return jsonify(blockchain)


@app.route("/add_transaction", methods=["POST"])
def add_transaction():
    blockchain = read_blockchain()  # Load blockchain from file
    if not blockchain:
        return jsonify({"error": "No blocks exist yet. Mine a block first."}), 400

    data = request.get_json()
    if not data or "item_id" not in data or "description" not in data or "signature" not in data:
        return jsonify({"error": "Invalid transaction format"}), 400

    latest_block = blockchain[-1]
    if len(latest_block["transactions"]) >= MAX_TRANSACTIONS:
        return jsonify({"error": "Block is full. Mine a new block first."}), 400

    # Add transaction to latest block
    latest_block["transactions"].append({
        "item_id": data["item_id"],
        "description": data["description"],
        "signature": data["signature"]
    })

    save_blockchain(blockchain)  # ✅ Save immediately
    return jsonify({"message": "✅ Transaction added successfully!"})

@app.route("/mine_block", methods=["POST"])
def mine_block():
    blockchain = read_blockchain()  # Always load the latest blockchain

    prev_hash = blockchain[-1]["hash"] if blockchain else "00000000000000000000000000000000"
    index = len(blockchain)
    timestamp = time.time()
    nonce = 0

    while True:
        hash_attempt = f"{index}{timestamp}{prev_hash}{nonce}".encode("utf-8")
        block_hash = hashlib.sha256(hash_attempt).hexdigest()
        
        if block_hash[:DIFFICULTY] == "0" * DIFFICULTY:
            break
        nonce += 1

    new_block = {
        "index": index,
        "timestamp": time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(timestamp)),
        "prev_hash": prev_hash,
        "hash": block_hash,
        "nonce": nonce,
        "transactions": []  # ✅ No pending transactions in a newly mined block
    }

    blockchain.append(new_block)
    save_blockchain(blockchain)  # ✅ Save immediately

    return jsonify({"message": f"✅ Block {index} mined successfully!", "hash": block_hash})


@app.route("/validate", methods=["GET"])
def validate_blockchain():
    blockchain = read_blockchain()
    for i in range(1, len(blockchain)):
        if blockchain[i]["prev_hash"] != blockchain[i - 1]["hash"]:
            return jsonify({"status": "❌ Blockchain is INVALID!"}), 400
    return jsonify({"status": "✅ Blockchain is VALID!"})

if __name__ == "__main__":
    app.run(debug=True)
