import os, sys
import re
from nltk.tokenize import word_tokenize

DIR = "data/reuters21578"

word_dict = {}
docid = 0
ntoken = 0
for f in os.listdir(DIR):
    if not f.endswith("sgm"): continue
    path = os.path.join(DIR, f)
    
    contents = []
    for l in open(path):
        contents.append(l.rstrip())
    content = " ".join(contents)

    for m in re.findall("<BODY>(.*?)</BODY>", content):
        body = m
        tokens = word_tokenize(body)
        #print tokens

        for token in tokens:
            ltoken = token.lower()
            if ltoken in word_dict:
                wid = word_dict[ltoken]
            else: 
                word_dict[ltoken] = len(word_dict)
                wid = word_dict[ltoken]
            print wid, docid
            ntoken = ntoken + 1

        docid = docid + 1

print >>sys.stderr, ntoken, len(word_dict), docid
for word in word_dict:
    print >>sys.stderr, word, word_dict[word]
