import sys
import argparse

parser = argparse.ArgumentParser()
parser.add_argument('vocab')
parser.add_argument('--unk_token', default='<unk>', type=str)
args = parser.parse_args()

vocab = set([line.strip() for line in open(args.vocab).readlines()])
unk = args.unk_token

for line in sys.stdin:
	words = line.strip().split()
	words = [w if w in vocab else unk for w in words]
	print ' '.join(words)
