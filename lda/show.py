import sys, os
import operator

DIR = "data/reuters21578"
FILE = "MINIMAL_worddict"

worddict = {}
path = os.path.join(DIR, FILE)
n = -1
for l in open(path):
  n = n + 1
  if n == 0: continue
  (word, id) = l.split(' ')
  id = int(id)
  worddict[id] = word

topics = {}
for line in sys.stdin:
    (wordid, topicid, prob) = line.split(' ')
    wordid = int(wordid)
    topicid = int(topicid)
    prob = float(prob)
    if topicid not in topics: topics[topicid] = {}
    topics[topicid][worddict[wordid]] = float(prob)

for topic in topics:
    sorted_x = sorted(topics[topic].items(), key=operator.itemgetter(1), reverse=True)
    print "\n\n-------------------------------\n\n"
    for i in range(0, min(len(sorted_x), 20)):
        print sorted_x[i]
