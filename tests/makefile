COMPILER = gcc
CFLAGS = -Wall
CFLAGS += -I ../util -I Unity/src

all: test_ring_buf test_board 

test_ring_buf:
	@$(COMPILER) $(CFLAGS) ../util/RingBuf.c TestRingBuf.c Unity/src/unity.c -o TestRingBuf
	@echo =======================
	@echo "  Ring Buf Test"
	@echo =======================
	@./TestRingBuf
	@rm TestRingBuf
	
test_board:
	@echo 
	@$(COMPILER) $(CFLAGS) ../util/Board.c TestBoard.c Unity/src/unity.c -o TestBoard
	@echo =======================
	@echo "  Board Test"
	@echo =======================
	@./TestBoard
	@rm TestBoard
