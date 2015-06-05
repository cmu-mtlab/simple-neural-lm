CC = g++
DEBUG = -g -O3
CFLAGS = -Wall -std=c++11 -pthread -c $(DEBUG)
LFLAGS = -Wall -pthread -ladept -lboost_serialization $(DEBUG)

all: train score

OBJECTS = main.o utils.o vocabulary.o
train: $(OBJECTS)
	$(CC) $(OBJECTS) $(LFLAGS) -o train

SCORE_OBJECTS = score.o utils.o vocabulary.o
score: $(SCORE_OBJECTS)
	$(CC) $(SCORE_OBJECTS) $(LFLAGS) -o score

vocabulary.o: vocabulary.cc vocabulary.h
	$(CC) $(CFLAGS) vocabulary.cc

utils.o: utils.cc utils.h
	$(CC) $(CFLAGS) utils.cc

main.o: main.cc utils.h vocabulary.h param.h context.h neurallm.h
	$(CC) $(CFLAGS) main.cc

score.o: score.cc utils.h vocabulary.h param.h context.h neurallm.h
	$(CC) $(CFLAGS) score.cc

clean:
	rm -f train
	rm *.o
