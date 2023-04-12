#!/usr/bin/env python3

import datetime
import hashlib
import os
import pickle
import sys
'''~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
    Authors:        MinhHien Luong
    File Name:      blockchain.py
    Description:    Chain of custody stored in binary blocks 
                    with the design as a blockchain.
    ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
'''

class Case:
    def __init__(self, case_id):
        self.case_id = case_id
        self.items = []

    def add_item(self, item_id):
        self.items.append({
            'item_id': item_id,
            'status': 'CHECKEDIN',
            'time': datetime.datetime.now().isoformat(),
        })

    def get_items(self):
        return self.items

class ChainOfCustody:
    def __init__(self, data, previous_hash):
        self.timestamp = datetime.datetime.now()
        self.data = data
        self.previous_hash = previous_hash
        self.hash = self.calculate_hash()

    def calculate_hash(self):
        sha = hashlib.sha256()
        sha.update(str(self.timestamp).encode('utf-8') +
                   str(self.data).encode('utf-8') +
                   str(self.previous_hash).encode('utf-8'))
        return sha.hexdigest()

class Blockchain:
    def __init__(self):
        self.blocks_file = 'blocks.bin'
        if os.path.exists(self.blocks_file):
            with open(self.blocks_file, 'rb') as f:
                self.chain = pickle.load(f)
                print(f"Blockchain file found with INITIAL block.")
        else:
            self.chain = [self.create_genesis_block()]
            with open(self.blocks_file, 'wb') as f:
                pickle.dump(self.chain, f)
                print(f"Blockchain file not found. Created INITIAL block.")


    def create_genesis_block(self):
        return ChainOfCustody("Genesis Block", "0")

    def add_block(self, new_data):
        previous_hash = self.chain[-1].hash
        new_block = ChainOfCustody(new_data, previous_hash)
        self.chain.append(new_block)
        with open(self.blocks_file, 'wb') as f:
            pickle.dump(self.chain, f)

    def get_chain(self):
        return self.chain

    def get_cases(self):
        cases = {}
        for block in self.chain:
            if isinstance(block.data, Case):
                cases[block.data.case_id] = block.data
        return cases

    def get_case(self, case_id):
        for block in self.chain:
            if isinstance(block.data, Case) and block.data.case_id == case_id:
                return block.data
        return None

'''
# Example usage
if __name__ == '__main__':
    
    blockchain = Blockchain()

    # Add a case with a few items to the chain
    case_id = '65cc391d-6568-4dcc-a3f1-86a2f04140f3'
    case = Case(case_id)
    case.add_item(987654321)
    case.add_item(987654321)
    blockchain.add_block(case)

    # Print out the blockchain
    for block in blockchain.get_chain():
        print(f"Timestamp: {block.timestamp}")
        if isinstance(block.data, Case):
            case = block.data
            print(f"Case ID: {case.case_id}")
            for item in case.get_items():
                print(f"Item ID: {item['item_id']}")
                print(f"Status: {item['status']}")
                print(f"Time: {item['time']}")
        else:
            print(f"Data: {block.data}")
        print(f"Previous Hash: {block.previous_hash}")
        print(f"Hash: {block.hash}")
        print("-----------------------------")
'''

if __name__ == '__main__':

    # ititializes blockchain
    blockchain = Blockchain()

    #if((str(sys.argv[1]) == 'init')):
        #blockchain = Blockchain()

    # Continually addes items from the command line
    if((str(sys.argv[1]) == 'add')):
        if(str(sys.argv[2] == '-c')):
            case_id = str(sys.argv[3])
            case = Case(case_id)
            for x in range(4, len(sys.argv), 2):
                if(x % 2 == 0 and (str(sys.argv[x])) == '-i'):
                    # Make sure to check whether the item id is the same as any previous item id's and reject if so DO THIS LATER!!
                    case.add_item(str(sys.argv[x+1]))
            
            # Adds Case to Block chain
            blockchain.add_block(case)

            # Taken from Howeie's original code, prints all values in the blockchain. Figure out how to print only the values added
            for block in blockchain.get_chain():
                if isinstance(block.data, Case):
                    case = block.data
                    print(f"Case: {case.case_id}")
                    for item in case.get_items():
                        print(f"Added item: {item['item_id']}")
                        print(f"    Status: {item['status']}")
                        print(f"    Time of action: {item['time']}")
                else:
                    print(f"Data: {block.data}")
                    print(f"Previous Hash: {block.previous_hash}")
                    print(f"Hash: {block.hash}")
                    print("-----------------------------")
    
    #elif((str(sys.argv[1]) == 'checkout')):    

    #elif((str(sys.argv[1]) == 'log')):

    #elif((str(sys.argv[1]) == 'remove')):

    #elif((str(sys.argv[1]) == 'verify')):

    #elif((str(sys.argv[1]) == 'checkout')):









